# cpp-fastchan

High performance SPSC and MPSC ringbuffers which can be blocking/non-blocking on gets, puts, or both. 

If the size provided is not a power if 2, it's rounded up to the next power of 2.


# Usage

```cpp
// SPSC
fastchan::SPSC<int, blockingType, chan_size> c;

c,put(0);
c,put(1);
c,put(2);
c,put(3);

auto var = c.get();
```

```cpp
// MPSC
fastchan::MPSC<int, blockingType, chan_size> c;

c,put(0);
c,put(1);
c,put(2);
c,put(3);

auto var = c.get();

```
