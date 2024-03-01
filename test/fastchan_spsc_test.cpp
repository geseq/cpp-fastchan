#include <cassert>
#include <iostream>
#include <optional>
#include <spsc.hpp>
#include <thread>

using namespace std::chrono_literals;

const auto IterationsMultiplier = 100;

template <int iterations, class put_wait_strategy, class get_wait_strategy>
void testSPSCSingleThreaded_Fill() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    for (int i = 0; i < iterations; ++i) {
        if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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

template <int iterations, class put_wait_strategy, class get_wait_strategy>
void testSPSCSingleThreaded_PutGet() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    assert(chan.isFull() == false);

    // Test put and get with a single thread
    for (int i = 0; i < iterations; ++i) {
        if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
            assert(chan.put(i));
        } else {
            chan.put(i);
        }
    }

    for (int i = 0; i < iterations; ++i) {
        if constexpr (std::is_same<get_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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

template <int iterations, class put_wait_strategy, class get_wait_strategy>
void testSPSCMultiThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::SPSC<int, chan_size, put_wait_strategy, get_wait_strategy> chan;

    auto total_iterations = IterationsMultiplier * iterations;

    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 1; i <= total_iterations; ++i) {
            if constexpr (std::is_same<put_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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
            if constexpr (std::is_same<get_wait_strategy, fastchan::ReturnImmediateStrategy>::value) {
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

template <class put_wait_type, class get_wait_type>
void testSPSC() {
    testSPSCSingleThreaded_Fill<4096, put_wait_type, get_wait_type>();
    testSPSCSingleThreaded_PutGet<4096, put_wait_type, get_wait_type>();
    testSPSCMultiThreaded<4096, put_wait_type, get_wait_type>();
}

int main() {
    testSPSC<fastchan::PauseWaitStrategy, fastchan::PauseWaitStrategy>();
    testSPSC<fastchan::PauseWaitStrategy, fastchan::ReturnImmediateStrategy>();
    testSPSC<fastchan::ReturnImmediateStrategy, fastchan::PauseWaitStrategy>();
    testSPSC<fastchan::ReturnImmediateStrategy, fastchan::ReturnImmediateStrategy>();

    testSPSC<fastchan::YieldWaitStrategy, fastchan::YieldWaitStrategy>();
    testSPSC<fastchan::YieldWaitStrategy, fastchan::ReturnImmediateStrategy>();
    testSPSC<fastchan::ReturnImmediateStrategy, fastchan::YieldWaitStrategy>();
    testSPSC<fastchan::ReturnImmediateStrategy, fastchan::ReturnImmediateStrategy>();

    testSPSC<fastchan::CVWaitStrategy, fastchan::CVWaitStrategy>();
    testSPSC<fastchan::CVWaitStrategy, fastchan::ReturnImmediateStrategy>();
    testSPSC<fastchan::ReturnImmediateStrategy, fastchan::CVWaitStrategy>();
    testSPSC<fastchan::ReturnImmediateStrategy, fastchan::ReturnImmediateStrategy>();

    return 0;
}
