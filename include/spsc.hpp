#include <array>
#include <atomic>
#include <cwctype>
#include <optional>
#include <thread>
#include <type_traits>

#include "common.hpp"

namespace fastchan {

template <typename T, BlockingType blocking_type, size_t min_size, WaitType wait_type = WaitYield>
class SPSC {
   public:
    using put_t = typename std::conditional<(blocking_type == BlockingPutBlockingGet || blocking_type == BlockingPutNonBlockingGet), void, bool>::type;
    using get_t = typename std::conditional<(blocking_type == BlockingPutBlockingGet || blocking_type == NonBlockingPutBlockingGet), T, std::optional<T>>::type;

    SPSC() = default;

    put_t put(const T &value) noexcept {
        while (next_free_index_2_ > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            if constexpr (blocking_type == BlockingPutBlockingGet || blocking_type == BlockingPutNonBlockingGet) {
                if constexpr (wait_type == WaitYield) {
                    std::this_thread::yield();
                }
            } else {
                return false;
            }
        }

        contents_[next_free_index_2_ & index_mask_] = value;
        next_free_index_.store(++next_free_index_2_, std::memory_order_release);

        if constexpr (blocking_type != BlockingPutBlockingGet && blocking_type != BlockingPutNonBlockingGet) {
            return true;
        }
    }

    get_t get() noexcept {
        while (reader_index_2_ >= next_free_index_.load(std::memory_order_acquire)) {
            if constexpr (blocking_type == BlockingPutBlockingGet || blocking_type == NonBlockingPutBlockingGet) {
                if constexpr (wait_type == WaitYield) {
                    std::this_thread::yield();
                }
            } else {
                return std::nullopt;
            }
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

