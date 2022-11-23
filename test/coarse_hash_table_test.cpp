#include "coarse_hash_table.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <functional>
#include <thread>
#include <utility>
#include <unordered_map>
#include <vector>

static constexpr int NUM_THREADS = 4;
static constexpr int NUM_KEYS = 1000000;

void Test1() {
  std::cout << "-------Test 1-------\n";
  CoarseHashTable<int, int> hash_table;
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
  CoarseHashTable<int, int> hash_table(4, 0.75);

  for (int i = 0; i < 10; ++i) {
    hash_table.Insert(i + 1, i + 1);
  }

  for (int i = 0; i < 10; ++i) {
    if (i % 2 == 0) {
      hash_table.Delete(i);
    }
  }

  assert(hash_table.Contains(5));
  assert(!hash_table.Contains(8));
  assert(hash_table.Get(7) == 7);
}

void do_work(int id, CoarseHashTable<int, int> &hash_table,
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
  CoarseHashTable<int, int> hash_table;
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
    threads.push_back(std::thread(do_work, i, std::ref(hash_table), std::ref(random_kv)));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout
      << "1 millions access (800K reads and 200K writes) on coarse-grained hash table: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
      << " ms \n";

  // Unordered_map
  std::unordered_map<int, int> map;
  for (int i = 0; i < NUM_KEYS; ++i) {
    int k = rand();
    int v = rand();
    map.insert({k, v});
    random_kv.emplace_back(k, v);
  }
  start = std::chrono::steady_clock::now();
  for (int i = 0; i < NUM_KEYS; ++i) {
    if (i % 5 == 0) {
      map.insert({rand(), rand()});
    } else {
      map[random_kv[i].first];
    }
  }
  end = std::chrono::steady_clock::now();
  elapsed = end - start;
  std::cout
      << "1 millions access (800K reads and 200K writes) on unordered_map: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
      << " ms \n";
}

int main() {
  // Test1();
  // Test2();
  Test3();

  std::cout << "All test cases passed\n";

  return 0;
}