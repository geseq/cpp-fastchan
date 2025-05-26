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

template <int iterations, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testMPSCSingleThreaded_Fill() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test filling up with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (put_mode == fastchan::ReturnMode::NonBlocking) {
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

template <int iterations, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testMPSCSingleThreaded_PutGet() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    assert(chan.isFull() == false);

    // Test put and get with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (put_mode == fastchan::ReturnMode::NonBlocking) {
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
        if constexpr (get_mode == fastchan::ReturnMode::NonBlocking) {
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

template <int iterations, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testMPSCMultiThreadedSingleProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

    auto total_iterations = IterationsMultiplier * iterations;
    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= total_iterations; ++i) {
            if constexpr (put_mode == fastchan::ReturnMode::NonBlocking) {
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
            if constexpr (get_mode == fastchan::ReturnMode::NonBlocking) {
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

template <int iterations, int num_threads, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testMPSCMultiThreadedMultiProducer() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::MPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

    size_t total_iterations = IterationsMultiplier * iterations;
    size_t total = num_threads * (total_iterations * (total_iterations + 1) / 2);

    std::array<std::thread, num_threads> producers;

    for (auto i = 0; i < num_threads; i++) {
        // Test put and get with multiple threads
        producers[i] = std::thread([&] {
            for (int i = 1; i <= total_iterations; ++i) {
                if constexpr (put_mode == fastchan::ReturnMode::NonBlocking) {
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
            if constexpr (get_mode == fastchan::ReturnMode::NonBlocking) {
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

template <fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_type = fastchan::YieldWaitStrategy,
          class get_wait_type = fastchan::YieldWaitStrategy>
void testMPSC() {
    testMPSCSingleThreaded_Fill<4, put_mode, get_mode, put_wait_type, get_wait_type>();
    testMPSCSingleThreaded_PutGet<4, put_mode, get_mode, put_wait_type, get_wait_type>();
    testMPSCMultiThreadedSingleProducer<4, put_mode, get_mode, put_wait_type, get_wait_type>();
    if (std::thread::hardware_concurrency() > 5) {
        testMPSCMultiThreadedMultiProducer<4, 3, put_mode, get_mode, put_wait_type, get_wait_type>();
        testMPSCMultiThreadedMultiProducer<4, 5, put_mode, get_mode, put_wait_type, get_wait_type>();
    } else {
        testMPSCMultiThreadedMultiProducer<4, 2, put_mode, get_mode, put_wait_type, get_wait_type>();
    }

    testMPSCSingleThreaded_Fill<4096, put_mode, get_mode, put_wait_type, get_wait_type>();
    testMPSCSingleThreaded_PutGet<4096, put_mode, get_mode, put_wait_type, get_wait_type>();
    testMPSCMultiThreadedSingleProducer<4096, put_mode, get_mode, put_wait_type, get_wait_type>();
    if (std::thread::hardware_concurrency() > 5) {
        testMPSCMultiThreadedMultiProducer<4096, 3, put_mode, get_mode, put_wait_type, get_wait_type>();
        testMPSCMultiThreadedMultiProducer<4096, 5, put_mode, get_mode, put_wait_type, get_wait_type>();
    } else {
        testMPSCMultiThreadedMultiProducer<4096, 2, put_mode, get_mode, put_wait_type, get_wait_type>();
    }
}

int main() {
    using namespace fastchan;

    testMPSC<ReturnMode::Blocking, ReturnMode::Blocking, PauseWaitStrategy, PauseWaitStrategy>();
    testMPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, PauseWaitStrategy>();
    testMPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, PauseWaitStrategy>();
    testMPSC<ReturnMode::NonBlocking, ReturnMode::NonBlocking>();

    testMPSC<ReturnMode::Blocking, ReturnMode::Blocking, YieldWaitStrategy, YieldWaitStrategy>();
    testMPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, YieldWaitStrategy>();
    testMPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, YieldWaitStrategy>();
    testMPSC<ReturnMode::NonBlocking, ReturnMode::NonBlocking>();

    testMPSC<ReturnMode::Blocking, ReturnMode::Blocking, CVWaitStrategy, CVWaitStrategy>();
    testMPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, CVWaitStrategy>();
    testMPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, CVWaitStrategy>();
    testMPSC<ReturnMode::NonBlocking, ReturnMode::NonBlocking>();

    return 0;
}
