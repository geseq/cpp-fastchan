#include <benchmark/benchmark.h>

#include <atomic>
#include <chrono>
#include <mpsc.hpp>
#include <spsc.hpp>
#include <thread>

#include "boost/lockfree/policies.hpp"
#include "boost/lockfree/spsc_queue.hpp"

template <size_t min_size>
static void SPSC_BlockingBoth_Put(benchmark::State& state) {
    fastchan::SPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            auto&& it = c.get();
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.put(0);
    }
    shouldRun = false;

    // clear any blocks
    c.putWithoutBlocking(0);

    reader.join();
}

BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 16);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 64);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 256);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 1024);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 4096);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 16'384);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 65'536);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 262'144);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Put, 1'048'576);

template <size_t min_size>
static void SPSC_BlockingBoth_Get(benchmark::State& state) {
    fastchan::SPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            c.put(0);
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto&& it = c.get();
    }
    shouldRun.store(false);

    // clear any blocks
    c.getWithoutBlocking();
    reader.join();
}

BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 16);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 64);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 256);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 1024);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 4096);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 16'384);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 65'536);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 262'144);
BENCHMARK_TEMPLATE(SPSC_BlockingBoth_Get, 1'048'576);

template <size_t min_size>
static void SPSC_NonBlockingGet_Put(benchmark::State& state) {
    fastchan::SPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            auto&& it = c.getWithoutBlocking();
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.put(0);
    }
    shouldRun = false;

    // clear any blocks
    c.putWithoutBlocking(0);

    reader.join();
}

BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 16);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 64);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 256);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 1024);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 4096);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 16'384);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 65'536);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 262'144);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Put, 1'048'576);

template <size_t min_size>
static void SPSC_NonBlockingGet_Get(benchmark::State& state) {
    fastchan::SPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            c.put(0);
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto&& it = c.getWithoutBlocking();
    }
    shouldRun = false;

    // clear any blocks
    c.getWithoutBlocking();
    reader.join();
}

BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 16);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 64);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 256);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 1024);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 4096);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 16'384);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 65'536);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 262'144);
BENCHMARK_TEMPLATE(SPSC_NonBlockingGet_Get, 1'048'576);

template <size_t min_size>
static void SPSC_NonBlockingBoth_Put(benchmark::State& state) {
    fastchan::SPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            auto&& it = c.getWithoutBlocking();
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.putWithoutBlocking(0);
    }
    shouldRun = false;

    reader.join();
}

BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 16);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 64);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 256);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 1024);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 4096);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 16'384);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 65'536);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 262'144);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Put, 1'048'576);

template <size_t min_size>
static void SPSC_NonBlockingBoth_Get(benchmark::State& state) {
    fastchan::SPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            c.putWithoutBlocking(0);
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto&& it = c.getWithoutBlocking();
    }
    shouldRun = false;

    reader.join();
}

BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 16);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 64);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 256);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 1024);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 4096);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 16'384);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 65'536);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 262'144);
BENCHMARK_TEMPLATE(SPSC_NonBlockingBoth_Get, 1'048'576);

template <size_t min_size>
static void BoostSPSC_Put(benchmark::State& state) {
    boost::lockfree::spsc_queue<uint8_t, boost::lockfree::capacity<min_size>> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            auto&& it = c.pop();
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.push(0);
    }
    shouldRun = false;

    reader.join();
}

BENCHMARK_TEMPLATE(BoostSPSC_Put, 16);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 64);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 256);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 1024);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 4096);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 16'384);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 65'536);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 262'144);
BENCHMARK_TEMPLATE(BoostSPSC_Put, 1'048'576);

template <size_t min_size>
static void BoostSPSC_Get(benchmark::State& state) {
    boost::lockfree::spsc_queue<uint8_t, boost::lockfree::capacity<min_size>> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            c.push(0);
        }
    });

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        // NOTE: problem here is that this could be a no op throughput the bench
        auto&& it = c.pop();
    }
    shouldRun = false;
    reader.join();
}

