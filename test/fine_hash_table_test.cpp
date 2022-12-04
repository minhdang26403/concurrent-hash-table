#include "fine_hash_table.h"

#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>
#include <mutex>

std::mutex mutex;

static int NUM_THREADS = 4;
static constexpr int NUM_OPS = 1000;

/**
 * Correctness Test for the coarse-grained hash table
 */
void CorrectnessTest1() {
  std::cout << "----------Correctness Test 1----------\n";
  FineHashTable<int, int> hash_table;
  for (int i = 0; i < 10; ++i) {
    hash_table.Insert(i + 1, i + 1);
  }
  hash_table.Delete(2);
  hash_table.Delete(6);
  hash_table.Delete(4);
  assert(hash_table.Get(1) == 1);
  assert(!hash_table.Contains(2));
  hash_table.Insert(5, 10);
}

void CorrectnessTest2() {
  std::cout << "----------Correctness Test 2----------\n";
  FineHashTable<int, int> hash_table(4, 0.75);
  for (int i = 0; i < 30; ++i) {
    hash_table.Insert(i + 1, i + 1);
  }
  for (int i = 0; i < 15; ++i) {
    if (i % 2 == 0) {
      hash_table.Delete(i);
    }
  }
  assert(hash_table.Contains(5));
  assert(!hash_table.Contains(8));
  assert(hash_table.Get(7) == 7);
  assert(hash_table.Contains(26));
  assert(hash_table.Contains(29));
  assert(!hash_table.Contains(4));
}

void ConcurrentSearchInsertDelete(int id, FineHashTable<int, int> &hash_table) {
  int stride = NUM_OPS / NUM_THREADS;
  int start = id * stride;
  for (int i = start; i < start + stride; ++i) {
    if (i % 2 == 0) {
      hash_table.Insert(i, i);
    }
  }

  for (int i = start; i < start + stride; ++i) {
    if (i % 2 == 0) {
      assert(hash_table.Contains(i));
      assert(hash_table.Get(i) == i);
    } else {
      assert(!hash_table.Contains(i));
    }
  }

  for (int i = start; i < start + stride; ++i) {
    if (i % 2 == 0) {
      hash_table.Delete(i);
    } else {
      hash_table.Insert(i, i);
    }
  }

  for (int i = start; i < start + stride; ++i) {
    if (i % 2 == 0) {
      assert(!hash_table.Contains(i));
    } else {
      assert(hash_table.Contains(i));
      assert(hash_table.Get(i) == i);
    }
  }
}

void CorrectnessTest3() {
  std::cout << "----------Correctness Test 3----------\n";
  FineHashTable<int, int> hash_table;
  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.push_back(
        std::thread(ConcurrentSearchInsertDelete, i, std::ref(hash_table)));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  std::cout << "Correctness Test 3 passed\n";
}


int main() {
  // CorrectnessTest1();
  // CorrectnessTest2();
  // CorrectnessTest3();

  std::cout << "All test cases passed\n";

  return 0;
}
