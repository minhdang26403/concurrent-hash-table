#include "coarse_hash_table.h"

template<typename KeyType, typename ValueType>
CoarseHashTable<KeyType, ValueType>::~CoarseHashTable() {
  lock_.WriteLock();
  delete []table_;
  lock_.WriteUnlock();
}


template <typename KeyType, typename ValueType>
ValueType CoarseHashTable<KeyType, ValueType>::Get(const KeyType &key) {
  lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  for (const auto &entry : table_[idx]) {
    if (entry.key_ == key) {
      lock_.ReadUnlock();
      return entry.value_;
    }
  }
  lock_.ReadUnlock();
  return ValueType();
}

template <typename KeyType, typename ValueType>
void CoarseHashTable<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  lock_.WriteLock();
  size_t idx = KeyToIndex(key);
  for (auto &entry : table_[idx]) {
    if (entry.key_ == key) {
      entry.value_ = value;
      lock_.WriteUnlock();
      return;
    }
  }
  table_[idx].emplace_back(key, value);
  ++size_;
  if (size_ > max_load_factor_ * capacity_) {
    GrowHashTable();
  }
  lock_.WriteUnlock();
}

template <typename KeyType, typename ValueType>
void CoarseHashTable<KeyType, ValueType>::Delete(const KeyType &key) {
  lock_.WriteLock();
  size_t idx = KeyToIndex(key);
  std::vector<Entry> &list = table_[idx];
  for (auto it = list.begin(); it != list.end(); ++it) {
    if (it->key_ == key) {
      list.erase(it);
      --size_;
      break;
    }
  }
  lock_.WriteUnlock();
}

template<typename KeyType, typename ValueType>
bool CoarseHashTable<KeyType, ValueType>::Contains(const KeyType &key) {
  lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  for (const auto &entry : table_[idx]) {
    if (entry.key_ == key) {
      lock_.ReadUnlock();
      return true;
    }
  }
  lock_.ReadUnlock();
  return false;
}

template <typename KeyType, typename ValueType>
void CoarseHashTable<KeyType, ValueType>::GrowHashTable() {
  // Allocates a new hash table and copies all key-value pair from
  // the old hash table
  auto new_table = new std::vector<Entry>[capacity_ * 2];
  for (size_t idx = 0; idx < capacity_; ++idx) {
    for (const auto &entry : table_[idx]) {
      size_t new_idx = KeyToIndex(entry.key_);
      new_table[new_idx].push_back(entry);
    }
  }
  capacity_ *= 2;
  delete[] table_;
  table_ = new_table;
}