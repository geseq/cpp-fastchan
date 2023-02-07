# cpp-fastchan

SPSC ringbuffer that started out based on [fastchan](https://github.com/geseq/fastchan)


If the size provided is not a power if 2, it's rounded up to the next power of 2.

Can be blocking/non-blocking on gets, puts, or both.

