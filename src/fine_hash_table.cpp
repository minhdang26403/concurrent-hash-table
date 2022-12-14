#include "fine_hash_table.h"

template <typename KeyType, typename ValueType>
Bucket<KeyType, ValueType>& Bucket<KeyType, ValueType>::operator=(Bucket &other) {
  // avoid self-assignment
  if (this != &other) {
    lock_.WriteLock();
    list_ = other.GetKVList();
    lock_.WriteUnlock();
  }
  return *this;
}

template<typename KeyType, typename ValueType>
ValueType Bucket<KeyType, ValueType>::GetKV(const KeyType &key) {
  lock_.ReadLock();
  ValueType value {};
  for (const auto &entry : list_) {
    if (entry.key_ == key) {
      value = entry.value_;
      break;
    }
  }
  lock_.ReadUnlock();
  return value;
}

template<typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::ContainsKV(const KeyType &key) {
  lock_.ReadLock();
  for (const auto &entry : list_) {
    if (entry.key_ == key) {
      lock_.ReadUnlock();
      return true;
    }
  }
  lock_.ReadUnlock();
  return false;
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::InsertKV(const KeyType &key,
                                          const ValueType &value) {
  lock_.WriteLock();
  for (auto &entry : list_) {
    if (entry.key_ == key) {
      entry.value_ = value;
      lock_.WriteUnlock();
      return false;
    }
  }

  list_.emplace_back(key, value);
  lock_.WriteUnlock();
  return true;
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::DeleteKV(const KeyType &key) {
  lock_.WriteLock();
  for (auto it = list_.begin(); it != list_.end(); ++it) {
    if (it->key_ == key) {
      list_.erase(it);
      lock_.WriteUnlock();
      return true;
    }
  }
  lock_.WriteUnlock();
  return false;
}

template<typename KeyType, typename ValueType>
FineHashTable<KeyType, ValueType>::~FineHashTable() {
  // Must take a write lock to destroy the hash table
  global_lock_.WriteLock();
  delete []table_;
  global_lock_.WriteUnlock();
}

template<typename KeyType, typename ValueType>
ValueType FineHashTable<KeyType, ValueType>::Get(const KeyType &key) {
  global_lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  ValueType value = table_[idx].GetKV(key);
  global_lock_.ReadUnlock();
  return value;
}

template<typename KeyType, typename ValueType>
bool FineHashTable<KeyType, ValueType>::Contains(const KeyType &key) {
  global_lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  bool contains_key = table_[idx].ContainsKV(key);
  global_lock_.ReadUnlock();
  return contains_key;
}

template<typename KeyType, typename ValueType>
void FineHashTable<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  global_lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  if (table_[idx].InsertKV(key, value)) {
    ++size_;
  }
  if (size_ > capacity_ * max_load_factor_) {
    global_lock_.ReadUnlock();
    GrowHashTable();
  } else {
    global_lock_.ReadUnlock();
  }
}

template<typename KeyType, typename ValueType>
void FineHashTable<KeyType, ValueType>::Delete(const KeyType &key) {
  global_lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  if (table_[idx].DeleteKV(key)) {
    --size_;
  } 
  global_lock_.ReadUnlock();
}

template<typename KeyType, typename ValueType>
void FineHashTable<KeyType, ValueType>::GrowHashTable() {
  // Must take a global write lock since we modify the entire hash table
  global_lock_.WriteLock();
  // Another thread already grew the hash table
  if (size_ <= capacity_ * max_load_factor_) {
    global_lock_.WriteUnlock();
    return;
  }
  size_t old_capacity = capacity_;
  capacity_ *= 2;
  auto new_table = new Bucket<KeyType, ValueType>[capacity_];
  for (size_t idx = 0; idx < old_capacity; ++idx) {
    for (const auto &entry : table_[idx].GetKVList()) {
      size_t new_idx = KeyToIndex(entry.key_);
      new_table[new_idx].GetKVList().emplace_back(entry.key_, entry.value_);
    }
  }

  delete[] table_;
  table_ = new_table;
  global_lock_.WriteUnlock();
}