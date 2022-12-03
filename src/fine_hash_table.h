#ifndef FINE_HASH_TABLE_H_
#define FINE_HASH_TABLE_H_

#include "rwlock.h"

#include <atomic>
#include <list>


/**
 * Bucket object of a hash table
 */
template<typename KeyType, typename ValueType>
class Bucket {
 private:
  struct Entry {
    KeyType key_;
    ValueType value_;

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
   * Move assignment operator
   * @param other the bucket to move from
   * @return a reference to this bucket
   */
  Bucket& operator=(Bucket &other);

  /**
   * Gets the value of a key within the current bucket
   * @param key the key to retrieve
   * @return the value of that key
   */
  ValueType GetKV(const KeyType &key);

  /**
   * Checks if the current bucket has a specified key
   * @param key the key to check
   * @return true if this bucket contains that key; otherwise, returns false
   */
  bool ContainsKV(const KeyType &key);

  /**
   * Inserts a key-value pair into this bucket
   * @param key the key to insert
   * @param value the value to insert
   */
  bool InsertKV(const KeyType &key, const ValueType &value);

  /**
   * Deletes a key-value pair from this bucket
   * @param key the key to delete
   */
  bool DeleteKV(const KeyType &key);

  std::vector<Entry>& GetKVList() { return list_; }
 private: 
  // A private lock of each bucket
  ReaderWriterLock lock_;
  // A chain within each bucket (use vector for better locality)
  std::vector<Entry> list_;
};


/**
 * Fine-grained hash table where each bucket has its own reader/writer lock
 */
template<typename KeyType, typename ValueType>
class FineHashTable {
 public:
  /**
   * Default constructor
   */
  FineHashTable() : FineHashTable(DEFAULT_CAPACITY, DEFAULT_LOAD_FACTOR) {}

  /**
   * Creates a new FineHashTable instance
   * @param capacity the maximum bucket in the hash table
   * @param max_load_factor the maximum load factor (the average number of
   * elements per bucket)
   */
  FineHashTable(size_t capacity, float max_load_factor)
      : capacity_(capacity),
        max_load_factor_(max_load_factor),
        table_(new Bucket<KeyType, ValueType>[capacity_]) {}

  /**
   * Destroys an existing FineHashTable instance
   */
  ~FineHashTable();

  size_t size() const { return size_; }

  /**
   * Gets the value of a key-value pair
   * @param key the key of the key-value pair
   * @return the value of that key
   */
  ValueType Get(const KeyType &key);

  /**
   * Checks if a key exists in the hash table
   * @param key the key to check
   * @return true if that key exists; otherwise, false
   */
  bool Contains(const KeyType &key);

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
  static constexpr size_t DEFAULT_CAPACITY{128};
  static constexpr float DEFAULT_LOAD_FACTOR{0.75};
  size_t capacity_;
  float max_load_factor_;
  // An array of buckets
  Bucket<KeyType, ValueType> *table_;
  // The current number of key-value pairs in the hash table
  std::atomic<size_t> size_{0};
  // size_t size_{0};

  // One global reader/writer lock: a writer lock is used when growing the hash
  // table while other procedure use a reader lock
  ReaderWriterLock global_lock_;
};

#include "fine_hash_table.cpp"

#endif // FINE_HASH_TABLE_H_