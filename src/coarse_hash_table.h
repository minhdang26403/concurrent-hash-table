#ifndef COARSE_HASH_TABLE_H_
#define COARSE_HASH_TABLE_H_

#include <list>
#include <vector>

#include "rwlock.h"

/**
 * Coarse-grained hash table with one global reader/writer lock
 */
template<typename KeyType, typename ValueType>
class CoarseHashTable {
 private:
  struct Entry {
    KeyType key_{};
    ValueType value_{};

    Entry() = default;

    /**
     * Creates an Entry instance
     * @param key the key of the entry
     * @param value the value of the entry
     */
    Entry(const KeyType &key, const ValueType &value)
        : key_(key), value_(value) {}
  };

 public:
  /**
   * Default constructor
   */
  CoarseHashTable() : CoarseHashTable(DEFAULT_CAPACITY, DEFAULT_LOAD_FACTOR) {}

  /**
   * Creates a new CoarseHashTable instance
   * @param capacity the maximum bucket in the hash table
   * @param max_load_factor the maximum load factor (the average number of
   * elements per bucket)
   */
  CoarseHashTable(size_t capacity, float max_load_factor)
      : capacity_(capacity),
        max_load_factor_(max_load_factor),
        table_(new std::vector<Entry>[capacity_]) {}

  /**
   * Disallows copy
   */
  CoarseHashTable(const CoarseHashTable &other) = delete;
  CoarseHashTable& operator=(const CoarseHashTable &other) = delete;

  /**
   * Destroys an existing CoarseHashTable instance.
   */
  ~CoarseHashTable();

  /**
   * Gets the value of a key-value pair
   * @param key the key of the key-value pair
   * @return the value of that key
   */
  ValueType Get(const KeyType &key);

  /**
   * Inserts a key-value pair into the hash table
   * @param key the key to insert
   * @param value the value to insert
   */
  void Insert(const KeyType &key, const ValueType &value);

  /**
   * Deletes a key-value pair from the hash table
   * @param key the key to delete
   */
  void Delete(const KeyType &key);

  /**
   * Checks if a key exists in the hash table
   * @param key the key to check
   * @return true if that key exists; otherwise, false
   */
  bool Contains(const KeyType &key);

 private:
  /**
   * Calculates the index into the hash table given a key
   * @param key the key to calculate index from
   * @return the index into the hash table
   */
  size_t KeyToIndex(const KeyType &key) const {
    return std::hash<KeyType>{}(key) % capacity_;
  }

  /**
   * Grows the hash table (doubles the number of buckets) when the hash table
   * gets dense
   */
  void GrowHashTable();

  // Default number of buckets
  static constexpr size_t DEFAULT_CAPACITY {128};
  static constexpr float DEFAULT_LOAD_FACTOR {0.75};
  size_t capacity_;
  float max_load_factor_;
  // An array of buckets
  std::vector<Entry> *table_;
  // The current number of key-value pairs in the hash table
  size_t size_ {0};
  // The global reader/writer lock
  ReaderWriterLock lock_;
};

#include "coarse_hash_table.cpp"

#endif