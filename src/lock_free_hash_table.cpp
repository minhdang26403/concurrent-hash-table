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
