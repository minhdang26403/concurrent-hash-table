#ifndef ATOMIC_LINKED_LIST_H_
#define ATOMIC_LINKED_LIST_H_

#include <atomic>
#include <iostream>
#include <vector>

template <typename KeyType, typename ValueType>
class AtomicLinkedList {
 public:
  // Forward declaration
  struct MarkPtrType;

  /**
   * Node object contains key-value pair and MarkPtr field
   */
  struct Node {
    KeyType key_;      // the key of a node
    ValueType value_;  // the value of a node
    MarkPtrType ptr_{};  // a wrapper for the `next` pointer pointing to the next node

    /**
     * Constructs a Node instance
     * @param key the key of an entry
     * @param value the value of an entry
     */
    Node(const KeyType &key, const ValueType &value)
        : key_(key), value_(value) {}
  };

  /**
   * MarkPtrType is a wrapper for `next` field within each node of the linked
   * list. The MarkPtrType contains a `next` pointer pointing to the next node
   * and extra information
   */

  struct MarkPtrType {
    __int128_t val{};  // underlying type for the pointer

    MarkPtrType() = default;

    /**
     * Constructs a MarkPtrType object
     * @param mark a mark bit indicates whether the node holds this MarkPtrType
     * object is deleted
     * @param next a pointer to the next node
     * @param tag a unique tag for this node
     */
    MarkPtrType(bool mark, Node *next, uint64_t tag) {
      val |= tag;
      val <<= 64;
      val |= ((uint64_t)next) | (mark ? 1 : 0);
    }

    /**
     * Equal operator overloading for MarkPtrType object
     * @param other the other MarkPtrType object to compare with
     * @return true if two objects are equal; otherwise, return false
     */
    bool operator==(const MarkPtrType &other) { return val == other.val; }

    /**
     * Not equal operator overloading for MarkPtrType object
     * @param other the other MarkPtrType object to compare with
     * @return true if two objects are not equal; otherwise, return false
     */
    bool operator!=(const MarkPtrType &other) { return !(*this == other); }

    /**
     * Gets the pointer field
     * @return a pointer to the next node
     */
    Node *GetPtr() { return (Node *)(val & MASK); }

    /**
     * Gets the tag field
     * @return the tag of the MarkPtrType object
     */
    uint64_t GetTag() { return static_cast<uint64_t>((val >> 64) & MASK); }

    /**
     * Gets the mark field
     * @return true if the node holding this MarkPtrType object is deleted;
     * otherwise, return false
     */
    bool GetMark() { return static_cast<bool>(val & 0x1); }

    /**
     * Sets the mark and `next` pointer field
     * @param mark a bit indicates whether the node holding this MarkPtrType
     * object
     * @param next a pointer to the next node
     */
    void SetMarkPtr(bool mark, Node *next) {
      val |= ((uint64_t)next) | (mark ? 1 : 0);
    }

    /**
     * Sets the tag field
     * @param tag the tag of the MarkPtrType object
     */
    void SetTag(uint64_t tag) {
      __int128_t ctag = tag;
      ctag <<= 64;
      val |= ctag;
    }

    // Mask for extracting lower-order 8 bytes
    static const uint64_t MASK = 0xffffffffffffffff;
  };

  /**
   * a struct captures a snapshot of the linked list
   */
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
    Snapshot snapshot;
    MarkPtrType *prev;
    MarkPtrType cur;

    while (true) {
      if (Find(key, nullptr, &snapshot)) {
        delete node;
        return false;
      }
      prev = snapshot.prev;
      cur = snapshot.cur;

      node->ptr_.SetMarkPtr(0, cur.GetPtr());

      MarkPtrType oldval(0, cur.GetPtr(), cur.GetTag());
      MarkPtrType newval(0, node, cur.GetTag() + 1);

      if (__sync_bool_compare_and_swap((__int128_t *)prev, oldval.val,
                                       newval.val)) {
        return true;
      }
    }
  }

  bool Delete(const KeyType &key) {
    Snapshot snapshot;
    MarkPtrType *prev;
    MarkPtrType cur;
    MarkPtrType next;

    while (true) {
      if (!Find(key, nullptr, &snapshot)) {
        return false;
      }
      prev = snapshot.prev;
      cur = snapshot.cur;
      next = snapshot.next;

      MarkPtrType oldval(0, next.GetPtr(), next.GetTag());
      MarkPtrType newval(1, next.GetPtr(), next.GetTag() + 1);
      if (!__sync_bool_compare_and_swap((__int128_t *)&cur.GetPtr()->ptr_,
                                        oldval.val, newval.val)) {
        continue;
      }

      if (__sync_bool_compare_and_swap(
              (__int128_t *)prev,
              MarkPtrType(0, cur.GetPtr(), cur.GetTag()).val,
              MarkPtrType(0, next.GetPtr(), cur.GetTag() + 1).val)) {
        DeleteNode(cur.GetPtr());
        return true;
      } else {
        Find(key, nullptr, &snapshot);
      }
    }
    return true;
  }

  bool Find(const KeyType &key, ValueType *value=nullptr, Snapshot *snapshot=nullptr) {
  try_again:
    MarkPtrType *prev = head;
    MarkPtrType cur = *prev;
    MarkPtrType next;
    while (true) {
      if (cur.GetPtr() == nullptr) {
        if (snapshot != nullptr) {
          snapshot->prev = prev;
          snapshot->cur = cur;
          snapshot->next = next;
        }
        return false;
      }
      next = cur.GetPtr()->ptr_;
      KeyType ckey = cur.GetPtr()->key_;
      if (*prev != MarkPtrType(0, cur.GetPtr(), cur.GetTag())) {
        goto try_again;
      }
      if (!next.GetMark()) {
        if (ckey >= key) {
          if (ckey == key && value != nullptr) {
            *value = cur.GetPtr()->value_;
          }
          if (snapshot != nullptr) {
            snapshot->prev = prev;
            snapshot->cur = cur;
            snapshot->next = next;
          }
          return ckey == key;
        }
        prev = &(cur.GetPtr()->ptr_);
      } else {
        MarkPtrType oldval(0, cur.GetPtr(), cur.GetTag());
        MarkPtrType newval(0, next.GetPtr(), cur.GetTag() + 1);

        if (__sync_bool_compare_and_swap((__int128_t *)prev, oldval.val,
                                         newval.val)) {
          DeleteNode(cur.GetPtr());
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
    Find(key, &value);
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
  MarkPtrType *head;
};

#endif  // ATOMIC_LINKED_LIST_H_