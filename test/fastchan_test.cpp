#include <cassert>
#include <fastchan.hpp>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main() {
    constexpr int iterations = 8;  // power of 2
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::FastChan<int, chan_size> chan;

    assert(chan.size() == 0);
    assert(chan.isEmpty() == true);
    // Test put and read with a single thread
    for (int i = 0; i < iterations; ++i) {
        chan.put(i);

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
        chan.put(i);
    }

    for (int i = 0; i < iterations; ++i) {
        assert(chan.get() == i);
    }

    assert(chan.size() == 0);
    chan.empty();
    assert(chan.size() == 0);

    std::cout << "Test with multiple threads" << std::endl;
    // Test put and get with multiple threads
    std::thread producer([&chan] {
        for (int i = 0; i < iterations * 2; ++i) {
            std::cout << "PUT" << i << std::endl;
            chan.put(i);
        }
    });

    std::thread consumer([&chan]() {
        for (int i = 0; i < iterations * 2; ++i) {
            std::cout << "GET" << i << std::endl;
            assert(chan.get() == i);
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
        assert(chan.get() == i);
    }
    assert(chan.isEmpty());

    return 0;
}
