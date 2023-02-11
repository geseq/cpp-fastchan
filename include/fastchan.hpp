#include <array>
#include <atomic>
#include <limits>
#include <optional>
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
    FastChan() = default;

    void put(const T &value) noexcept {
        while (next_free_index_2_ > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            std::this_thread::yield();
        }

        contents_[next_free_index_2_ & index_mask_] = value;
        next_free_index_.store(++next_free_index_2_, std::memory_order_release);
    }

    bool putWithoutBlocking(const T &value) noexcept {
        if (next_free_index_2_ > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            return false;
        }

        contents_[next_free_index_2_ & index_mask_] = value;
        next_free_index_.store(++next_free_index_2_, std::memory_order_release);
        return true;
    }

    T get() noexcept {
        while (reader_index_2_ >= next_free_index_.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        auto contents = contents_[reader_index_2_ & index_mask_];
        reader_index_.store(++reader_index_2_, std::memory_order_release);
        return contents;
    }

    std::optional<T> getWithoutBlocking() noexcept {
        if (reader_index_2_ >= next_free_index_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        auto contents = contents_[reader_index_2_ & index_mask_];
        reader_index_.store(++reader_index_2_, std::memory_order_release);
        return contents;
    }

    void empty() noexcept {
        reader_index_2_ = 0;
        next_free_index_2_ = 0;
        next_free_index_.store(0, std::memory_order_release);
        reader_index_.store(0, std::memory_order_release);
    }

    std::size_t size() const noexcept { return next_free_index_.load(std::memory_order_acquire) - reader_index_.load(std::memory_order_acquire); }

    bool isEmpty() const noexcept { return reader_index_.load(std::memory_order_acquire) >= next_free_index_.load(std::memory_order_acquire); }

    bool isFull() const noexcept { return next_free_index_.load(std::memory_order_relaxed) > (reader_index_.load(std::memory_order_acquire) + index_mask_); }

   private:
    const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    alignas(64) std::size_t next_free_index_2_{0};
    alignas(64) std::size_t reader_index_2_{0};
    alignas(64) std::atomic<std::size_t> reader_index_{0};
    alignas(64) std::atomic<std::size_t> next_free_index_{0};
    alignas(64) std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;
};

}  // namespace fastchan

