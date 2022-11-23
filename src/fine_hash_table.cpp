#include "fine_hash_table.h"

template<typename KeyType, typename ValueType>
ValueType Bucket<KeyType, ValueType>::GetKV(const KeyType &key) {
  lock_.ReadLock();
  for (const auto &entry : list_) {
    if (entry.key_ == key) {
      lock_.ReadUnlock();
      return entry.value_;
    }
  }
  lock_.ReadUnlock();
  return ValueType();
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

template<typename KeyType, typename ValueType>
void Bucket<KeyType, ValueType>::InsertKV(const KeyType &key, const ValueType &value, size_t &size) {
  lock_.WriteLock();
  for (auto &entry : list_) {
    if (entry.key_ == key) {
      entry.value_ = value;
      lock_.WriteUnlock();
    }
  }

  list_.emplace_back(key, value);
  ++size;
  lock_.WriteUnlock();
}

template<typename KeyType, typename ValueType>
void Bucket<KeyType, ValueType>::DeleteKV(const KeyType &key, size_t &size) {
  lock_.WriteLock();
  for (auto it = list_.begin(); it != list_.end(); ++it) {
    if (it->key_ == key) {
      list_.erase(it);
      --size;
      break;
    }
  }
  lock_.WriteUnlock();
}

template<typename KeyType, typename ValueType>
FineHashTable<KeyType, ValueType>::~FineHashTable() {
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
  if (size_ > capacity_ * max_load_factor_) {
    global_lock_.ReadUnlock();
    GrowHashTable();
    global_lock_.ReadLock();
  }

  size_t idx = KeyToIndex(key);
  table_[idx].InsertKV(key, value, size_);
  global_lock_.ReadUnlock();
}

template<typename KeyType, typename ValueType>
void FineHashTable<KeyType, ValueType>::Delete(const KeyType &key) {
  global_lock_.ReadLock();
  size_t idx = KeyToIndex(key);
  table_[idx].DeleteKV(key, size_);
  global_lock_.ReadUnlock();
}

template<typename KeyType, typename ValueType>
void FineHashTable<KeyType, ValueType>::GrowHashTable() {
  global_lock_.WriteLock();
  auto new_table = new Bucket<KeyType, ValueType>[capacity_ * 2];
  for (size_t idx = 0; idx < capacity_; ++idx) {
    new_table[idx] = std::move(table_[idx]);
  }
  capacity_ *= 2;
  delete[] table_;
  table_ = new_table;
  global_lock_.WriteUnlock();
}