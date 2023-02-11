#include <benchmark/benchmark.h>

#include <chrono>
#include <fastchan.hpp>
#include <iostream>
#include <thread>

#include "boost/lockfree/policies.hpp"
#include "boost/lockfree/spsc_queue.hpp"

template <size_t min_size>
static void FastChan_BlockingBoth_Put(benchmark::State& state) {
    fastchan::FastChan<uint8_t, min_size> c;
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

BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 16);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 64);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 256);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 1024);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 4096);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 16'384);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 65'536);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 262'144);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Put, 1'048'576);

template <size_t min_size>
static void FastChan_BlockingBoth_Get(benchmark::State& state) {
    fastchan::FastChan<uint8_t, min_size> c;
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

BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 16);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 64);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 256);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 1024);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 4096);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 16'384);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 65'536);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 262'144);
BENCHMARK_TEMPLATE(FastChan_BlockingBoth_Get, 1'048'576);

template <size_t min_size>
static void FastChan_NonBlockingGet_Put(benchmark::State& state) {
    fastchan::FastChan<uint8_t, min_size> c;
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

BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 16);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 64);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 256);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 1024);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 4096);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 16'384);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 65'536);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 262'144);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Put, 1'048'576);

template <size_t min_size>
static void FastChan_NonBlockingGet_Get(benchmark::State& state) {
    fastchan::FastChan<uint8_t, min_size> c;
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

BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 16);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 64);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 256);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 1024);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 4096);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 16'384);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 65'536);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 262'144);
BENCHMARK_TEMPLATE(FastChan_NonBlockingGet_Get, 1'048'576);

template <size_t min_size>
static void FastChan_NonBlockingBoth_Put(benchmark::State& state) {
    fastchan::FastChan<uint8_t, min_size> c;
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

BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 16);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 64);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 256);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 1024);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 4096);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 16'384);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 65'536);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 262'144);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Put, 1'048'576);

template <size_t min_size>
static void FastChan_NonBlockingBoth_Get(benchmark::State& state) {
    fastchan::FastChan<uint8_t, min_size> c;
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

BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 16);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 64);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 256);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 1024);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 4096);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 16'384);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 65'536);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 262'144);
BENCHMARK_TEMPLATE(FastChan_NonBlockingBoth_Get, 1'048'576);

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

// Run the benchmark
BENCHMARK_MAIN();
