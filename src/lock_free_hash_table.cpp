#include "lock_free_hash_table.h"

template<typename KeyType, typename ValueType>
LockFreeHashTable<KeyType, ValueType>::~LockFreeHashTable() {
  lock_.WriteLock();
  delete []table_;
  lock_.WriteUnlock();
}


template <typename KeyType, typename ValueType>
ValueType LockFreeHashTable<KeyType, ValueType>::Get(const KeyType &key) {
  size_t idx = KeyToIndex(key);
  ValueType value = table_[idx].Search(key);
  return value;
}

template <typename KeyType, typename ValueType>
void LockFreeHashTable<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  size_t idx = KeyToIndex(key);
  if (table_[idx].Insert(key, value)) {
    ++size_;
  }

  // if (size_ > max_load_factor_ * capacity_) {
  //   lock_.WriteUnlock();
  //   GrowHashTable();
  // } else {
  //   lock_.WriteUnlock();
  // }
}

template <typename KeyType, typename ValueType>
void LockFreeHashTable<KeyType, ValueType>::Delete(const KeyType &key) {
  size_t idx = KeyToIndex(key);
  if (table_[idx].Delete(key)) {
    --size_;
  }
}

template<typename KeyType, typename ValueType>
bool LockFreeHashTable<KeyType, ValueType>::Contains(const KeyType &key) {
  size_t idx = KeyToIndex(key);
  return table_[idx].Find(key);
}

// template <typename KeyType, typename ValueType>
// void LockFreeHashTable<KeyType, ValueType>::GrowHashTable() {
//   // Allocates a new hash table and copies all key-value pair from
//   // the old hash table
//   lock_.WriteLock();
//   auto new_table = new std::vector<Entry>[capacity_ * 2];
//   for (size_t idx = 0; idx < capacity_; ++idx) {
//     for (const auto &entry : table_[idx]) {
//       size_t new_idx = KeyToIndex(entry.key_);
//       new_table[new_idx].push_back(entry);
//     }
//   }

//   capacity_ *= 2;
//   delete[] table_;
//   table_ = new_table;
//   lock_.WriteUnlock();
// }