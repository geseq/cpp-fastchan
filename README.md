# cpp-fastchan

High performance SPSC and MPSC ringbuffers which can be blocking/non-blocking on gets, puts, or both. For blocking queue it's also possibe to spin wait instead of yield thread execution.

If the size provided is not a power if 2, it's rounded up to the next power of 2.

# Usage

```cpp
// SPSC
fastchan::SPSC<int, blockingType, chan_size> c;
// OR
fastchan::SPSC<int, blockingType, chan_size, fastchan::WaitSpin> c;

c,put(0);
c,put(1);
c,put(2);
c,put(3);

auto val = c.get();
```

```cpp
// MPSC
fastchan::MPSC<int, blockingType, chan_size> c;
// OR
fastchan::MPSC<int, blockingType, chan_size, fastchan::WaitSpin> c;

c,put(0);
c,put(1);
c,put(2);
c,put(3);

auto val = c.get();

```
