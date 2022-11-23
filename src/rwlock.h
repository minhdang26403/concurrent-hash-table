#ifndef RWLOCK_H_
#define RWLOCK_H_

#include <condition_variable>
#include <mutex>

class ReaderWriterLock {
  static constexpr uint32_t MAX_READERS = UINT_MAX;
 public:
  ReaderWriterLock() = default;
  ~ReaderWriterLock() {
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
    while (writer_entered_ || reader_count_ == MAX_READERS) {
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
    if (writer_entered_) {
      if (reader_count_ == 0) {
        writer_.notify_one();
      }
    } else {
      if (reader_count_ == MAX_READERS - 1) {
        reader_.notify_one();
      }
    }
  }

  /**
   * Acquire a write lock
   */
  void WriteLock() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (writer_entered_) {
      reader_.wait(lock);
    }
    writer_entered_ = true;
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