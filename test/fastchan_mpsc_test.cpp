#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <mpsc.hpp>
#include <thread>

using namespace std::chrono_literals;

enum BlockingType { NonBlocking, NonBlockingPut, NonBlockingGet, Blocking };

template <BlockingType blockingType, int iterations>
void testMPSCSingleThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test filling up with a single thread
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
            auto got = chan.get();
            assert(got == i);
        }
    }

    assert(chan.isEmpty());
    assert(chan.size() == 0);
    chan.empty();
    assert(chan.size() == 0);
    assert(chan.isEmpty());
}

template <BlockingType blockingType, int iterations>
void testMPSCMultiThreadedSingleProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size> chan;

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

template <BlockingType blockingType, int iterations, int num_threads>
void testMPSCMultiThreadedMultiProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size> chan;

    auto total_iterations = 2 * iterations;
    uint64_t total = num_threads * (total_iterations * (total_iterations + 1) / 2);

    std::array<std::thread, num_threads> producers;

    for (auto i = 0; i < num_threads; i++) {
        // Test put and get with multiple threads
        producers[i] = std::thread([&] {
            for (int i = 1; i <= total_iterations; ++i) {
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
    }

    std::thread consumer([&] {
        for (int i = 1; i <= total_iterations * num_threads;) {
            if (blockingType == NonBlocking || blockingType == NonBlockingGet) {
                auto&& val = chan.getWithoutBlocking();
                while (!val) {
                    val = chan.getWithoutBlocking();
                }

                total -= *val;
                ++i;
                continue;
            }

            auto val = chan.get();
            total -= val;
            ++i;
        }
    });

    for (auto i = 0; i < num_threads; i++) {
        producers[i].join();
    }
    consumer.join();

    assert(chan.size() == 0);
}

template <BlockingType blockingType>
void testMPSC() {
    testMPSCSingleThreaded<blockingType, 4096>();
    testMPSCMultiThreadedSingleProducer<blockingType, 4096>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4096, 3>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4096, 5>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4096, 15>();

    // TODO: add many threads test
}

int main() {
    testMPSC<Blocking>();
    testMPSC<NonBlockingGet>();
    testMPSC<NonBlockingPut>();
    testMPSC<NonBlocking>();

    return 0;
}
