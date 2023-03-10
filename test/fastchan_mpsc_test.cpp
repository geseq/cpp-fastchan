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

const auto IterationsMultiplier = 100;

template <fastchan::BlockingType blockingType, int iterations>
void testMPSCSingleThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, blockingType, chan_size> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test filling up with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::NonBlockingPutBlockingGet) {
            auto result = false;
            do {
                result = chan.put(i);
            } while (!result);
            assert(result);
        } else {
            chan.put(i);
        }

        assert(chan.size() == i + 1);
        assert(chan.isEmpty() == false);
        if ((i + 1) % iterations != 0) {
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
            auto result = false;
            do {
                result = chan.put(i);
            } while (!result);
            assert(result);
            continue;
        }

        chan.put(i);
    }

    for (int i = 0; i < iterations; ++i) {
        if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::BlockingPutNonBlockingGet) {
            auto&& val = chan.get();
            while (!val) {
                val = chan.get();
            }

            assert(val == i);
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

template <fastchan::BlockingType blockingType, int iterations>
void testMPSCMultiThreadedSingleProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, blockingType, chan_size> chan;

    auto total_iterations = IterationsMultiplier * iterations;
    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= total_iterations; ++i) {
            if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::NonBlockingPutBlockingGet) {
                auto result = false;
                do {
                    result = chan.put(i);
                } while (!result);
            } else {
                chan.put(i);
            }
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
            } else {
                auto val = chan.get();
                assert(val == i);
                ++i;
            }
        }
    });

    producer.join();
    consumer.join();

    assert(chan.size() == 0);
}

template <fastchan::BlockingType blockingType, int iterations, int num_threads>
void testMPSCMultiThreadedMultiProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, blockingType, chan_size> chan;

    size_t total_iterations = IterationsMultiplier * iterations;
    size_t total = num_threads * (total_iterations * (total_iterations + 1) / 2);

    std::array<std::thread, num_threads> producers;

    for (auto i = 0; i < num_threads; i++) {
        // Test put and get with multiple threads
        producers[i] = std::thread([&] {
            for (int i = 1; i <= total_iterations; ++i) {
                if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::NonBlockingPutBlockingGet) {
                    auto result = false;
                    do {
                        result = chan.put(i);
                    } while (!result);
                } else {
                    chan.put(i);
                }
            }
        });
    }

    std::thread consumer([&] {
        for (int i = 1; i <= total_iterations * num_threads;) {
            if constexpr (blockingType == fastchan::NonBlockingPutNonBlockingGet || blockingType == fastchan::BlockingPutNonBlockingGet) {
                auto&& val = chan.get();
                while (!val) {
                    val = chan.get();
                }

                total -= *val;
                ++i;
            } else {
                auto val = chan.get();
                total -= val;
                ++i;
            }
        }
    });

    for (auto i = 0; i < num_threads; i++) {
        producers[i].join();
    }
    consumer.join();

    assert(total == 0);
    assert(chan.size() == 0);
}

template <fastchan::BlockingType blockingType>
void testMPSC() {
    testMPSCSingleThreaded<blockingType, 4>();
    testMPSCMultiThreadedSingleProducer<blockingType, 4>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4, 3>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4, 5>();

    testMPSCSingleThreaded<blockingType, 4096>();
    testMPSCMultiThreadedSingleProducer<blockingType, 4096>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4096, 3>();
    testMPSCMultiThreadedMultiProducer<blockingType, 4096, 5>();
}

int main() {
    testMPSC<fastchan::BlockingPutBlockingGet>();
    testMPSC<fastchan::BlockingPutNonBlockingGet>();
    testMPSC<fastchan::NonBlockingPutBlockingGet>();
    testMPSC<fastchan::NonBlockingPutNonBlockingGet>();

    return 0;
}