BENCHMARK_TEMPLATE(BoostSPSC_Get, 16);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 64);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 256);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 1024);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 4096);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 16'384);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 65'536);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 262'144);
BENCHMARK_TEMPLATE(BoostSPSC_Get, 1'048'576);

template <size_t min_size, int num_producers>
static void MPSC_BlockingBoth_Put(benchmark::State& state) {
    fastchan::MPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            auto&& it = c.get();
        }
    });

    // create n-1 producers
    std::array<std::thread, num_producers - 1> producers;
    for (auto i = 0; i < num_producers - 1; ++i) {
        producers[i] = std::thread([&]() {
            while (shouldRun) {
                c.put(0);
            }
        });
    }

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.put(0);
    }

    shouldRun = false;

    // clear any blocks
    while (c.getWithoutBlocking()) {
    }
    while (c.putWithoutBlocking(0)) {
    }

    for (auto i = 0; i < num_producers - 1; ++i) {
        producers[i].join();
    }

    reader.join();
}

BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 16, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 64, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 256, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 1024, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 4096, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 16'384, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 65'536, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 262'144, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 1'048'576, 1);

BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 16, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 64, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 256, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 1024, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 4096, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 16'384, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 65'536, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 262'144, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 1'048'576, 2);

BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 16, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 64, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 256, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 1024, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 4096, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 16'384, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 65'536, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 262'144, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Put, 1'048'576, 5);

template <size_t min_size, int num_producers>
static void MPSC_BlockingBoth_Get(benchmark::State& state) {
    fastchan::MPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::array<std::thread, num_producers> producers;
    for (auto i = 0; i < num_producers; ++i) {
        producers[i] = std::thread([&]() {
            while (shouldRun) {
                c.put(0);
            }
        });
    }

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto&& it = c.get();
    }

    shouldRun = false;

    // clear any blocks
    while (c.getWithoutBlocking()) {
    }
    while (c.putWithoutBlocking(0)) {
    }

    for (auto i = 0; i < num_producers; ++i) {
        producers[i].join();
    }
}

BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 16, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 64, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 256, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 1024, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 4096, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 16'384, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 65'536, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 262'144, 1);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 1'048'576, 1);

BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 16, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 64, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 256, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 1024, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 4096, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 16'384, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 65'536, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 262'144, 2);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 1'048'576, 2);

BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 16, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 64, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 256, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 1024, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 4096, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 16'384, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 65'536, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 262'144, 5);
BENCHMARK_TEMPLATE(MPSC_BlockingBoth_Get, 1'048'576, 5);

template <size_t min_size, int num_producers>
static void MPSC_NonBlockingGet_Put(benchmark::State& state) {
    fastchan::MPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun.load(std::memory_order_relaxed)) {
            auto&& it = c.getWithoutBlocking();
        }
    });

    // create n-1 producers
    std::array<std::thread, num_producers - 1> producers;
    for (auto i = 0; i < num_producers - 1; ++i) {
        producers[i] = std::thread([&]() {
            while (shouldRun) {
                c.put(0);
            }
        });
    }

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.put(0);
    }
    shouldRun = false;

    // clear any blocks
    while (c.getWithoutBlocking()) {
    }
    while (c.putWithoutBlocking(0)) {
    }

    for (auto i = 0; i < num_producers - 1; ++i) {
        producers[i].join();
    }
    reader.join();
}

BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 16, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 64, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 256, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 1024, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 4096, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 16'384, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 65'536, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 262'144, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 1'048'576, 1);

BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 16, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 64, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 256, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 1024, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 4096, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 16'384, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 65'536, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 262'144, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 1'048'576, 2);

BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 16, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 64, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 256, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 1024, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 4096, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 16'384, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 65'536, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 262'144, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Put, 1'048'576, 5);

