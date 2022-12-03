
#include "lock_free_hash_table.h"
#include <vector>
#include <thread>

static constexpr int NUM_THREADS = 4;
static constexpr int NUM_KEYS = 1000;

void Test1() {
  LockFreeHashTable<int, int> l;

  for (int i = 1; i < 10; ++i) {
    l.Insert(i, i);
  }

  l.Insert(1, 1);
  l.Delete(2);
  l.Delete(3);
  l.Delete(1);
  l.Delete(9);

  // l.Print();
}

void do_work(int id, AtomicLinkedList<int, int> &hash_table,
             std::vector<std::pair<int, int>> &random_kv) {
  int stride = NUM_KEYS / NUM_THREADS;
  int start = id * stride;
  for (int i = start; i < start + stride; ++i) {
    if (i % 5 == 0) {
      hash_table.Insert(rand(), rand());
    } else {
      hash_table.Search(random_kv[i].first);
    }
  }
}

void Test2() {
  AtomicLinkedList<int, int> hash_table;
  std::vector<std::thread> threads;
  std::vector<std::pair<int, int>> random_kv;
  // Prep data
  for (int i = 0; i < NUM_KEYS; ++i) {
    int k = rand();
    int v = rand();
    hash_table.Insert(k, v);
    random_kv.emplace_back(k, v);
  }

  // hash_table.Print();

  
  // auto start = std::chrono::steady_clock::now();
  // for (int i = 0; i < NUM_THREADS; ++i) {
  //   threads.push_back(
  //       std::thread(do_work, i, std::ref(hash_table), std::ref(random_kv)));
  // }

  // for (auto &thread : threads) {
  //   thread.join();
  // }

  // auto end = std::chrono::steady_clock::now();
  // std::chrono::duration<double> elapsed = end - start;
  // std::cout
  //     << "1 millions access (800K reads and 200K writes) on lock-free hash table: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
  //     << " ms \n";
}


int main() {
  // Test1();
  Test2();


  return 0;
}