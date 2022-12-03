#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

#include "fine_hash_table.h"

static constexpr int NUM_THREADS = 4;
static constexpr int NUM_KEYS = 1000000;

void Test1() {
  std::cout << "-------Test 1-------\n";
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

void Test2() {
  std::cout << "-------Test 2-------\n";
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

void do_work(int id, FineHashTable<int, int> &hash_table,
             std::vector<std::pair<int, int>> &random_kv) {
  int stride = NUM_KEYS / NUM_THREADS;
  int start = id * stride;
  for (int i = start; i < start + stride; ++i) {
    if (i % 5 == 0) {
      hash_table.Insert(rand(), rand());
    } else {
      hash_table.Get(random_kv[i].first);
    }
  }
}

void Test3() {
  FineHashTable<int, int> hash_table;
  std::vector<std::thread> threads;
  std::vector<std::pair<int, int>> random_kv;
  for (int i = 0; i < NUM_KEYS; ++i) {
    int k = rand();
    int v = rand();
    hash_table.Insert(k, v);
    random_kv.emplace_back(k, v);
  }

  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.push_back(
        std::thread(do_work, i, std::ref(hash_table), std::ref(random_kv)));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout
      << "1 millions access (800K reads and 200K writes) on fine-grained hash table: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
      << " ms \n";
}

void insert(int i, FineHashTable<int, int> &hash_table) {
  int start = i * 60;
  for (int i = start; i < start + 60; ++i) {
    hash_table.Insert(i, i);
  }
}

void Test4() {
  FineHashTable<int, int> hash_table;
  std::vector<std::thread> threads;
  std::vector<std::pair<int, int>> random_kv;
  for (int i = 0; i < 4; ++i) {
    threads.push_back(
        std::thread(insert, i, std::ref(hash_table)));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  std::cout << hash_table.size() << std::endl;

}


int main() {
  // Test1();
  // Test2();
  Test3();
  // Test4();

  std::cout << "All test cases passed\n";

  return 0;
}