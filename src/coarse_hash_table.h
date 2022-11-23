#ifndef COARSE_HASH_TABLE_H_
#define COARSE_HASH_TABLE_H_

#include <list>
#include <vector>

#include "rwlock.h"

template<typename KeyType, typename ValueType>
class CoarseHashTable {
 private:
  struct Entry {
    KeyType key_{};
    ValueType value_{};

    Entry() = default;
    Entry(const KeyType &key, const ValueType &value)
        : key_(key), value_(value) {}
  };

 public:
  CoarseHashTable() : CoarseHashTable(DEFAULT_CAPACITY, DEFAULT_LOAD_FACTOR) {}

  CoarseHashTable(size_t capacity, float max_load_factor)
      : capacity_(capacity),
        max_load_factor_(max_load_factor),
        table_(new std::vector<Entry>[capacity_]) {}

  CoarseHashTable(const CoarseHashTable &other) = delete;
  CoarseHashTable& operator=(const CoarseHashTable &other) = delete;

  ~CoarseHashTable();

  ValueType Get(const KeyType &key);

  void Insert(const KeyType &key, const ValueType &value);

  void Delete(const KeyType &key);

  bool Contains(const KeyType &key);

 private:
  size_t KeyToIndex(const KeyType &key) const {
    return std::hash<KeyType>{}(key) % capacity_;
  }

  void GrowHashTable();

  static constexpr size_t DEFAULT_CAPACITY {128};
  static constexpr float DEFAULT_LOAD_FACTOR {0.75};
  size_t capacity_;
  float max_load_factor_;
  std::vector<Entry> *table_;
  size_t size_ {0};
  ReaderWriterLock lock_;
};

#include "coarse_hash_table.cpp"

#endif