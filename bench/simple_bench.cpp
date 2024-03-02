#include <pthread.h>

#include <algorithm>
#include <array>
#include <atomic>
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

void set_affinity(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        std::cerr << "Error setting thread affinity: " << result << std::endl;
    }
}

template <typename Chan>
void producer(Chan &chan, int num_producers, int producer_id, int cpu_id = -1) {
    if (cpu_id > -1) {
        set_affinity(cpu_id);
    }

    for (int i = producer_id; i <= num_iterations; i += num_producers) {
        chan.put(i);
    }
}

template <typename Chan>
void consumer(Chan &chan, int cpu_id = -1) {
    if (cpu_id > -1) {
        set_affinity(cpu_id);
    }

    int consumed = 0;
    while (consumed < num_iterations) {
        auto item = chan.get();
        consumed++;
    }
}

template <typename Chan>
void run_benchmark(const std::string &name, int num_producers) {
    Chan chan;

    std::thread consumer_thread(consumer<Chan>, std::ref(chan), -1);

    auto start = std::chrono::steady_clock::now();
    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; i++) {
        producers.emplace_back(producer<Chan>, std::ref(chan), num_producers, i, -1);
    }

    for (auto &t : producers) {
        t.join();
    }
    consumer_thread.join();

    auto end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << name << "\t" << num_producers << " Producers\tUnPinned"
              << "\t" << num_iterations << "\t" << std::setfill(' ') << std::setw(3) << (elapsed / num_iterations) << " ns/op"
              << "\t" << elapsed / 1'000'000 << " ms" << std::endl;
}

template <typename Chan>
void run_spsc_benchmark_for_all_cpu_pairs(const std::string &name) {
    size_t cpu_count = std::thread::hardware_concurrency();

    std::cout << std::fixed;
    std::cout << std::setprecision(1);

    std::cout << name << std::endl;
    std::cout << std::setfill(' ') << std::setw(10) << "C\\P ms";
    for (size_t i = 0; i < cpu_count; ++i) {
        std::cout << std::setfill(' ') << std::setw(8) << "CPU " << std::setfill(' ') << std::setw(2) << i;
    }
    std::cout << std::endl;

    std::vector<std::tuple<double, size_t, size_t>> cost_per_op;
    for (size_t consumer_cpu = 0; consumer_cpu < cpu_count; ++consumer_cpu) {
        std::cout << std::setfill(' ') << std::setw(8) << "CPU " << std::setfill(' ') << std::setw(2) << consumer_cpu;
        for (size_t producer_cpu = 0; producer_cpu < cpu_count; ++producer_cpu) {
            // Skip the iteration if producer and consumer would run on the same CPU.
            if (producer_cpu == consumer_cpu) {
                std::cout << std::setfill(' ') << std::setw(10) << "N/A";
                continue;
            };

            Chan chan;

            auto consumer_thread = std::thread([&] {
                set_affinity(consumer_cpu);
                int val;
                for (int i = 0; i < num_iterations; ++i) {
                    val = chan.get();
                    if (val != i) {
                        throw std::runtime_error("invalid result");
                    }
                }
            });

            set_affinity(producer_cpu);
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < num_iterations; ++i) {
                chan.put(i);
            }

            consumer_thread.join();
            while (!chan.isEmpty())
                ;

            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            auto cost = double(elapsed) / double(num_iterations);

            cost_per_op.emplace_back(cost, producer_cpu, consumer_cpu);
            std::cout << std::setfill(' ') << std::setw(10) << cost;
        }
        std::cout << std::endl;
    }

    // Sort the cost_per_op in ascending order
    std::sort(cost_per_op.begin(), cost_per_op.end(), [](const auto &a, const auto &b) { return std::get<0>(a) < std::get<0>(b); });

    // Display the best 5 cost_per_op and their corresponding CPU pairs
    std::cout << "\nBest 5 Cost per Op (ns/iteration) and their CPU pairs:\n";
    for (size_t i = 0; i < std::min(cost_per_op.size(), size_t(5)); ++i) {
        auto [latency, producer_cpu, consumer_cpu] = cost_per_op[i];
        std::cout << "Cost: " << latency << ", Producer CPU: " << producer_cpu << ", Consumer CPU: " << consumer_cpu << std::endl;
    }

    // Display the worst 5 cost_per_op and their corresponding CPU pairs
    std::cout << "\nWorst 5 Cost per Op (ns/iteration) and their CPU pairs:\n";
    for (size_t i = cost_per_op.size() - 1; i >= std::max(size_t(0), cost_per_op.size() - size_t(5)); --i) {
        auto [latency, producer_cpu, consumer_cpu] = cost_per_op[i];
        std::cout << "Cost: " << latency << ", Producer CPU: " << producer_cpu << ", Consumer CPU: " << consumer_cpu << std::endl;
    }
}

int main() {
#if defined(__linux__)
    run_spsc_benchmark_for_all_cpu_pairs<SPSC<int, 32768, PauseWaitStrategy, PauseWaitStrategy>>("SPSC_Pause");
    std::cout << "============================" << std::endl;
    run_spsc_benchmark_for_all_cpu_pairs<SPSC<int, 32768, NoOpWaitStrategy, NoOpWaitStrategy>>("SPSC_NoOp");
    std::cout << "============================" << std::endl;
    run_spsc_benchmark_for_all_cpu_pairs<SPSC<int, 32768, YieldWaitStrategy, YieldWaitStrategy>>("SPSC_Yield");
    std::cout << "============================" << std::endl;
    run_spsc_benchmark_for_all_cpu_pairs<SPSC<int, 32768, CVWaitStrategy, CVWaitStrategy>>("SPSC_CV");
#else
    run_benchmark<SPSC<int, 32768, YieldWaitStrategy, YieldWaitStrategy>>("SPSC_Yield", 1);
    run_benchmark<SPSC<int, 32768, YieldWaitStrategy, YieldWaitStrategy>>("SPSC_Yield", 1);
    run_benchmark<SPSC<int, 32768, PauseWaitStrategy, PauseWaitStrategy>>("SPSC_Pause", 1);
    run_benchmark<SPSC<int, 32768, PauseWaitStrategy, PauseWaitStrategy>>("SPSC_Pause", 1);
    run_benchmark<SPSC<int, 32768, CVWaitStrategy, CVWaitStrategy>>("SPSC_Cond", 1);
    run_benchmark<SPSC<int, 32768, CVWaitStrategy, CVWaitStrategy>>("SPSC_Cond", 1);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 1);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 1);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 1);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 1);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 1);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 1);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 3);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 3);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 3);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 3);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 3);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 3);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 5);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 5);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 5);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 5);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 5);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 5);

    std::cout << "============================" << std::endl;

    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 7);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitYield>>("MPSC_Yield", 7);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 7);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitPause>>("MPSC_Pause", 7);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 7);
    run_benchmark<MPSC<int, BlockingPutBlockingGet, 32768, WaitCondition>>("MPSC_Cond", 7);
    ;

    std::cout << "This benchmark requires a Linux platform to run. Exiting." << std::endl;
#endif
    return 0;
}
