#include <cassert>
#include <iostream>
#include <spsc.hpp>
#include <thread>

using namespace std::chrono_literals;

enum BlockingType { NonBlocking, NonBlockingPut, NonBlockingGet, Blocking };

template <BlockingType blockingType, int iterations>
void testSPSCSingleThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test put and read with a single thread
    for (int i = 0; i < iterations; ++i) {
        if (blockingType == NonBlocking || blockingType == NonBlockingPut) {
            assert(chan.putWithoutBlocking(i));
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
        if (blockingType == NonBlocking || blockingType == NonBlockingPut) {
            assert(chan.putWithoutBlocking(i));
        } else {
            chan.put(i);
        }
    }

    for (int i = 0; i < iterations; ++i) {
        if (blockingType == NonBlocking || blockingType == NonBlockingGet) {
            assert(chan.getWithoutBlocking() == i);
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

template <BlockingType blockingType, int iterations>
void testSPSCMultiThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size> chan;

    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= iterations * 2; ++i) {
            if (blockingType == NonBlocking || blockingType == NonBlockingPut) {
                auto result = false;
                do {
                    result = chan.putWithoutBlocking(i);
                } while (!result);
                continue;
            }

            chan.put(i);
        }
    });

    std::thread consumer([&] {
        for (int i = 1; i <= iterations * 2;) {
            if (blockingType == NonBlocking || blockingType == NonBlockingGet) {
                auto&& val = chan.getWithoutBlocking();
                while (!val) {
                    val = chan.getWithoutBlocking();
                }

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

template <BlockingType blockingType>
void testSPSC() {
    testSPSCSingleThreaded<blockingType, 4096>();
    testSPSCMultiThreaded<blockingType, 4096>();
}

int main() {
    testSPSC<Blocking>();
    testSPSC<NonBlockingGet>();
    testSPSC<NonBlockingPut>();
    testSPSC<NonBlocking>();

    return 0;
}
