#include <chrono>
#include <fastchan.hpp>
#include <iostream>
#include <thread>

#include "boost/lockfree/policies.hpp"
#include "boost/lockfree/spsc_queue.hpp"

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
    std::cout << "BenchmarkFastChan-Put" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
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
    std::cout << "BenchmarkFastChan-Get" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkFastChanPutNonBlockingGet(int n) {
    fastchan::FastChan<uint8_t, min_size> c;

    std::thread reader([&]() {
        for (int i = 0; i < n;) {
            if (c.getWithoutBlocking()) {
                ++i;
            }
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        c.put(1);
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkFastChan-NonBlockingGet-Put" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkFastChanGetNonBlockingGet(int n) {
    fastchan::FastChan<uint8_t, min_size> c;

    std::thread reader([&]() {
        for (int i = 0; i < n; ++i) {
            c.put(1);
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n;) {
        if (c.getWithoutBlocking()) {
            ++i;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkFastChan-NonBlockingGet-Get" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkFastChanNonBlockingPut(int n) {
    fastchan::FastChan<uint8_t, min_size> c;

    std::thread reader([&]() {
        for (int i = 0; i < n;) {
            if (c.getWithoutBlocking()) {
                ++i;
            }
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n;) {
        if (c.putWithoutBlocking(1)) {
            ++i;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkFastChanNonBlocking-Put" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkFastChanNonBlockingGet(int n) {
    fastchan::FastChan<uint8_t, min_size> c;

    std::thread reader([&]() {
        for (int i = 0; i < n;) {
            if (c.putWithoutBlocking(1)) {
                ++i;
            }
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n;) {
        if (c.getWithoutBlocking()) {
            ++i;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkFastChanNonBlocking-Get" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkBoostSPSCPut(int n) {
    boost::lockfree::spsc_queue<uint8_t, boost::lockfree::capacity<min_size>> c;

    std::thread reader([&]() {
        for (int i = 0; i < n;) {
            if (c.pop()) {
                ++i;
            }
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n;) {
        if (c.push(1)) {
            ++i;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkBoostSPSC-Put" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
}

template <size_t min_size>
void benchmarkBoostSPSCGet(int n) {
    boost::lockfree::spsc_queue<uint8_t, boost::lockfree::capacity<min_size>> c;

    std::thread reader([&]() {
        for (int i = 0; i < n;) {
            if (c.push(1)) {
                ++i;
            }
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n;) {
        if (c.pop()) {
            ++i;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    reader.join();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "BenchmarkBoostSPSC-Get" << min_size << "\t" << n << "\t" << duration / n << " ns/op" << std::endl;
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

    std::cout << std::endl;

    benchmarkFastChanPutNonBlockingGet<16>(n);
    benchmarkFastChanPutNonBlockingGet<64>(n);
    benchmarkFastChanPutNonBlockingGet<256>(n);
    benchmarkFastChanPutNonBlockingGet<1024>(n);
    benchmarkFastChanPutNonBlockingGet<4096>(n);
    benchmarkFastChanPutNonBlockingGet<16'384>(n);
    benchmarkFastChanPutNonBlockingGet<65'536>(n);
    benchmarkFastChanPutNonBlockingGet<262'144>(n);
    benchmarkFastChanPutNonBlockingGet<1'048'576>(n);

    benchmarkFastChanGetNonBlockingGet<16>(n);
    benchmarkFastChanGetNonBlockingGet<64>(n);
    benchmarkFastChanGetNonBlockingGet<256>(n);
    benchmarkFastChanGetNonBlockingGet<1024>(n);
    benchmarkFastChanGetNonBlockingGet<4096>(n);
    benchmarkFastChanGetNonBlockingGet<16'384>(n);
    benchmarkFastChanGetNonBlockingGet<65'536>(n);
    benchmarkFastChanGetNonBlockingGet<262'144>(n);
    benchmarkFastChanGetNonBlockingGet<1'048'576>(n);

    std::cout << std::endl;

    benchmarkFastChanNonBlockingPut<16>(n);
    benchmarkFastChanNonBlockingPut<64>(n);
    benchmarkFastChanNonBlockingPut<256>(n);
    benchmarkFastChanNonBlockingPut<1024>(n);
    benchmarkFastChanNonBlockingPut<4096>(n);
    benchmarkFastChanNonBlockingPut<16'384>(n);
    benchmarkFastChanNonBlockingPut<65'536>(n);
    benchmarkFastChanNonBlockingPut<262'144>(n);
    benchmarkFastChanNonBlockingPut<1'048'576>(n);

    benchmarkFastChanNonBlockingGet<16>(n);
    benchmarkFastChanNonBlockingGet<64>(n);
    benchmarkFastChanNonBlockingGet<256>(n);
    benchmarkFastChanNonBlockingGet<1024>(n);
    benchmarkFastChanNonBlockingGet<4096>(n);
    benchmarkFastChanNonBlockingGet<16'384>(n);
    benchmarkFastChanNonBlockingGet<65'536>(n);
    benchmarkFastChanNonBlockingGet<262'144>(n);
    benchmarkFastChanNonBlockingGet<1'048'576>(n);

    std::cout << std::endl;

    benchmarkBoostSPSCPut<16>(n);
    benchmarkBoostSPSCPut<64>(n);
    benchmarkBoostSPSCPut<256>(n);
    benchmarkBoostSPSCPut<1024>(n);
    benchmarkBoostSPSCPut<4096>(n);
    benchmarkBoostSPSCPut<16'384>(n);
    benchmarkBoostSPSCPut<65'536>(n);
    benchmarkBoostSPSCPut<262'144>(n);
    benchmarkBoostSPSCPut<1'048'576>(n);

    benchmarkBoostSPSCGet<16>(n);
    benchmarkBoostSPSCGet<64>(n);
    benchmarkBoostSPSCGet<256>(n);
    benchmarkBoostSPSCGet<1024>(n);
    benchmarkBoostSPSCGet<4096>(n);
    benchmarkBoostSPSCGet<16'384>(n);
    benchmarkBoostSPSCGet<65'536>(n);
    benchmarkBoostSPSCGet<262'144>(n);
    benchmarkBoostSPSCGet<1'048'576>(n);

    return 0;
}

