# Benchmark

## Unordered_map

- 1000000 access (90% read, 5% insert, 5% delete) on std::unordered_map: 510 ms
- 1000000 access (80% read, 10% insert, 10% delete) on std::unordered_map: 495 ms
- 1000000 access (40% read, 30% insert, 30% delete) on std::unordered_map: 360 ms

## Coarse-grained hash table

- 1000000 access (80% read, 10% insert, 10% delete) on coarse-grained hash table: 
  - 2 threads: 477 ms
  - 4 threads: 532 ms
  - 6 threads: 547 ms
  - 8 threads: 554 ms
  - 10 threads: 564 ms
  - 12 threads: 570 ms

## Fine-grained hash table

- 1000000 access (80% read, 10% insert, 10% delete) on coarse-grained hash table: 
  - 2 threads: 182 ms
  - 4 threads: 189 ms
  - 6 threads: 205 ms
  - 8 threads: 190 ms
  - 10 threads: 197 ms
  - 12 threads: 196 ms

## Lock-free hash table

- 1000000 access (80% read, 10% insert, 10% delete) on coarse-grained hash table: 
  - 2 threads: 40 ms
  - 4 threads: 22 ms
  - 6 threads: 16 ms
  - 8 threads: 14 ms
  - 10 threads: 16 ms
  - 12 threads: 14 ms


Further data may be collected for the paper.
