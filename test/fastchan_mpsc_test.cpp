#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <mpsc.hpp>
#include <thread>

using namespace std::chrono_literals;

const auto IterationsMultiplier = 100;

template <int iterations, class put_wait_strategy, class get_wait_strategy>
void testMPSCSingleThreaded_Fill() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test filling up with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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
}

template <int iterations, class put_wait_strategy, class get_wait_strategy>
void testMPSCSingleThreaded_PutGet() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    assert(chan.isFull() == false);

    // Test put and get with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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
        if constexpr (std::is_same<get_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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
}

template <int iterations, class put_wait_strategy, class get_wait_strategy>
void testMPSCMultiThreadedSingleProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    auto total_iterations = IterationsMultiplier * iterations;
    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= total_iterations; ++i) {
            if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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
            if constexpr (std::is_same<get_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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

template <int iterations, int num_threads, class put_wait_strategy, class get_wait_strategy>
void testMPSCMultiThreadedMultiProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    size_t total_iterations = IterationsMultiplier * iterations;
    size_t total = num_threads * (total_iterations * (total_iterations + 1) / 2);

    std::array<std::thread, num_threads> producers;

    for (auto i = 0; i < num_threads; i++) {
        // Test put and get with multiple threads
        producers[i] = std::thread([&] {
            for (int i = 1; i <= total_iterations; ++i) {
                if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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
            if constexpr (std::is_same<get_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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

template <class put_wait_type, class get_wait_type>
void testMPSC() {
    testMPSCSingleThreaded_Fill<4, put_wait_type, get_wait_type>();
    testMPSCSingleThreaded_PutGet<4, put_wait_type, get_wait_type>();
    testMPSCMultiThreadedSingleProducer<4, put_wait_type, get_wait_type>();
    if (std::thread::hardware_concurrency() > 5) {
        testMPSCMultiThreadedMultiProducer<4, 3, put_wait_type, get_wait_type>();
        testMPSCMultiThreadedMultiProducer<4, 5, put_wait_type, get_wait_type>();
    } else {
        testMPSCMultiThreadedMultiProducer<4, 2, put_wait_type, get_wait_type>();
    }

    testMPSCSingleThreaded_Fill<4096, put_wait_type, get_wait_type>();
    testMPSCSingleThreaded_PutGet<4096, put_wait_type, get_wait_type>();
    testMPSCMultiThreadedSingleProducer<4096, put_wait_type, get_wait_type>();
    if (std::thread::hardware_concurrency() > 5) {
        testMPSCMultiThreadedMultiProducer<4096, 3, put_wait_type, get_wait_type>();
        testMPSCMultiThreadedMultiProducer<4096, 5, put_wait_type, get_wait_type>();
    } else {
        testMPSCMultiThreadedMultiProducer<4096, 2, put_wait_type, get_wait_type>();
    }
}

int main() {
    testMPSC<fastchan::PauseWaitStrategy, fastchan::PauseWaitStrategy>();
    testMPSC<fastchan::PauseWaitStrategy, fastchan::ReturnImmediateStrategy>();
    testMPSC<fastchan::ReturnImmediateStrategy, fastchan::PauseWaitStrategy>();
    testMPSC<fastchan::ReturnImmediateStrategy, fastchan::ReturnImmediateStrategy>();

    testMPSC<fastchan::YieldWaitStrategy, fastchan::YieldWaitStrategy>();
    testMPSC<fastchan::YieldWaitStrategy, fastchan::ReturnImmediateStrategy>();
    testMPSC<fastchan::ReturnImmediateStrategy, fastchan::YieldWaitStrategy>();
    testMPSC<fastchan::ReturnImmediateStrategy, fastchan::ReturnImmediateStrategy>();

    testMPSC<fastchan::CVWaitStrategy, fastchan::CVWaitStrategy>();
    testMPSC<fastchan::CVWaitStrategy, fastchan::ReturnImmediateStrategy>();
    testMPSC<fastchan::ReturnImmediateStrategy, fastchan::CVWaitStrategy>();
    testMPSC<fastchan::ReturnImmediateStrategy, fastchan::ReturnImmediateStrategy>();

    return 0;
}