template <size_t min_size, int num_producers>
static void MPSC_NonBlockingGet_Get(benchmark::State& state) {
    fastchan::MPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    // create n producers
    std::array<std::thread, num_producers> producers;
    for (auto i = 0; i < num_producers; ++i) {
        producers[i] = std::thread([&]() {
            while (shouldRun) {
                c.put(0);
            }
        });
    }

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto&& it = c.getWithoutBlocking();
    }
    shouldRun = false;

    // clear any blocks
    while (c.getWithoutBlocking()) {
    }
    while (c.putWithoutBlocking(0)) {
    }

    for (auto i = 0; i < num_producers; ++i) {
        producers[i].join();
    }
}

BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 16, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 64, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 256, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 1024, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 4096, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 16'384, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 65'536, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 262'144, 1);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 1'048'576, 1);

BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 16, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 64, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 256, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 1024, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 4096, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 16'384, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 65'536, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 262'144, 2);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 1'048'576, 2);

BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 16, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 64, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 256, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 1024, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 4096, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 16'384, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 65'536, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 262'144, 5);
BENCHMARK_TEMPLATE(MPSC_NonBlockingGet_Get, 1'048'576, 5);

template <size_t min_size, int num_producers, int loop>
static void MPSC_NonBlockingBoth_Put(benchmark::State& state) {
    fastchan::MPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::thread reader([&]() {
        while (shouldRun) {
            auto&& it = c.getWithoutBlocking();
        }
    });

    // create n-1 producers
    std::array<std::thread, num_producers - 1> producers;
    for (auto i = 0; i < num_producers - 1; ++i) {
        producers[i] = std::thread([&]() {
            auto otherWork = 0;
            while (shouldRun) {
                c.putWithoutBlocking(0);
            }
        });
    }

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        c.putWithoutBlocking(0);
    }
    shouldRun = false;

    // clear any blocks
    while (c.getWithoutBlocking()) {
    }
    while (c.putWithoutBlocking(0)) {
    }

    for (auto i = 0; i < num_producers - 1; ++i) {
        producers[i].join();
    }
    reader.join();
}

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 64, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 256, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1024, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 4096, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16'384, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 65'536, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 262'144, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1'048'576, 1, 0);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 64, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 256, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1024, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 4096, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16'384, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 65'536, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 262'144, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1'048'576, 2, 0);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 64, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 256, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1024, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 4096, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16'384, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 65'536, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 262'144, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1'048'576, 5, 0);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 64, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 256, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1024, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 4096, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16'384, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 65'536, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 262'144, 1, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1'048'576, 1, 100);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 64, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 256, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1024, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 4096, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16'384, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 65'536, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 262'144, 2, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1'048'576, 2, 100);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 64, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 256, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1024, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 4096, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 16'384, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 65'536, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 262'144, 5, 100);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Put, 1'048'576, 5, 100);

template <size_t min_size, int num_producers, int loop>
static void MPSC_NonBlockingBoth_Get(benchmark::State& state) {
    fastchan::MPSC<uint8_t, min_size> c;
    std::atomic_bool shouldRun = true;
    std::array<std::thread, num_producers> producers;
    for (auto i = 0; i < num_producers; ++i) {
        producers[i] = std::thread([&]() {
            auto otherWork = 0;
            while (shouldRun) {
                c.putWithoutBlocking(0);
            }
        });
    }

    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto&& it = c.getWithoutBlocking();
    }
    shouldRun = false;

    // clear any blocks
    while (c.getWithoutBlocking()) {
    }
    while (c.putWithoutBlocking(0)) {
    }

    for (auto i = 0; i < num_producers; ++i) {
        producers[i].join();
    }
}

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 16, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 64, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 256, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 1024, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 4096, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 16'384, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 65'536, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 262'144, 1, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 1'048'576, 1, 0);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 16, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 64, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 256, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 1024, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 4096, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 16'384, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 65'536, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 262'144, 2, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 1'048'576, 2, 0);

BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 16, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 64, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 256, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 1024, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 4096, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 16'384, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 65'536, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 262'144, 5, 0);
BENCHMARK_TEMPLATE(MPSC_NonBlockingBoth_Get, 1'048'576, 5, 0);

// Run the benchmark
BENCHMARK_MAIN();

