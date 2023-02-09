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
        auto my_index = next_free_index_.load(std::memory_order_relaxed);
        while (my_index > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            std::this_thread::yield();
        }

        contents_[my_index & index_mask_] = value;
        next_free_index_.store(my_index + 1, std::memory_order_release);
    }

    bool putWithoutBlocking(const T &value) noexcept {
        auto my_index = next_free_index_.load(std::memory_order_relaxed);
        if (my_index > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            return false;
        }

        contents_[my_index & index_mask_] = value;
        next_free_index_.store(my_index + 1, std::memory_order_release);
        return true;
    }

    T get() noexcept {
        auto my_index = reader_index_.load(std::memory_order_relaxed);
        while (my_index + 1 > next_free_index_.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        auto contents = contents_[my_index & index_mask_];
        reader_index_.store(my_index + 1, std::memory_order_release);
        return contents;
    }

    std::optional<T> getWithoutBlocking() noexcept {
        auto my_index = reader_index_.load(std::memory_order_relaxed);
        if (my_index + 1 > next_free_index_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        auto contents = contents_[my_index & index_mask_];
        reader_index_.store(my_index + 1, std::memory_order_release);
        return contents;
    }

    void empty() noexcept {
        next_free_index_.store(1, std::memory_order_release);
        reader_index_.store(1, std::memory_order_release);
    }

    std::size_t size() const noexcept { return next_free_index_.load(std::memory_order_acquire) - reader_index_.load(std::memory_order_acquire); }

    bool isEmpty() const noexcept { return reader_index_.load(std::memory_order_acquire) > next_free_index_.load(std::memory_order_acquire) - 1; }

    bool isFull() const noexcept { return next_free_index_.load(std::memory_order_relaxed) > (reader_index_.load(std::memory_order_acquire) + index_mask_); }

   private:
    const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    alignas(64) std::atomic<std::size_t> reader_index_{1};
    alignas(64) std::atomic<std::size_t> next_free_index_{1};
    alignas(64) std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;
};

}  // namespace fastchan

