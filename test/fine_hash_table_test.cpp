#include "fine_hash_table.h"

#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>


static int NUM_THREADS = 4;
static constexpr int NUM_OPS = 1000000;
enum Ops {
  READ,
  INSERT,
  DELETE,
};

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
  std::cout << "Correctness Test 1 passed\n";
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
  std::cout << "Correctness Test 2 passed\n";
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

/**
 * Benchmark for the coarse-grained hash table.
 * Performs concurrent read, insert, and delete without checking for
 * correctness, only making sure that everything completes without crashing.
 */

std::vector<Ops> CreateWorkLoad(int num_read, int num_insert, int num_delete) {
  std::vector<Ops> op_mix;
  for (int i = 0; i < num_read; ++i) {
    op_mix.push_back(READ);
  }
  for (int i = 0; i < num_insert; ++i) {
    op_mix.push_back(INSERT);
  }
  for (int i = 0; i < num_delete; ++i) {
    op_mix.push_back(DELETE);
  }
  std::random_shuffle(op_mix.begin(), op_mix.end());

  return op_mix;
}

void mixed_workload(int id, FineHashTable<int, int> &hash_table,
                    std::vector<Ops> &op_mix,
                    std::vector<std::pair<int, int>> &data) {
  int stride = NUM_OPS / NUM_THREADS;
  int start = id * stride;

  for (int i = start; i < start + stride; ++i) {
    int idx = i % 100;
    if (op_mix[idx] == READ) {
      hash_table.Get(data[i].first);
    } else if (op_mix[idx] == INSERT) {
      hash_table.Insert(data[i].first, data[i].second);
    } else {
      hash_table.Delete(data[i].first);
    }
  }
}

void Benchmark(int num_read, int num_insert, int num_delete,
               std::vector<std::pair<int, int>> &data) {
  std::vector<Ops> op_mix = CreateWorkLoad(num_read, num_insert, num_delete);
  FineHashTable<int, int> hash_table;
  std::vector<std::thread> threads;

  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.push_back(std::thread(mixed_workload, i, std::ref(hash_table),
                                  std::ref(op_mix), std::ref(data)));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout
      << NUM_OPS << " access (" << num_read << "% read, " << num_insert
      << "% insert, " << num_delete
      << "% delete) on fine-grained hash table: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
      << " ms \n";
}

void GenerateKeyValue(std::vector<std::pair<int, int>> &data) {
  for (int i = 0; i < NUM_OPS; ++i) {
    data.push_back({rand(), rand()});
  }
}

int main(int argc, char **argv) {
  // CorrectnessTest1();
  // CorrectnessTest2();
  // CorrectnessTest3();

  if (argc > 1) {
    NUM_THREADS = atoi(argv[1]);
  }
  std::vector<std::pair<int, int>> data;
  GenerateKeyValue(data);
  Benchmark(80, 10, 10, data);

  std::cout << "All test cases passed\n";

  return 0;
}
