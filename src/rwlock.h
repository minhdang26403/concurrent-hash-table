#ifndef RWLOCK_H_
#define RWLOCK_H_

#include <condition_variable>
#include <mutex>

class ReaderWriterLock {
 public:
  ReaderWriterLock() = default;
  ~ReaderWriterLock() {
    // Try to get the lock before destroying the mutex
    std::lock_guard<std::mutex> lock(mutex_);
  }

  /**
   * Disallow copy
   */
  ReaderWriterLock(const ReaderWriterLock& other) = delete;
  ReaderWriterLock& operator=(const ReaderWriterLock& other) = delete;

  /**
   * Acquire a read lock
   */
  void ReadLock() {
    std::unique_lock<std::mutex> lock(mutex_);
    // Readers have to wait if at least one writer comes
    while (writer_entered_) {
      reader_.wait(lock);
    }
    ++reader_count_;
  }

  /**
   * Release a read lock
   */
  void ReadUnlock() {
    std::lock_guard<std::mutex> lock(mutex_);
    --reader_count_;
    // If there is a writer waiting to acquire the lock
    // and this is the last reader, we notify other writers
    if (writer_entered_ && reader_count_ == 0) {
        writer_.notify_one();
    }
  }

  /**
   * Acquire a write lock
   */
  void WriteLock() {
    std::unique_lock<std::mutex> lock(mutex_);
    // Wait one the same variable as readers
    while (writer_entered_) {
      reader_.wait(lock);
    }
    writer_entered_ = true;
    // Wait for readers to complete
    while (reader_count_ > 0) {
      writer_.wait(lock);
    }
  }

  /**
   * Release a write lock
   */
  void WriteUnlock() {
    std::lock_guard<std::mutex> lock(mutex_);
    writer_entered_ = false;
    reader_.notify_all();
  }

 private:
  // A mutex protects shared variables
  std::mutex mutex_;
  std::condition_variable writer_;
  std::condition_variable reader_;
  uint32_t reader_count_ {0};
  bool writer_entered_ {false};
};

#endif // RWLOCK_H_