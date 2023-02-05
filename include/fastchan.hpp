#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ostream>
#include <thread>

namespace fastchan {

size_t roundUpNextPowerOfTwo(size_t v) {
    v--;
    for (size_t i = 1; i < sizeof(v) * CHAR_BIT; i *= 2) {
        v |= v >> i;
    }
    return ++v;
}

template <typename T, size_t min_size>
class FastChan {
   public:
    FastChan() : size_(roundUpNextPowerOfTwo(min_size)), index_mask_(size_ - 1), last_committed_index_(0), next_free_index_(1), reader_index_(1) {
        wait_for_reader_.clear();
        wait_for_writer_.clear();
    }

    void put(const T &value) {
        std::size_t my_index = next_free_index_.fetch_add(1);
        while (my_index > (reader_index_.load() + size_ - 1)) {
            wait_for_reader_.wait(true);
        }

        contents_[my_index & index_mask_] = value;

        auto prev_index = my_index - 1;
        while (!last_committed_index_.compare_exchange_strong(prev_index, my_index)) {
            wait_for_reader_.wait(true);
        }
        wait_for_writer_.clear();
    }

    T read() {
        while (reader_index_.load() > last_committed_index_.load()) {
            wait_for_writer_.wait(true);
        }

        auto contents = contents_[reader_index_.load() & index_mask_];
        reader_index_.fetch_add(1);
        wait_for_reader_.clear();
        return contents;
    }

    void empty() {
        last_committed_index_.store(0);
        next_free_index_.store(1);
        reader_index_.store(1);
    }

    std::size_t size() const { return last_committed_index_.load() - reader_index_.load() + 1; }

    bool isEmpty() const { return reader_index_.load() >= last_committed_index_.load(); }

    bool isFull() const { return next_free_index_.load() >= (reader_index_.load() + index_mask_); }

   private:
    const std::size_t size_;
    const std::size_t index_mask_;
    alignas(64) std::atomic<std::size_t> last_committed_index_;
    alignas(64) std::atomic<std::size_t> next_free_index_;
    alignas(64) std::atomic<std::size_t> reader_index_;
    alignas(64) std::atomic_flag wait_for_reader_;
    alignas(64) std::atomic_flag wait_for_writer_;
    std::array<T, min_size> contents_;
};

}  // namespace fastchan

