#include <sys/types.h>

#include <chrono>
#include <cstdint>
#include <fastchan.hpp>
#include <iostream>
#include <thread>
#include <vector>

template <size_t min_size>
void benchmarkFastChanPut(int n) {
    fastchan::FastChan<uint8_t, min_size> c;

    std::thread reader([&]() {
        for (int i = 0; i < n; i++) {
            c.get();
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        c.put(0);
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkFastChanPut" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkFastChanGet(int n) {
    fastchan::FastChan<uint8_t, min_size> c;

    std::thread reader([&]() {
        for (int i = 0; i < n; i++) {
            c.put(0);
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        c.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkFastChanGet" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

int main() {
    auto n = 5'000'000;
    benchmarkFastChanPut<16>(n);
    benchmarkFastChanPut<64>(n);
    benchmarkFastChanPut<256>(n);
    benchmarkFastChanPut<1024>(n);
    benchmarkFastChanPut<4096>(n);
    benchmarkFastChanPut<16'384>(n);
    benchmarkFastChanPut<65'536>(n);
    benchmarkFastChanPut<262'144>(n);
    benchmarkFastChanPut<1'048'576>(n);

    benchmarkFastChanGet<16>(n);
    benchmarkFastChanGet<64>(n);
    benchmarkFastChanGet<256>(n);
    benchmarkFastChanGet<1024>(n);
    benchmarkFastChanGet<4096>(n);
    benchmarkFastChanGet<16'384>(n);
    benchmarkFastChanGet<65'536>(n);
    benchmarkFastChanGet<262'144>(n);
    benchmarkFastChanGet<1'048'576>(n);

    return 0;
}

