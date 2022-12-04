#ifndef ATOMIC_LINKED_LIST_H_
#define ATOMIC_LINKED_LIST_H_

#include <atomic>
#include <iostream>


template <typename KeyType, typename ValueType>
class AtomicLinkedList {
 public:
  // Forward declaration
  struct MarkPtrType;

  /**
   * Node object contains key-value pair and MarkPtr field
   */
  struct Node {
    KeyType key_;
    ValueType value_;
    MarkPtrType ptr_{};

    Node(const KeyType &key, const ValueType &value)
        : key_(key), value_(value) {}
  };

  struct MarkPtrType {
    __int128_t val{};

    MarkPtrType() = default;

    MarkPtrType(bool mark, Node *next, uint64_t tag) {
      val |= tag;
      val <<= 64;
      val |= ((uint64_t)next) | (mark ? 1 : 0);
    }

    bool operator==(const MarkPtrType &other) { return val == other.val; }

    bool operator!=(const MarkPtrType &other) { return !(*this == other); }

    Node *GetPtr() { return (Node *)(val & MASK); }

    uint64_t GetTag() { return static_cast<uint64_t>((val >> 64) & MASK); }

    bool GetMark() { return static_cast<bool>(val & 0x1); }

    void SetMarkPtr(bool mark, Node *next) {
      val |= ((uint64_t)next) | (mark ? 1 : 0);
    }

    void SetTag(uint64_t tag) {
      __int128_t ctag = tag;
      ctag <<= 64;
      val |= ctag;
    }

    static const uint64_t MASK = 0xffffffffffffffff;
  };

  struct Snapshot {
    MarkPtrType *prev;
    MarkPtrType cur;
    MarkPtrType next;
  };

 public:
  AtomicLinkedList() { head = new MarkPtrType(); }

  ~AtomicLinkedList() {
    Deallocate(head);
    delete head;
  }

  bool Insert(const KeyType &key, const ValueType &value) {
    auto node = new Node(key, value);
    ValueType dummy_value{};
    Snapshot snapshot;
    MarkPtrType *prev;
    MarkPtrType cur;

    while (true) {
      if (Find(key, dummy_value, snapshot)) {
        delete node;
        return false;
      }
      prev = snapshot.prev;
      cur = snapshot.cur;
    
      node->ptr_.SetMarkPtr(0, cur.GetPtr());

      MarkPtrType oldval(false, cur.GetPtr(), cur.GetTag());
      MarkPtrType newval(0, node, cur.GetTag() + 1);

      if (__sync_bool_compare_and_swap((__int128_t *)prev, oldval.val,
                                       newval.val)) {
        return true;
      }
    }
  }

  bool Delete(const KeyType &key) {
    ValueType dummy_value{};
    Snapshot snapshot;
    MarkPtrType *prev;
    MarkPtrType cur;
    MarkPtrType next;

    while (true) {
      if (!Find(key, dummy_value, snapshot)) {
        return false;
      }
      prev = snapshot.prev;
      cur = snapshot.cur;
      next = snapshot.next;

      MarkPtrType oldval(false, next.GetPtr(), next.GetTag());
      MarkPtrType newval(true, next.GetPtr(), next.GetTag() + 1);
      if (!__sync_bool_compare_and_swap((__int128_t *)&cur.GetPtr()->ptr_,
                                        oldval.val, newval.val)) {
        continue;
      }

      if (__sync_bool_compare_and_swap(
              (__int128_t *)prev,
              MarkPtrType(false, cur.GetPtr(), cur.GetTag()).val,
              MarkPtrType(false, next.GetPtr(), cur.GetTag() + 1).val)) {
        DeleteNode(cur.GetPtr());
        return true;
      } else {
      }
    }
    return true;
  }

  bool Find(const KeyType &key, ValueType &value, Snapshot &snapshot) {
  try_again:
    MarkPtrType *prev = head;
    MarkPtrType cur = *prev;
    MarkPtrType next;
    while (true) {
      if (cur.GetPtr() == nullptr) {
        snapshot.prev = prev;
        snapshot.cur = cur;
        snapshot.next = next;
        return false;
      }
      next = cur.GetPtr()->ptr_;
      KeyType ckey = cur.GetPtr()->key_;
      if (*prev != MarkPtrType(0, cur.GetPtr(), cur.GetTag())) {
        goto try_again;
      }
      if (!next.GetMark()) {
        if (ckey >= key) {
          if (ckey == key) {
            value = cur.GetPtr()->value_;
          }
          snapshot.prev = prev;
          snapshot.cur = cur;
          snapshot.next = next;
          return ckey == key;
        }
        prev = &(cur.GetPtr()->ptr_);
      } else {
        MarkPtrType oldval(false, cur.GetPtr(), cur.GetTag());
        MarkPtrType newval(0, next.GetPtr(), cur.GetTag() + 1);

        if (__sync_bool_compare_and_swap((__int128_t *)prev, oldval.val,
                                         newval.val)) {
          next.SetTag(cur.GetTag() + 1);
        } else {
          goto try_again;
        }
      }
      cur = next;
    }
  }

  ValueType Search(const KeyType &key) {
    ValueType value{};
    Snapshot snapshot;
    Find(key, value, snapshot);
    return value;
  }

  void Print() {
    MarkPtrType *node = head;
    while (node->GetPtr() != nullptr) {
      std::cout << node->GetPtr()->key_ << ' ' << node->GetPtr()->value_
                << std::endl;
      node = &(node->GetPtr()->ptr_);
    }
  }

  void DeleteNode(Node *node) { 
    delete node; 
  }

  void Deallocate(MarkPtrType *node) {
    if (node->GetPtr() == nullptr) {
      return;
    }
    Deallocate(&(node->GetPtr()->ptr_));
    delete node->GetPtr();
  }

 private:
  // MarkPtrType *prev;
  // MarkPtrType cur;
  // MarkPtrType next;

  MarkPtrType *head;
};

#endif  // ATOMIC_LINKED_LIST_H_