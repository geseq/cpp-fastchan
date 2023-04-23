#include <pthread.h>

#include <array>
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "common.hpp"
#include "mpsc.hpp"
#include "spsc.hpp"

using namespace fastchan;

constexpr int num_iterations = 100'000'000;

template <typename Chan>
void producer(Chan &chan, int num_producers, int producer_id) {
    for (int i = producer_id; i <= num_iterations; i += num_producers) {
        chan.put(i);
    }
}

template <typename Chan>
void consumer(Chan &chan) {
    int consumed = 0;
    while (consumed < num_iterations) {
        auto item = chan.get();
        consumed++;
    }
}

void set_affinity(std::thread &t, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    auto native_handle = t.native_handle();
    int result = pthread_setaffinity_np(native_handle, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        std::cerr << "Error setting thread affinity: " << result << std::endl;
    }
}

template <typename Chan>
void run_benchmark(const std::string &name, int num_producers, bool pin_threads) {
    Chan chan;

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; i++) {
        producers.emplace_back(producer<Chan>, std::ref(chan), num_producers, i);
        if (pin_threads) {
            set_affinity(producers.back(), i % std::thread::hardware_concurrency());
        }
    }
    std::thread consumer_thread(consumer<Chan>, std::ref(chan));
    if (pin_threads) {
        set_affinity(consumer_thread, num_producers % std::thread::hardware_concurrency());
    }

    for (auto &t : producers) {
        t.join();
    }
    consumer_thread.join();

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << name << "\t" << num_producers << " Producers"
              << "\t" << (pin_threads ? "  Pinned" : "UnPinned") << "\t" << num_iterations << "\t" << std::setfill(' ') << std::setw(3)
              << (elapsed / num_iterations) << " ns/op"
              << "\t" << elapsed / 1'000'000 << " ms" << std::endl;
}

int main() {
#if defined(__linux__)
    run_benchmark<SPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("SPSC_Yield", 1, true);
    run_benchmark<SPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("SPSC_Yield", 1, false);
    run_benchmark<SPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("SPSC_Spin", 1, true);
    run_benchmark<SPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("SPSC_Spin", 1, false);
    run_benchmark<SPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("SPSC_Cond", 1, true);
    run_benchmark<SPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("SPSC_Cond", 1, false);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 1, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 1, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 1, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 1, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 1, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 1, false);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 3, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 3, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 3, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 3, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 3, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 3, false);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 5, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 5, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 5, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 5, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 5, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 5, false);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 7, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitYield>>("MPSC_Yield", 7, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 7, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitSpin>>("MPSC_Spin", 7, false);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 7, true);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 1024, WaitCondition>>("MPSC_Cond", 7, false);

#else
    std::cout << "This benchmark requires a Linux platform to run. Exiting." << std::endl;
#endif
    return 0;
}
