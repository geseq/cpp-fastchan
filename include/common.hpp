#include <limits>

#ifndef FASTCHANCOMMON_HPP
#define FASTCHANCOMMON_HPP

namespace fastchan {

#ifndef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__
#endif

inline void cpu_pause() {
#if defined(__x86_64__) || defined(__i386__)
    asm volatile("pause" ::: "memory");
#elif defined(__arm__) || defined(__aarch64__)
    asm volatile("yield" ::: "memory");
#else
    // do nothing
#endif
}

enum BlockingType {
    BlockingPutBlockingGet,
    BlockingPutNonBlockingGet,
    NonBlockingPutBlockingGet,
    NonBlockingPutNonBlockingGet,
};

enum WaitType {
    WaitPause,
    WaitYield,
    WaitCondition,
    WaitNoOp,
};

constexpr size_t roundUpNextPowerOfTwo(size_t v) {
    v--;
    for (size_t i = 1; i < sizeof(v) * CHAR_BIT; i *= 2) {
        v |= v >> i;
    }
    return ++v;
}
}  // namespace fastchan

#endif
