# cpp-fastchan

High performance SPSC and MPSC ringbuffers which can be blocking/non-blocking on gets, puts, or both. For blocking queue it's also possibe to spin wait, no op, or yield thread execution. Optimized for x64 architecture.

If the size provided is not a power if 2, it's rounded up to the next power of 2.

# Usage

```cpp
// SPSC
fastchan::SPSC<int, chan_size> c;
// OR
fastchan::SPSC<int, chan_size, fastchan::PauseWaitStrategy, fastchan::PauseWaitStrategy> c;

c.put(0);
c.put(1);
c.put(2);
c.put(3);

auto val = c.get();
```

```cpp
// MPSC
fastchan::MPSC<int, chan_size> c;
// OR
fastchan::MPSC<int, chan_size, fastchan::PauseWaitStrategy, fastchan::PauseWaitStrategy> c;

c.put(0);
c.put(1);
c.put(2);
c.put(3);

auto val = c.get();

```

## Benchmark

There's a comparison benchmark comparing SPSC to Rigtorp in all comparable wait strategies (except CV). Feel free to run it yourself. The entire suite runs twice to make sure the comparisons are reliable. Here are the indicative results. Threads are pinned to a given set of cores per iteration:

```
fastchan_SPSC_Pause
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       1.7       1.7       1.8       5.3       2.8       5.3       5.3       2.8       3.1       5.3       2.8
    CPU  1       1.4       N/A       1.4       1.6       3.9       3.9       3.9       3.9       3.9       1.9       3.9       3.9
    CPU  2       1.5       1.5       N/A       1.5       4.4       2.2       2.3       4.5       4.4       2.3       4.4       2.2
    CPU  3       1.5       1.6       1.5       N/A       4.5       4.4       4.5       2.1       4.4       4.4       4.4       2.0
    CPU  4       1.4       1.7       1.6       1.7       N/A       1.0       1.1       1.0       4.9       5.3       2.8       2.6
    CPU  5       1.6       1.8       1.6       1.8       2.6       N/A       1.0       2.6       4.9       2.7       4.9       2.5
    CPU  6       1.4       1.5       1.6       2.0       2.6       2.6       N/A       1.0       4.8       2.8       4.8       2.5
    CPU  7       1.4       1.6       1.6       1.7       1.0       1.0       2.6       N/A       4.8       4.9       2.9       2.6
    CPU  8       1.5       1.6       1.7       1.8       3.0       5.6       6.2       6.2       N/A       1.0       1.0       1.0
    CPU  9       1.5       1.6       1.7       1.8       5.8       3.0       5.9       5.9       1.0       N/A       1.0       1.0
    CPU 10       1.5       1.6       1.7       1.8       3.4       5.8       5.7       5.8       1.0       2.6       N/A       2.6
    CPU 11       1.6       1.6       1.7       1.9       3.3       5.8       5.8       6.0       1.0       1.0       2.6       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.0, Producer CPU: 10, Consumer CPU:  9
Cost: 1.0, Producer CPU: 11, Consumer CPU:  8
Cost: 1.0, Producer CPU:  9, Consumer CPU: 11
Cost: 1.0, Producer CPU:  8, Consumer CPU: 11
Cost: 1.0, Producer CPU:  8, Consumer CPU:  9

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 6.2, Producer CPU:  7, Consumer CPU:  8
Cost: 6.2, Producer CPU:  6, Consumer CPU:  8
Cost: 6.0, Producer CPU:  7, Consumer CPU: 11
Cost: 5.9, Producer CPU:  7, Consumer CPU:  9
Cost: 5.9, Producer CPU:  6, Consumer CPU:  9
============================
Rigtorp_SPSC_Pause
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       2.2       2.6       2.6      15.8      15.7      16.0      15.8      15.5      15.3      15.5      15.3
    CPU  1       2.4       N/A       2.4       2.8      15.9      15.9      16.0      15.8      15.4      15.5      15.4      15.4
    CPU  2       1.7       1.6       N/A       1.7       7.5       7.5       7.4       7.5       7.2       7.2       7.2       7.2
    CPU  3       1.7       1.8       2.0       N/A      10.4      10.4      10.4      10.4       9.9       9.9       9.9       9.9
    CPU  4       6.1       5.8       5.3       5.3       N/A       5.3       7.0       5.6       4.2       4.2       4.4       4.5
    CPU  5       5.6       5.4       5.6       6.0       5.3       N/A       5.3       5.4       4.2       4.2       4.2       4.2
    CPU  6       5.6       5.4       5.3       5.3       5.4       5.3       N/A       5.3       4.2       4.2       4.2       4.2
    CPU  7       5.5       5.4       5.3       5.3       5.3       5.3       5.4       N/A       4.2       4.2       4.2       4.2
    CPU  8       5.4       5.4       5.4       5.2       4.1       4.1       4.1       4.1       N/A       5.3       5.3       5.3
    CPU  9       5.4       5.4       5.3       5.3       4.1       4.1       4.1       4.8       9.4       N/A       9.4       9.4
    CPU 10       5.4       5.4       5.4       5.2       4.1       4.1       4.1       4.1       5.4       5.4       N/A       5.4
    CPU 11       5.4       5.4       5.3      11.2       4.1       4.1       4.1       4.1       5.3       8.2       5.4       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.6, Producer CPU:  1, Consumer CPU:  2
Cost: 1.7, Producer CPU:  0, Consumer CPU:  2
Cost: 1.7, Producer CPU:  0, Consumer CPU:  3
Cost: 1.7, Producer CPU:  3, Consumer CPU:  2
Cost: 1.8, Producer CPU:  1, Consumer CPU:  3

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 16.0, Producer CPU:  6, Consumer CPU:  0
Cost: 16.0, Producer CPU:  6, Consumer CPU:  1
Cost: 15.9, Producer CPU:  5, Consumer CPU:  1
Cost: 15.9, Producer CPU:  4, Consumer CPU:  1
Cost: 15.8, Producer CPU:  7, Consumer CPU:  1
============================
fastchan_SPSC_NoOp
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A      15.9      12.7      11.6      17.9      15.5      15.4      17.9      16.4      15.5      15.6      15.5
    CPU  1       6.3       N/A      10.5      10.5       7.4       7.9       7.9       7.9       7.2       7.5       7.2       7.2
    CPU  2       7.7      13.8       N/A      13.9      13.0      13.9      13.0      13.2      12.5      12.6      12.5      12.5
    CPU  3       7.2      17.0      12.8       N/A      14.0      15.0      14.9      14.8      13.5      13.7      13.4      14.5
    CPU  4      13.7      12.2       7.6      11.6       N/A       1.9       2.7       2.7      17.0       2.1      17.0      16.9
    CPU  5      12.4      12.0       7.6      11.6       1.9       N/A       1.9       2.0      16.9       3.7       4.0       2.2
    CPU  6      12.5      11.7       7.7      11.9       2.7       2.0       N/A       2.7       2.7      16.9      17.1      16.9
    CPU  7      12.4      13.8       7.6      12.3       1.9       2.7       2.7       N/A       2.4       2.5      16.9      17.1
    CPU  8      11.3      10.8       8.1      11.7       4.8      19.2       2.8       4.4       N/A       1.9       2.7       2.7
    CPU  9      11.4      11.0       8.1      11.6      19.1      19.2       1.9       3.9       2.7       N/A       2.7       2.8
    CPU 10      11.5      10.9       8.0      11.5      19.3      19.2      19.2      19.2       2.7       2.7       N/A       2.7
    CPU 11      12.3      11.6       8.1      11.7      19.1       3.8      19.2      19.1       2.7       2.7       2.7       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.9, Producer CPU:  5, Consumer CPU:  4
Cost: 1.9, Producer CPU:  4, Consumer CPU:  7
Cost: 1.9, Producer CPU:  6, Consumer CPU:  5
Cost: 1.9, Producer CPU:  9, Consumer CPU:  8
Cost: 1.9, Producer CPU:  4, Consumer CPU:  5

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 19.3, Producer CPU:  4, Consumer CPU: 10
Cost: 19.2, Producer CPU:  5, Consumer CPU:  8
Cost: 19.2, Producer CPU:  5, Consumer CPU:  9
Cost: 19.2, Producer CPU:  6, Consumer CPU: 10
Cost: 19.2, Producer CPU:  7, Consumer CPU: 10
============================
rigtorp_SPSC_NoOp
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       2.0       1.8       1.8      14.9      15.1      14.5      14.6      14.0      14.6      13.5      13.6
    CPU  1       2.0       N/A       2.9       2.9      14.7      14.2      14.3      14.2      13.2      13.5      13.3      13.5
    CPU  2       1.6       3.4       N/A       1.9       7.4       7.2       7.3       7.1       7.0       6.9       7.0       6.9
    CPU  3       1.6       3.7       2.0       N/A       9.3       9.5       9.5       9.4       9.5       9.5       9.6       9.5
    CPU  4       7.0       6.7       7.5       7.0       N/A       9.2       8.9       9.2       7.0       7.0       7.0       7.0
    CPU  5       7.0       6.6       7.5       7.1       5.6       N/A       6.3      10.0       7.0       6.0       6.6       7.0
    CPU  6       7.0       6.6       7.5       7.1       6.7       8.8       N/A       6.4       7.0       7.0       6.7       6.0
    CPU  7       7.0       6.6       7.5       7.0       9.2       9.4       9.4       N/A       6.9       6.0       6.2       7.0
    CPU  8       6.5       6.4       7.2       7.2       6.8       7.6       7.6       7.9       N/A      10.8      10.8      10.8
    CPU  9       6.6       6.4       7.2       7.2       7.9       7.8       7.9       7.9      10.8       N/A      10.8      10.8
    CPU 10       6.5       6.4       7.2       7.2       6.2       7.9       7.8       7.9      10.8      10.8       N/A      10.8
    CPU 11       6.6       6.4       7.2       7.2       7.8       7.9       7.8       7.8      10.8      10.8       9.8       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.6, Producer CPU:  0, Consumer CPU:  3
Cost: 1.6, Producer CPU:  0, Consumer CPU:  2
Cost: 1.8, Producer CPU:  2, Consumer CPU:  0
Cost: 1.8, Producer CPU:  3, Consumer CPU:  0
Cost: 1.9, Producer CPU:  3, Consumer CPU:  2

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 15.1, Producer CPU:  5, Consumer CPU:  0
Cost: 14.9, Producer CPU:  4, Consumer CPU:  0
Cost: 14.7, Producer CPU:  4, Consumer CPU:  1
Cost: 14.6, Producer CPU:  7, Consumer CPU:  0
Cost: 14.6, Producer CPU:  9, Consumer CPU:  0
============================
fastchan_SPSC_Yield
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       1.3       1.3       1.3       3.3       1.3       3.3       1.4       3.4       3.3       1.3       1.3
    CPU  1       1.2       N/A       1.2       1.3       1.2       3.0       1.2       1.2       3.0       3.0       3.0       1.2
    CPU  2       1.2       1.2       N/A       1.2       3.1       3.1       1.3       1.2       3.1       3.1       3.1       1.2
    CPU  3       1.2       1.3       1.2       N/A       1.2       2.9       3.1       3.1       3.1       1.2       1.2       3.1
    CPU  4       1.2       1.2       1.2       1.2       N/A       1.0       2.3       1.0       1.3       1.3       1.3       1.2
    CPU  5       1.2       1.2       1.2       1.2       1.0       N/A       1.0       1.1       1.3       1.3       1.2       3.0
    CPU  6       1.2       1.2       1.2       1.2       1.4       2.3       N/A       1.1       1.3       1.3       1.3       3.0
    CPU  7       1.2       1.2       1.2       1.2       2.3       2.3       2.2       N/A       1.2       1.3       1.2       1.3
    CPU  8       1.2       1.2       1.2       1.2       1.3       1.3       1.3       1.3       N/A       1.0       2.3       2.3
    CPU  9       1.2       1.2       1.2       1.2       3.1       3.1       1.3       3.1       2.3       N/A       2.3       2.3
    CPU 10       1.2       1.2       1.2       1.2       1.3       1.3       1.3       3.1       1.0       2.3       N/A       2.3
    CPU 11       1.2       1.2       1.2       1.2       3.1       1.3       1.3       3.1       2.3       1.0       1.0       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.0, Producer CPU:  5, Consumer CPU:  4
Cost: 1.0, Producer CPU:  9, Consumer CPU:  8
Cost: 1.0, Producer CPU:  9, Consumer CPU: 11
Cost: 1.0, Producer CPU:  6, Consumer CPU:  5
Cost: 1.0, Producer CPU:  4, Consumer CPU:  5

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 3.4, Producer CPU:  8, Consumer CPU:  0
Cost: 3.3, Producer CPU:  4, Consumer CPU:  0
Cost: 3.3, Producer CPU:  6, Consumer CPU:  0
Cost: 3.3, Producer CPU:  9, Consumer CPU:  0
Cost: 3.1, Producer CPU:  5, Consumer CPU:  2
============================
rigtorp_SPSC_Yield
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       2.0       2.2       2.4       9.5       9.4       9.2       9.2       8.9       9.0       8.8       8.9
    CPU  1       1.9       N/A       2.0       2.2       9.2       9.3       9.2       9.1       8.9       8.8       8.9       8.9
    CPU  2       1.3       1.3       N/A       1.4       3.8       3.8       3.9       3.8       3.7       3.8       3.8       3.8
    CPU  3       1.3       1.6       1.7       N/A       4.3       4.5       4.3       4.3       4.4       4.4       4.5       4.3
    CPU  4       7.7       7.2       6.5       6.8       N/A       6.9       6.9       6.9       5.4       5.4       5.4       5.4
    CPU  5       7.6       7.3       6.5       6.8       6.9       N/A       6.9       6.9       5.4       5.4       5.4       5.4
    CPU  6       7.6       7.3       6.5       6.8       6.9       6.9       N/A       6.9       5.4       5.4       5.4       5.4
    CPU  7       7.6       7.3       6.5       6.8       6.9       6.9       6.9       N/A       5.4       5.4       5.4       5.4
    CPU  8       7.3       7.1       6.3       6.5       5.7       5.6       5.7       5.7       N/A       7.1       7.2       7.1
    CPU  9       7.2       7.1       6.3       6.6       5.7       5.7       5.6       5.7       5.7       N/A       7.2       7.1
    CPU 10       7.3       7.2       6.3       6.5       5.6       5.6       5.6       5.6       7.2       7.2       N/A       7.2
    CPU 11       7.2       7.1       6.2       6.6       5.6       5.7       5.7       5.6       7.2       7.2       7.2       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.3, Producer CPU:  0, Consumer CPU:  3
Cost: 1.3, Producer CPU:  0, Consumer CPU:  2
Cost: 1.3, Producer CPU:  1, Consumer CPU:  2
Cost: 1.4, Producer CPU:  3, Consumer CPU:  2
Cost: 1.6, Producer CPU:  1, Consumer CPU:  3

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 9.5, Producer CPU:  4, Consumer CPU:  0
Cost: 9.4, Producer CPU:  5, Consumer CPU:  0
Cost: 9.3, Producer CPU:  5, Consumer CPU:  1
Cost: 9.2, Producer CPU:  7, Consumer CPU:  0
Cost: 9.2, Producer CPU:  6, Consumer CPU:  0
============================
SPSC_CV
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A      15.5      14.7      19.8      25.6      28.9      28.8      28.8      26.9      26.9      27.0      26.8
    CPU  1       7.4       N/A       6.8       7.3      24.3      24.2      24.3      24.2      28.6      28.5      28.7      28.7
    CPU  2       9.3       7.3       N/A      12.7      30.7      30.4      30.5      30.2      30.4      30.2      30.5      30.1
    CPU  3      10.3       9.2      13.3       N/A      27.3      29.9      29.7      29.6      25.3      25.5      25.3      25.2
    CPU  4       6.9       6.7       6.8       9.1       N/A       5.7       5.7       5.7       6.4       7.0       6.0       7.0
    CPU  5       8.0       6.8       6.9       7.2       8.0       N/A       8.0       8.0      11.5      11.5      11.4      11.4
    CPU  6       9.7       7.1       7.0       7.3       8.0       8.0       N/A       8.0      11.6      11.5      11.4      11.4
    CPU  7       7.5       9.1       8.7       9.3       8.0       8.0       8.0       N/A      11.5      11.4      11.5      11.5
    CPU  8       7.5       6.9       6.8       7.1      12.0      12.0      11.9      12.1       N/A       7.9       7.9       7.9
    CPU  9       7.5       7.0       6.8       7.0      11.9      12.0      11.9      12.0       5.7       N/A       5.7       5.7
    CPU 10       7.6       7.0       6.7       8.6       7.9       6.3       7.1       8.6       5.7       5.7       N/A       8.0
    CPU 11       7.5       6.9       6.7       7.0      10.6       8.7       6.3       7.5       5.7       5.7       8.0       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 5.7, Producer CPU: 11, Consumer CPU:  9
Cost: 5.7, Producer CPU:  6, Consumer CPU:  4
Cost: 5.7, Producer CPU:  8, Consumer CPU:  9
Cost: 5.7, Producer CPU:  5, Consumer CPU:  4
Cost: 5.7, Producer CPU:  8, Consumer CPU: 11

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 30.7, Producer CPU:  4, Consumer CPU:  2
Cost: 30.5, Producer CPU: 10, Consumer CPU:  2
Cost: 30.5, Producer CPU:  6, Consumer CPU:  2
Cost: 30.4, Producer CPU:  5, Consumer CPU:  2
Cost: 30.4, Producer CPU:  8, Consumer CPU:  2
fastchan_SPSC_Pause
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       1.7       1.8       1.7       5.2       5.3       5.3       5.3       5.3       3.0       3.0       5.3
    CPU  1       1.5       N/A       1.6       1.6       3.9       3.9       2.0       1.9       2.0       3.9       2.0       1.9
    CPU  2       1.6       1.5       N/A       1.5       4.5       4.4       2.2       4.5       4.4       2.1       4.4       4.4
    CPU  3       1.5       1.6       1.6       N/A       4.5       4.4       2.0       2.2       2.0       2.1       4.4       2.1
    CPU  4       1.5       1.7       1.6       1.7       N/A       1.0       2.6       1.0       4.8       3.0       2.8       4.9
    CPU  5       1.5       1.6       1.6       1.7       2.6       N/A       2.6       2.6       4.9       2.3       3.1       4.8
    CPU  6       1.4       1.6       1.6       1.7       1.0       2.6       N/A       1.0       2.8       4.9       4.6       2.5
    CPU  7       1.4       1.6       1.6       1.7       1.0       2.6       1.0       N/A       4.7       2.9       4.8       4.7
    CPU  8       1.5       1.7       1.7       1.8       5.8       3.0       3.3       5.5       N/A       1.0       2.6       1.0
    CPU  9       1.5       1.8       1.7       1.8       5.8       3.3       3.2       5.8       1.0       N/A       2.6       2.6
    CPU 10       1.5       1.9       1.7       1.9       3.4       5.8       5.8       5.9       1.0       2.6       N/A       1.0
    CPU 11       1.5       1.8       1.7       1.9       5.8       3.3       3.2       3.3       1.0       2.6       1.0       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.0, Producer CPU:  7, Consumer CPU:  4
Cost: 1.0, Producer CPU: 11, Consumer CPU: 10
Cost: 1.0, Producer CPU:  7, Consumer CPU:  6
Cost: 1.0, Producer CPU: 11, Consumer CPU:  8
Cost: 1.0, Producer CPU: 10, Consumer CPU: 11

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 5.9, Producer CPU:  7, Consumer CPU: 10
Cost: 5.8, Producer CPU:  7, Consumer CPU:  9
Cost: 5.8, Producer CPU:  5, Consumer CPU: 10
Cost: 5.8, Producer CPU:  6, Consumer CPU: 10
Cost: 5.8, Producer CPU:  4, Consumer CPU:  8
============================
Rigtorp_SPSC_Pause
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       2.3       2.7       2.5      15.8      15.9      15.7      16.0      15.3      15.6      15.3      15.5
    CPU  1       2.4       N/A       2.5       2.6      15.8      15.9      15.9      15.9      15.4      15.4      15.4      15.5
    CPU  2       1.6       1.6       N/A       1.6       7.5       7.4       7.4       7.5       7.2       7.2       7.2       7.2
    CPU  3       1.7       1.7       2.1       N/A      10.3      10.3      10.4      10.4       9.9       9.8       9.8       9.8
    CPU  4       5.5       5.4       5.3       5.3       N/A       5.3       5.4       5.4       4.2       4.2       4.4       4.8
    CPU  5       5.6       5.4       5.4       5.3       5.4       N/A       5.3       5.3       4.4       4.2       4.2       4.5
    CPU  6       6.4       6.6       5.3       5.3       5.4       5.3       N/A       8.4       5.0       5.0       5.0       4.2
    CPU  7       5.5       6.4       5.4       5.3       6.8       5.3       7.6       N/A       4.2       4.5       4.3       4.5
    CPU  8       5.4       5.4       5.4       5.3       4.1       4.1       4.1       4.1       N/A       5.4       5.4       5.4
    CPU  9       5.4       5.4       5.4       5.2       4.1       4.1       4.1       4.1       5.3       N/A       5.4       5.4
    CPU 10       5.4       5.4       5.3       5.3       4.1       4.1       4.1       4.1       5.3       5.4       N/A       5.3
    CPU 11       5.4       5.4       5.4       5.2       4.1       4.2       4.1       4.1       5.4       5.3       5.4       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.6, Producer CPU:  1, Consumer CPU:  2
Cost: 1.6, Producer CPU:  3, Consumer CPU:  2
Cost: 1.6, Producer CPU:  0, Consumer CPU:  2
Cost: 1.7, Producer CPU:  0, Consumer CPU:  3
Cost: 1.7, Producer CPU:  1, Consumer CPU:  3

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 16.0, Producer CPU:  7, Consumer CPU:  0
Cost: 15.9, Producer CPU:  5, Consumer CPU:  1
Cost: 15.9, Producer CPU:  5, Consumer CPU:  0
Cost: 15.9, Producer CPU:  7, Consumer CPU:  1
Cost: 15.9, Producer CPU:  6, Consumer CPU:  1
============================
fastchan_SPSC_NoOp
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A      15.8      12.8      11.7      15.3      18.0      18.0      17.0      16.3      15.5      16.3      15.4
    CPU  1       6.4       N/A      10.5      10.5       7.9       7.9       7.9       7.3       7.2       7.2       7.7       7.2
    CPU  2       7.7      14.0       N/A      13.7      13.0      13.9      13.9      13.9      12.5      13.3      12.4      13.2
    CPU  3       7.2      16.9      12.9       N/A      15.0      13.9      15.0      15.0      14.6      14.1      14.5      14.5
    CPU  4      12.4      12.4       7.6      11.4       N/A       2.7       2.7       2.8      16.8       2.2      16.9      17.0
    CPU  5      12.7      12.1       7.6      11.6       1.9       N/A       2.7       1.9      16.9       2.0      16.9      16.8
    CPU  6      11.8      11.6       9.2      12.0       2.8       1.9       N/A       2.8      17.0      16.9       2.1      16.8
    CPU  7      12.2      11.7       9.2      12.2       2.7       2.1       2.7       N/A       3.2      16.9       4.7       2.1
    CPU  8      12.2      11.0       8.1      11.2      19.1      19.2      19.2      10.8       N/A       2.8       1.9       2.8
    CPU  9      13.5      10.8       7.7      11.4      18.3       9.8      19.2       4.3       2.7       N/A       2.0       2.8
    CPU 10      11.4      10.9       8.1      11.7      19.2      19.2      19.2      19.1       2.7       2.7       N/A       2.8
    CPU 11      12.3      11.4       8.1      11.5       1.9       2.1       8.7       3.3       1.9       1.9       2.7       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.9, Producer CPU:  9, Consumer CPU: 11
Cost: 1.9, Producer CPU: 10, Consumer CPU:  8
Cost: 1.9, Producer CPU:  7, Consumer CPU:  5
Cost: 1.9, Producer CPU:  4, Consumer CPU:  5
Cost: 1.9, Producer CPU:  5, Consumer CPU:  6

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 19.2, Producer CPU:  4, Consumer CPU: 10
Cost: 19.2, Producer CPU:  6, Consumer CPU:  8
Cost: 19.2, Producer CPU:  6, Consumer CPU:  9
Cost: 19.2, Producer CPU:  6, Consumer CPU: 10
Cost: 19.2, Producer CPU:  5, Consumer CPU:  8
============================
rigtorp_SPSC_NoOp
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       2.0       1.8       1.7      14.6      14.9      14.6      14.4      14.1      13.6      13.4      14.1
    CPU  1       2.0       N/A       3.1       3.0      14.1      14.1      14.0      14.2      13.6      13.5      13.2      13.5
    CPU  2       1.7       3.3       N/A       1.9       7.3       7.2       7.3       7.1       6.8       6.9       6.9       6.9
    CPU  3       1.6       3.7       2.1       N/A       9.3       9.3       9.4       9.4       9.4       9.4       9.4       9.5
    CPU  4       7.0       6.7       7.5       7.1       N/A       8.3       6.3      11.0       7.0       7.0       7.0       7.0
    CPU  5       7.0       6.7       7.5       7.1       9.9       N/A       8.8      10.0       6.8       7.0       6.7       7.0
    CPU  6       7.0       6.6       7.5       8.5       8.8      10.0       N/A      10.0       6.4       7.0       7.0       7.0
    CPU  7       7.0       6.6       7.5       7.0      11.1       7.0       6.3       N/A       5.8       6.0       6.6       7.0
    CPU  8       6.6       6.4       7.2       7.2       7.8       6.2       6.2       7.8       N/A       9.0       9.0       9.8
    CPU  9       6.5       6.4       7.2       7.1       7.8       7.8       7.8       7.8      10.8       N/A      10.8      10.8
    CPU 10       6.5       6.4       7.2       7.2       7.9       7.8       7.8       7.8      10.8      10.8       N/A      10.8
    CPU 11       6.5       6.4       7.2       7.2       7.9       7.8       7.8       7.8      10.8      10.8       9.8       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.6, Producer CPU:  0, Consumer CPU:  3
Cost: 1.7, Producer CPU:  3, Consumer CPU:  0
Cost: 1.7, Producer CPU:  0, Consumer CPU:  2
Cost: 1.8, Producer CPU:  2, Consumer CPU:  0
Cost: 1.9, Producer CPU:  3, Consumer CPU:  2

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 14.9, Producer CPU:  5, Consumer CPU:  0
Cost: 14.6, Producer CPU:  4, Consumer CPU:  0
Cost: 14.6, Producer CPU:  6, Consumer CPU:  0
Cost: 14.4, Producer CPU:  7, Consumer CPU:  0
Cost: 14.2, Producer CPU:  7, Consumer CPU:  1
============================
fastchan_SPSC_Yield
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A       1.3       1.3       1.3       1.3       3.3       3.3       2.8       1.3       1.3       3.3       3.3
    CPU  1       1.2       N/A       1.2       1.3       1.2       3.0       1.9       1.2       1.2       3.0       3.0       3.0
    CPU  2       1.2       1.2       N/A       1.2       3.1       1.2       1.2       3.1       1.2       3.1       1.3       1.4
    CPU  3       1.2       1.3       1.2       N/A       3.0       2.0       1.2       3.1       3.1       1.2       1.2       3.1
    CPU  0       N/A       2.1       2.2       2.3       9.2       9.1       8.9       8.5       8.6       8.9       8.7       8.8
    CPU  1       1.9       N/A       2.1       2.2       9.1       9.1       9.1       9.1       8.9       8.5       8.7       8.7
    CPU  2       1.3       1.4       N/A       1.3       3.8       3.8       3.8       3.8       3.8       3.8       3.8       3.7
    CPU  3       1.3       1.4       1.7       N/A       4.0       4.4       4.2       4.2       4.3       4.4       4.5       4.0
    CPU  4       7.6       7.1       6.5       6.8       N/A       6.9       7.0       7.0       5.4       5.4       5.4       5.4
    CPU  5       7.6       7.3       6.5       6.8       6.9       N/A       7.0       7.0       5.4       5.4       5.4       5.4
    CPU  6       7.6       7.3       6.5       6.8       6.7       7.0       N/A       6.9       5.4       5.4       5.4       7.0
    CPU  7       7.6       7.3       6.5       6.8       7.0       7.0       7.0       N/A       5.4       5.4       5.4       5.4
    CPU  8       7.2       7.2       6.3       6.5       5.6       5.7       5.6       5.6       N/A       7.2       7.2       7.2
    CPU  9       7.2       7.2       6.3       6.6       5.6       5.6       5.7       5.7       7.1       N/A       7.1       7.1
    CPU 10       7.2       7.2       6.2       6.6       5.6       5.7       5.6       5.6       7.1       7.2       N/A       7.1
    CPU 11       7.2       7.2       6.3       6.6       5.7       5.6       5.6       5.7       7.2       7.2       7.1       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 1.3, Producer CPU:  0, Consumer CPU:  2
Cost: 1.3, Producer CPU:  0, Consumer CPU:  3
Cost: 1.3, Producer CPU:  3, Consumer CPU:  2
Cost: 1.4, Producer CPU:  1, Consumer CPU:  2
Cost: 1.4, Producer CPU:  1, Consumer CPU:  3

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 9.2, Producer CPU:  4, Consumer CPU:  0
Cost: 9.1, Producer CPU:  5, Consumer CPU:  0
Cost: 9.1, Producer CPU:  5, Consumer CPU:  1
Cost: 9.1, Producer CPU:  4, Consumer CPU:  1
Cost: 9.1, Producer CPU:  7, Consumer CPU:  1
============================
SPSC_CV
    C\P ms    CPU  0    CPU  1    CPU  2    CPU  3    CPU  4    CPU  5    CPU  6    CPU  7    CPU  8    CPU  9    CPU 10    CPU 11
    CPU  0       N/A      15.4      14.7      17.7      28.7      28.2      28.3      28.8      26.4      26.2      26.0      26.0
    CPU  1       7.4       N/A       6.8       7.3      24.1      23.9      24.2      24.1      28.5      28.5      28.3      28.3
    CPU  2       9.4       7.3       N/A      12.8      30.5      29.9      29.8      30.0      29.2      30.0      30.3      29.6
    CPU  3      10.3       9.2      13.4       N/A      29.0      29.3      29.5      29.2      25.2      25.4      25.1      25.2
    CPU  4       6.9       6.7       6.9       7.2       N/A       8.1       8.0       5.7       7.8      11.4      11.3       6.3
    CPU  5       7.1       7.0       6.9       7.3       8.0       N/A       8.0       8.0       5.9       5.9      11.5      11.5
    CPU  6       9.8       9.0       8.7       9.3       5.7       8.0       N/A       8.0      11.3      11.5      11.5      11.4
    CPU  7       7.4       6.8       7.1       9.2       5.7       5.7       8.0       N/A      11.5      11.5      11.5      11.3
    CPU  8       7.4       6.6       6.7       7.0      12.0      11.9      11.9      12.0       N/A       8.0       8.0       7.9
    CPU  9       7.5       6.9       6.8       7.0      11.8      11.9      11.8      11.9       8.0       N/A       8.1       7.9
    CPU 10       7.5       6.9       6.7       7.0      12.0      12.0      11.9      12.1       8.0       8.0       N/A       8.0
    CPU 11       7.5       6.9       8.3       8.7       8.1       6.1       9.8      11.9       8.0       8.0       8.0       N/A

Best 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 5.7, Producer CPU:  4, Consumer CPU:  6
Cost: 5.7, Producer CPU:  4, Consumer CPU:  7
Cost: 5.7, Producer CPU:  5, Consumer CPU:  7
Cost: 5.7, Producer CPU:  7, Consumer CPU:  4
Cost: 5.9, Producer CPU:  9, Consumer CPU:  5

Worst 5 Cost per Op (ns/iteration) and their CPU pairs:
Cost: 30.5, Producer CPU:  4, Consumer CPU:  2
Cost: 30.3, Producer CPU: 10, Consumer CPU:  2
Cost: 30.0, Producer CPU:  9, Consumer CPU:  2
Cost: 30.0, Producer CPU:  7, Consumer CPU:  2
Cost: 29.9, Producer CPU:  5, Consumer CPU:  2
```

