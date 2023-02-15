# cpp-fastchan

Ringbuffer that started out based on [fastchan](https://github.com/geseq/fastchan)

For now this includes only an SPSC and MPSC ringbuffer.

If the size provided is not a power if 2, it's rounded up to the next power of 2.

Can be blocking/non-blocking on gets, puts, or both. However, you probably don't want to use blocking puts for MPSC. The higher the number of threads the worse the performance unless your consumers are much faster than your producers.
