#include <array>
#include <atomic>
#include <limits>
#include <thread>

namespace fastchan {

#ifndef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__
#endif

constexpr size_t roundUpNextPowerOfTwo(size_t v) {
    v--;
    for (size_t i = 1; i < sizeof(v) * CHAR_BIT; i *= 2) {
        v |= v >> i;
    }
    return ++v;
}

template <typename T, size_t min_size>
class FastChan {
   public:
    FastChan() : last_committed_index_(0), next_free_index_(1), reader_index_(1) {}

    void put(const T &value) noexcept {
        auto my_index = next_free_index_.fetch_add(1, std::memory_order_acq_rel);
        while (my_index > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            std::this_thread::yield();
        }

        contents_[my_index & index_mask_] = value;
        last_committed_index_.store(my_index, std::memory_order_release);
    }

    T get() noexcept {
        while (reader_index_.load(std::memory_order_acquire) > last_committed_index_.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        auto contents = contents_[reader_index_.load(std::memory_order_acquire) & index_mask_];
        reader_index_.fetch_add(1, std::memory_order_acq_rel);
        return contents;
    }

    void empty() {
        last_committed_index_.store(0, std::memory_order_release);
        next_free_index_.store(1, std::memory_order_release);
        reader_index_.store(1, std::memory_order_release);
    }

    std::size_t size() const { return last_committed_index_.load() - reader_index_.load() + 1; }

    bool isEmpty() const { return reader_index_.load() >= last_committed_index_.load(); }

    bool isFull() const { return next_free_index_.load() >= (reader_index_.load() + index_mask_); }

   private:
    const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    alignas(64) std::atomic<std::size_t> last_committed_index_;
    alignas(64) std::atomic<std::size_t> next_free_index_;
    alignas(64) std::atomic<std::size_t> reader_index_;
    alignas(64) std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;
};

}  // namespace fastchan

