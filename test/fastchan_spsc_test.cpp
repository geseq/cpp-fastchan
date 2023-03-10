#include <cassert>
#include <iostream>
#include <spsc.hpp>
#include <thread>

using namespace std::chrono_literals;

const auto IterationsMultiplier = 100;

template <fastchan::BlockingType blockingType, int iterations>
void testSPSCSingleThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, blockingType, chan_size> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test put and read with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::NonBlockingPutBlockingGet) {
            assert(chan.put(i));
        } else {
            chan.put(i);
        }

        assert(chan.size() == i + 1);
        assert(chan.isEmpty() == false);
        if (i < iterations - 1) {
            assert(chan.isFull() == false);
        } else {
            assert(chan.isFull() == true);
        }
    }
    assert(chan.size() == iterations);
    assert(chan.isFull() == true);
    assert(chan.isEmpty() == false);
    chan.empty();
    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    assert(chan.isFull() == false);

    // Test put and get with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::NonBlockingPutBlockingGet) {
            assert(chan.put(i));
        } else {
            chan.put(i);
        }
    }

    for (int i = 0; i < iterations; ++i) {
        if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::BlockingPutNonBlockingGet) {
            assert(chan.get() == i);
        } else {
            assert(chan.get() == i);
        }
    }

    assert(chan.isEmpty());
    assert(chan.size() == 0);
    chan.empty();
    assert(chan.size() == 0);
    assert(chan.isEmpty());
}

template <fastchan::BlockingType blockingType, int iterations>
void testSPSCMultiThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, blockingType, chan_size> chan;

    auto total_iterations = IterationsMultiplier * iterations;

    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= total_iterations; ++i) {
            if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::NonBlockingPutBlockingGet) {
                auto result = false;
                do {
                    result = chan.put(i);
                } while (!result);
                continue;
            }

            chan.put(i);
        }
    });

    std::thread consumer([&] {
        for (int i = 1; i <= total_iterations;) {
            if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::BlockingPutNonBlockingGet) {
                auto&& val = chan.get();
                while (!val) {
                    val = chan.get();
                }

                assert(*val == i);
                ++i;
                continue;
            }

            auto val = chan.get();
            assert(val == i);
            ++i;
        }
    });

    producer.join();
    consumer.join();

    assert(chan.size() == 0);
}

template <fastchan::BlockingType blockingType>
void testSPSC() {
    testSPSCSingleThreaded<blockingType, 4096>();
    testSPSCMultiThreaded<blockingType, 4096>();
}

int main() {
    testSPSC<fastchan::BlockingPutBlockingGet>();
    testSPSC<fastchan::BlockingPutNonBlockingGet>();
    testSPSC<fastchan::NonBlockingPutBlockingGet>();
    testSPSC<fastchan::NonBlockingPutNonBlockingGet>();

    return 0;
}
