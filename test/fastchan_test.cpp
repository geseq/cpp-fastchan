#include <cassert>
#include <fastchan.hpp>
#include <iostream>
#include <ostream>
#include <thread>

using namespace std::chrono_literals;

int main() {
    constexpr int iterations = 4096;  // power of 2
    constexpr std::size_t chan_size = iterations - 1;
    fastchan::FastChan<int, chan_size> chan;

    // Test put and read with a single thread
    for (int i = 0; i < iterations; ++i) {
        chan.put(i);
    }
    assert(chan.size() == iterations);
    chan.empty();
    assert(chan.size() == 0);

    // Test put and read with a single thread
    for (int i = 0; i < iterations; ++i) {
        chan.put(i);
    }

    for (int i = 0; i < iterations; ++i) {
        auto read = chan.read();
        assert(read == i);
    }

    assert(chan.size() == 0);
    chan.empty();
    assert(chan.size() == 0);

    // Test put and read with multiple threads
    std::thread producer([&] {
        for (int i = 0; i < iterations * 2; ++i) {
            chan.put(i);
        }
    });

    std::thread consumer([&] {
        for (int i = 0; i < iterations * 2; ++i) {
            auto read = chan.read();
            assert(read == i);
        }
    });

    producer.join();
    consumer.join();

    assert(chan.size() == 0);

    // Test filling and then emptying the chan
    for (int i = 0; i < iterations; ++i) {
        chan.put(i);
    }
    assert(chan.isFull());
    for (int i = 0; i < iterations; ++i) {
        assert(chan.read() == i);
    }
    assert(chan.isEmpty());

    return 0;
}
