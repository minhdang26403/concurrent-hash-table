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
    MarkPtrType
        ptr_{};  // a wrapper for the `next` pointer pointing to the next node

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

  class MarkPtrType {
   public:
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
    bool operator==(const MarkPtrType &other) const { return val == other.val; }

    /**
     * Not equal operator overloading for MarkPtrType object
     * @param other the other MarkPtrType object to compare with
     * @return true if two objects are not equal; otherwise, return false
     */
    bool operator!=(const MarkPtrType &other) const {
      return !(*this == other);
    }

    /**
     * Gets the pointer field
     * @return a pointer to the next node
     */
    constexpr Node *GetNextPtr() const { return (Node *)(val & MASK); }

    /**
     * Gets the tag field
     * @return the tag of the MarkPtrType object
     */
    constexpr uint64_t GetTag() const {
      return static_cast<uint64_t>((val >> 64) & MASK);
    }

    /**
     * Gets the mark field
     * @return true if the node holding this MarkPtrType object is deleted;
     * otherwise, return false
     */
    constexpr bool GetMark() const { return static_cast<bool>(val & 0x1); }

    /**
     * Gets the underlying value of the MarkPtrType
     * Used for compare-and-swap
     */
    constexpr __int128_t GetValue() const { return val; }

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

   private:
    __int128_t val{};  // underlying type for the pointer
    // Mask for extracting lower-order 8 bytes
    static const uint64_t MASK = 0xffffffffffffffff;
  };

  /**
   * a struct captures a snapshot of the linked list
   */
  struct Snapshot {
    MarkPtrType *prev_ptr;
    MarkPtrType prev;
    MarkPtrType cur;
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
    MarkPtrType *prev_ptr;
    MarkPtrType prev;

    while (true) {
      if (Find(key, nullptr, &snapshot)) {
        delete node;
        return false;
      }
      prev_ptr = snapshot.prev_ptr;
      prev = snapshot.prev;

      node->ptr_.SetMarkPtr(0, prev.GetNextPtr());

      MarkPtrType oldval(0, prev.GetNextPtr(), prev.GetTag());
      MarkPtrType newval(0, node, prev.GetTag() + 1);

      if (__sync_bool_compare_and_swap((__int128_t *)prev_ptr,
                                       oldval.GetValue(), newval.GetValue())) {
        return true;
      }
    }
  }

  bool Delete(const KeyType &key) {
    Snapshot snapshot;
    MarkPtrType *prev_ptr;
    MarkPtrType prev;
    MarkPtrType cur;

    while (true) {
      if (!Find(key, nullptr, &snapshot)) {
        return false;
      }
      prev_ptr = snapshot.prev_ptr;
      prev = snapshot.prev;
      cur = snapshot.cur;

      MarkPtrType oldval(0, cur.GetNextPtr(), cur.GetTag());
      MarkPtrType newval(1, cur.GetNextPtr(), cur.GetTag() + 1);
      if (!__sync_bool_compare_and_swap((__int128_t *)&prev.GetNextPtr()->ptr_,
                                        oldval.GetValue(), newval.GetValue())) {
        continue;
      }

      oldval = MarkPtrType(0, prev.GetNextPtr(), prev.GetTag());
      newval = MarkPtrType(0, cur.GetNextPtr(), prev.GetTag() + 1);

      if (__sync_bool_compare_and_swap((__int128_t *)prev_ptr,
                                       oldval.GetValue(), newval.GetValue())) {
        DeleteNode(prev.GetNextPtr());
      } else {
        Find(key, nullptr, &snapshot);
      }
      return true;
    }
  }

  bool Find(const KeyType &key, ValueType *value = nullptr,
            Snapshot *snapshot = nullptr) {
  try_again:
    MarkPtrType *prev_ptr = head;
    MarkPtrType prev = *prev_ptr;
    MarkPtrType cur;
    while (true) {
      if (prev.GetNextPtr() == nullptr) {
        if (snapshot != nullptr) {
          snapshot->prev_ptr = prev_ptr;
          snapshot->prev = prev;
          snapshot->cur = cur;
        }
        return false;
      }
      cur = prev.GetNextPtr()->ptr_;
      KeyType ckey = prev.GetNextPtr()->key_;
      if (*prev_ptr != MarkPtrType(0, prev.GetNextPtr(), prev.GetTag())) {
        goto try_again;
      }
      if (!cur.GetMark()) {
        if (ckey >= key) {
          if (ckey == key && value != nullptr) {
            *value = prev.GetNextPtr()->value_;
          }
          if (snapshot != nullptr) {
            snapshot->prev_ptr = prev_ptr;
            snapshot->prev = prev;
            snapshot->cur = cur;
          }
          return ckey == key;
        }
        prev_ptr = &(prev.GetNextPtr()->ptr_);
      } else {
        MarkPtrType oldval(0, prev.GetNextPtr(), prev.GetTag());
        MarkPtrType newval(0, cur.GetNextPtr(), prev.GetTag() + 1);

        if (__sync_bool_compare_and_swap(
                (__int128_t *)prev_ptr, oldval.GetValue(), newval.GetValue())) {
          DeleteNode(prev.GetNextPtr());
          cur.SetTag(prev.GetTag() + 1);
        } else {
          goto try_again;
        }
      }
      prev = cur;
    }
  }

  ValueType Search(const KeyType &key) {
    ValueType value{};
    Find(key, &value);
    return value;
  }

  void Print() {
    MarkPtrType *node = head;
    while (node->GetNextPtr() != nullptr) {
      std::cout << node->GetNextPtr()->key_ << ' ' << node->GetNextPtr()->value_
                << std::endl;
      node = &(node->GetNextPtr()->ptr_);
    }
  }

  void DeleteNode(Node *node) { delete node; }

  void Deallocate(MarkPtrType *node) {
    if (node->GetNextPtr() == nullptr) {
      return;
    }
    Deallocate(&(node->GetNextPtr()->ptr_));
    delete node->GetNextPtr();
  }

 private:
  MarkPtrType *head;
};

#endif  // ATOMIC_LINKED_LIST_H_