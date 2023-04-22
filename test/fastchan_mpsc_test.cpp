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

template <fastchan::BlockingType blockingType, int iterations, fastchan::WaitType waitType>
void testMPSCSingleThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, blockingType, chan_size, waitType> chan;

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

template <fastchan::BlockingType blockingType, int iterations, fastchan::WaitType waitType>
void testMPSCMultiThreadedSingleProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, blockingType, chan_size, waitType> chan;

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

template <fastchan::BlockingType blockingType, int iterations, int num_threads, fastchan::WaitType waitType>
void testMPSCMultiThreadedMultiProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, blockingType, chan_size, waitType> chan;

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

template <fastchan::BlockingType blockingType, fastchan::WaitType waitType>
void testMPSC() {
    testMPSCSingleThreaded<blockingType, 4, waitType>();
    testMPSCMultiThreadedSingleProducer<blockingType, 4, waitType>();
    std::cout << "Concurrency: " << std::thread::hardware_concurrency() << std::endl;
    if (std::thread::hardware_concurrency() > 5) {
        testMPSCMultiThreadedMultiProducer<blockingType, 4, 3, waitType>();
        testMPSCMultiThreadedMultiProducer<blockingType, 4, 5, waitType>();
    } else {
        testMPSCMultiThreadedMultiProducer<blockingType, 4, 2, waitType>();
    }

    testMPSCSingleThreaded<blockingType, 4096, waitType>();
    testMPSCMultiThreadedSingleProducer<blockingType, 4096, waitType>();
    if (std::thread::hardware_concurrency() > 5) {
        testMPSCMultiThreadedMultiProducer<blockingType, 4096, 3, waitType>();
        testMPSCMultiThreadedMultiProducer<blockingType, 4096, 5, waitType>();
    } else {
        testMPSCMultiThreadedMultiProducer<blockingType, 4096, 2, waitType>();
    }
}

int main() {
    testMPSC<fastchan::BlockingPutBlockingGet, fastchan::WaitSpin>();
    testMPSC<fastchan::BlockingPutNonBlockingGet, fastchan::WaitSpin>();
    testMPSC<fastchan::NonBlockingPutBlockingGet, fastchan::WaitSpin>();
    testMPSC<fastchan::NonBlockingPutNonBlockingGet, fastchan::WaitSpin>();

    testMPSC<fastchan::BlockingPutBlockingGet, fastchan::WaitYield>();
    testMPSC<fastchan::BlockingPutNonBlockingGet, fastchan::WaitYield>();
    testMPSC<fastchan::NonBlockingPutBlockingGet, fastchan::WaitYield>();
    testMPSC<fastchan::NonBlockingPutNonBlockingGet, fastchan::WaitYield>();

    testMPSC<fastchan::BlockingPutBlockingGet, fastchan::WaitCondition>();
    testMPSC<fastchan::BlockingPutNonBlockingGet, fastchan::WaitCondition>();
    testMPSC<fastchan::NonBlockingPutBlockingGet, fastchan::WaitCondition>();
    testMPSC<fastchan::NonBlockingPutNonBlockingGet, fastchan::WaitCondition>();
    return 0;
}
