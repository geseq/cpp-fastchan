#include <cassert>
#include <iostream>
#include <optional>
#include <spsc.hpp>
#include <thread>

using namespace std::chrono_literals;

const auto IterationsMultiplier = 100;

template <int iterations, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testSPSCSingleThreaded_Fill() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    for (int i = 0; i < iterations; ++i) {
        if constexpr (put_mode == fastchan::ReturnMode::NonBlocking) {
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
}

template <int iterations, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testSPSCSingleThreaded_PutGet() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

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
        } else {
            chan.put(i);
        }
    }

    for (int i = 0; i < iterations; ++i) {
        if constexpr (get_mode == fastchan::ReturnMode::NonBlocking) {
            auto val = chan.get();
            while (val == std::nullopt) val = chan.get();
            assert(val == i);
        } else {
            assert(chan.get() == i);
        }
    }

    assert(chan.isEmpty());
    assert(chan.size() == 0);
}

template <int iterations, fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_strategy = fastchan::YieldWaitStrategy,
          class get_wait_strategy = fastchan::YieldWaitStrategy>
void testSPSCMultiThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size, put_wait_strategy, get_wait_strategy, put_mode, get_mode> chan;

    auto total_iterations = IterationsMultiplier * iterations;

    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= total_iterations; ++i) {
            if constexpr (put_mode == fastchan::ReturnMode::NonBlocking) {
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
            if constexpr (get_mode == fastchan::ReturnMode::NonBlocking) {
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

template <fastchan::ReturnMode put_mode, fastchan::ReturnMode get_mode, class put_wait_type = fastchan::YieldWaitStrategy,
          class get_wait_type = fastchan::YieldWaitStrategy>
void testSPSC() {
    testSPSCSingleThreaded_Fill<4096, put_mode, get_mode, put_wait_type, get_wait_type>();
    testSPSCSingleThreaded_PutGet<4096, put_mode, get_mode, put_wait_type, get_wait_type>();
    testSPSCMultiThreaded<4096, put_mode, get_mode, put_wait_type, get_wait_type>();
}

int main() {
    using namespace fastchan;

    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, PauseWaitStrategy, PauseWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, PauseWaitStrategy, YieldWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, PauseWaitStrategy, NoOpWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, PauseWaitStrategy, CVWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, PauseWaitStrategy>();

    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, YieldWaitStrategy, YieldWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, YieldWaitStrategy, PauseWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, YieldWaitStrategy, NoOpWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, YieldWaitStrategy, CVWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, YieldWaitStrategy>();

    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, NoOpWaitStrategy, NoOpWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, NoOpWaitStrategy, YieldWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, NoOpWaitStrategy, PauseWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, NoOpWaitStrategy, CVWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, NoOpWaitStrategy>();

    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, CVWaitStrategy, CVWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, CVWaitStrategy, YieldWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, CVWaitStrategy, PauseWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::Blocking, CVWaitStrategy, NoOpWaitStrategy>();
    testSPSC<ReturnMode::Blocking, ReturnMode::NonBlocking, CVWaitStrategy>();

    testSPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, PauseWaitStrategy>();
    testSPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, YieldWaitStrategy>();
    testSPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, NoOpWaitStrategy>();
    testSPSC<ReturnMode::NonBlocking, ReturnMode::Blocking, YieldWaitStrategy, CVWaitStrategy>();
    testSPSC<ReturnMode::NonBlocking, ReturnMode::NonBlocking>();

    return 0;
}
