#include <array>
#include <atomic>
#include <condition_variable>
#include <cwctype>
#include <mutex>
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
                } else if constexpr (wait_type == WaitCondition) {
                    std::unique_lock<std::mutex> lock(put_mutex_);
                    put_cv_.wait(lock, [this] { return next_free_index_2_ <= (reader_index_.load(std::memory_order_acquire) + index_mask_); });
                } else {
                    pause();
                }
            } else {
                return false;
            }
        }

        contents_[next_free_index_2_ & index_mask_] = value;
        next_free_index_.store(++next_free_index_2_, std::memory_order_release);

        if constexpr (wait_type == WaitCondition) {
            std::lock_guard<std::mutex> lock(get_mutex_);
            get_cv_.notify_one();
        }
        if constexpr (blocking_type != BlockingPutBlockingGet && blocking_type != BlockingPutNonBlockingGet) {
            return true;
        }
    }

    get_t get() noexcept {
        while (reader_index_2_ >= next_free_index_.load(std::memory_order_acquire)) {
            if constexpr (blocking_type == BlockingPutBlockingGet || blocking_type == NonBlockingPutBlockingGet) {
                if constexpr (wait_type == WaitYield) {
                    std::this_thread::yield();
                } else if constexpr (wait_type == WaitCondition) {
                    std::unique_lock<std::mutex> lock(get_mutex_);
                    get_cv_.wait(lock, [this] { return reader_index_2_ < next_free_index_.load(std::memory_order_acquire); });
                } else {
                    pause();
                }
            } else {
                return std::nullopt;
            }
        }

        auto contents = contents_[reader_index_2_ & index_mask_];
        reader_index_.store(++reader_index_2_, std::memory_order_release);

        if constexpr (wait_type == WaitCondition) {
            std::lock_guard<std::mutex> lock(put_mutex_);
            put_cv_.notify_one();
        }

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
    std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;

    alignas(64) std::condition_variable put_cv_;
    alignas(64) std::mutex put_mutex_;
    alignas(64) std::condition_variable get_cv_;
    alignas(64) std::mutex get_mutex_;

    const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;

    alignas(64) std::size_t next_free_index_2_{0};
    alignas(64) std::size_t reader_index_2_{0};
    alignas(64) std::atomic<std::size_t> reader_index_{0};
    alignas(64) std::atomic<std::size_t> next_free_index_{0};
};
}  // namespace fastchan

