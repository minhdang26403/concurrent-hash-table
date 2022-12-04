#include <algorithm>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

static constexpr int NUM_OPS = 1000000;
enum Ops {
  READ,
  INSERT,
  DELETE,
};

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

void Benchmark(int num_read, int num_insert, int num_delete,
               std::vector<std::pair<int, int>> &data) {
  std::unordered_map<int, int> map;
  std::vector<Ops> op_mix = CreateWorkLoad(num_read, num_insert, num_delete);

  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < NUM_OPS; ++i) {
    int idx = i % 100;
    if (op_mix[idx] == READ) {
      map[data[i].first];
    } else if (op_mix[idx] == INSERT) {
      map.insert({data[i].first, data[i].second});
    } else {
      map.erase(data[i].first);
    }
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout
      << NUM_OPS << " access (" << num_read << "% read, " << num_insert
      << "% insert, " << num_delete << "% delete) on std::unordered_map: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
      << " ms \n";
}

void GenerateKeyValue(std::vector<std::pair<int, int>> &data) {
  for (int i = 0; i < NUM_OPS; ++i) {
    data.push_back({rand(), rand()});
  }
}

int main() {
  std::vector<std::pair<int, int>> data;
  GenerateKeyValue(data);
  Benchmark(90, 5, 5, data);
  Benchmark(80, 10, 10, data);
  Benchmark(40, 30, 30, data);

  return 0; 
}