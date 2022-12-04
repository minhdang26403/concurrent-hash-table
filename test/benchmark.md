# Benchmark

## Unordered_map

- 1000000 access (90% read, 5% insert, 5% delete) on std::unordered_map: 510 ms
- 1000000 access (80% read, 10% insert, 10% delete) on std::unordered_map: 495 ms
- 1000000 access (40% read, 30% insert, 30% delete) on std::unordered_map: 360 ms

## Coarse-grained hash table

- 1000000 access (80% read, 10% insert, 10% delete) on coarse-grained hash table: 
  - 2 threads: 595 ms
  - 4 threads: 650 ms
  - 6 threads: 720 ms
  - 8 threads: 730 ms
  - 10 threads: 760 ms
  - 12 threads: 783 ms

## Fine-grained hash table

- 1000000 access (80% read, 10% insert, 10% delete) on coarse-grained hash table: 
  - 2 threads: 253 ms
  - 4 threads: 240 ms
  - 6 threads: 234 ms
  - 8 threads: 240 ms
  - 10 threads: 277 ms
  - 12 threads: 293 ms

## Lock-free hash table

- 1000000 access (80% read, 10% insert, 10% delete) on coarse-grained hash table: 
  - 2 threads: 51 ms
  - 4 threads: 28 ms
  - 6 threads: 31 ms
  - 8 threads: 24 ms
  - 10 threads: 20 ms
  - 12 threads: 20 ms


Further data may be collected for the paper.