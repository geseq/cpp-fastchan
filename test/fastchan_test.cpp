#include <cassert>
#include <fastchan.hpp>
#include <thread>

using namespace std::chrono_literals;

template <fastchan::BlockingType blockingType, int iterations>
void testFastchanSingleThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::FastChan<int, chan_size, blockingType> chan;

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

    assert(chan.isEmpty());
    assert(chan.size() == 0);
    chan.empty();
    assert(chan.size() == 0);
    assert(chan.isEmpty());
}

template <fastchan::BlockingType blockingType, int iterations>
void testFastchanMultiThreaded() {
    constexpr std::size_t chan_size = (iterations / 2) + 1;
    fastchan::FastChan<int, chan_size, blockingType> chan;

    // Test put and get with multiple threads
    std::thread producer([&] {
        for (int i = 0; i < iterations * 2; ++i) {
            chan.put(i);
        }
    });

    std::thread consumer([&] {
        for (int i = 0; i < iterations * 2; ++i) {
            assert(chan.get() == i);
        }
    });

    producer.join();
    consumer.join();

    assert(chan.size() == 0);
}

template <fastchan::BlockingType blockingType>
void testFastChan() {
    testFastchanSingleThreaded<blockingType, 4096>();
    testFastchanMultiThreaded<blockingType, 4096>();
}

int main() {
    testFastChan<fastchan::Blocking>();

    return 0;
}
