#include <limits>

#ifndef FASTCHANCOMMON_HPP
#define FASTCHANCOMMON_HPP

namespace fastchan {

#ifndef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__
#endif

enum BlockingType {
    BlockingPutBlockingGet,
    BlockingPutNonBlockingGet,
    NonBlockingPutBlockingGet,
    NonBlockingPutNonBlockingGet,
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
