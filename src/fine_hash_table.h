#ifndef FINE_HASH_TABLE_H_
#define FINE_HASH_TABLE_H_

#include "rwlock.h"

#include <list>

template<typename KeyType, typename ValueType>
class Bucket {
 private:
  struct Entry {
    KeyType key_;
    ValueType value_;

    Entry() = default;
    Entry(const KeyType &key, const ValueType &value)
        : key_(key), value_(value) {}
  };

 public:
  Bucket &operator=(Bucket &&other) { 
    list_ = std::move(other.GetList());
    return *this;
  }

  ValueType GetKV(const KeyType &key);
  
  bool ContainsKV(const KeyType &key);

  void InsertKV(const KeyType &key, const ValueType &value, size_t &size);

  void DeleteKV(const KeyType &key, size_t &size);

  // std::list<Entry>& GetList() { return list_; }
  std::vector<Entry>& GetList() { return list_; }
 private: 
  ReaderWriterLock lock_;
  // std::list<Entry> list_;
  std::vector<Entry> list_;
};

template<typename KeyType, typename ValueType>
class FineHashTable {
 public:
  FineHashTable() : FineHashTable(DEFAULT_CAPACITY, DEFAULT_LOAD_FACTOR) {}

  FineHashTable(size_t capacity, float max_load_factor)
      : capacity_(capacity),
        max_load_factor_(max_load_factor),
        table_(new Bucket<KeyType, ValueType>[capacity_]) {}

  ~FineHashTable();
  
  ValueType Get(const KeyType &key);

  bool Contains(const KeyType &key);

  void Insert(const KeyType &key, const ValueType &value);

  void Delete(const KeyType &key);

 private:
  size_t KeyToIndex(const KeyType &key) const {
    return std::hash<KeyType>{}(key) % capacity_;
  }
  void GrowHashTable();

  static constexpr size_t DEFAULT_CAPACITY{128};
  static constexpr float DEFAULT_LOAD_FACTOR{0.75};
  size_t capacity_;
  float max_load_factor_;
  Bucket<KeyType, ValueType> *table_;
  size_t size_{0};
  ReaderWriterLock global_lock_;
};

#include "fine_hash_table.cpp"

#endif // FINE_HASH_TABLE_H_