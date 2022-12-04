#ifndef LOCK_FREE_HASH_TABLE_H_
#define LOCK_FREE_HASH_TABLE_H_


#include "atomic_linked_list.h"
#include "rwlock.h"

template <typename KeyType, typename ValueType>
class LockFreeHashTable {
 public:
  /**
   * Default constructor
   */
  LockFreeHashTable()
      : LockFreeHashTable(DEFAULT_CAPACITY, DEFAULT_LOAD_FACTOR) {}

  /**
   * Creates a new LockFreeHashTable instance
   * @param capacity the maximum bucket in the hash table
   * @param max_load_factor the maximum load factor (the average number of
   * elements per bucket)
   */
  LockFreeHashTable(size_t capacity, float max_load_factor)
      : capacity_(capacity),
        max_load_factor_(max_load_factor),
        table_(new AtomicLinkedList<int, int>[capacity_]) {}

  /**
   * Disallows copy
   */
  LockFreeHashTable(const LockFreeHashTable &other) = delete;
  LockFreeHashTable &operator=(const LockFreeHashTable &other) = delete;

  /**
   * Destroys an existing LockFreeHashTable instance.
   */
  ~LockFreeHashTable();

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

  // Default size of the hash table
  static constexpr size_t DEFAULT_CAPACITY{100013};
  static constexpr float DEFAULT_LOAD_FACTOR{0.75};

  size_t capacity_; // number of buckets
  float max_load_factor_;
  std::atomic<size_t> size_{0};  // current number of key-value pairs in the hash table
  AtomicLinkedList<KeyType, ValueType> *table_; // array of buckets
  ReaderWriterLock lock_; // global reader/writer lock
};

#include "lock_free_hash_table.cpp"

#endif  // LOCK_FREE_HASH_TABLE_H_