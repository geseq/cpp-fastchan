#include <cassert>
#include <fastchan.hpp>
#include <iostream>
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
        assert(chan.put(i));

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
        assert(chan.put(i));
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
        for (int i = 1; i <= iterations * 2; i++) {
            auto result = chan.put(i);
            if (blockingType == fastchan::NonBlocking || blockingType == fastchan::NonBlockingPut) {
                while (!result) {
                    result = chan.put(i);
                }
            }
        }
    });

    std::thread consumer([&] {
        for (int i = 1; i <= iterations * 2;) {
            auto&& val = chan.get();
            if (blockingType == fastchan::NonBlocking || blockingType == fastchan::NonBlockingGet) {
                if (val == 0) {
                    continue;
                }
            }

            assert(val == i);
            ++i;
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
    testFastChan<fastchan::NonBlockingGet>();
    testFastChan<fastchan::NonBlockingPut>();
    testFastChan<fastchan::NonBlocking>();

    return 0;
}
