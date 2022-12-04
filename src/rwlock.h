#ifndef RWLOCK_H_
#define RWLOCK_H_


#include <shared_mutex>

class ReaderWriterLock {
 public:
  /**
   * Acquire a read lock
   */
  void ReadLock() {
    mutex_.lock_shared();
  }

  /**
   * Release a read lock
   */
  void ReadUnlock() {
    mutex_.unlock_shared();
  }

  /**
   * Acquire a write lock
   */
  void WriteLock() {
    mutex_.lock();
  }

  /**
   * Release a write lock
   */
  void WriteUnlock() {
    mutex_.unlock();
  }

 private:
  // C++17 implementation for reader/writer lock
  std::shared_mutex mutex_;
};

#endif // RWLOCK_H_