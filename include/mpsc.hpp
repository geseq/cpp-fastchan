#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>

#include "common.hpp"

namespace fastchan {

template <typename T, BlockingType blocking_type, size_t min_size, WaitType wait_type = WaitYield>
class MPSC {
   public:
    using put_t = typename std::conditional<(blocking_type == BlockingPutBlockingGet || blocking_type == BlockingPutNonBlockingGet), void, bool>::type;
    using get_t = typename std::conditional<(blocking_type == BlockingPutBlockingGet || blocking_type == NonBlockingPutBlockingGet), T, std::optional<T>>::type;

    MPSC() = default;

    put_t put(const T &value) noexcept {
        auto write_index = next_free_index_.load(std::memory_order_acquire);
        do {
            while (write_index > (reader_index_.load(std::memory_order_relaxed) + index_mask_)) {
                if constexpr (blocking_type == BlockingPutBlockingGet || blocking_type == BlockingPutNonBlockingGet) {
                    if constexpr (wait_type == WaitYield) {
                        std::this_thread::yield();
                    } else if constexpr (wait_type == WaitCondition) {
                        std::unique_lock<std::mutex> lock(put_mutex_);
                        put_cv_.wait(lock, [this, write_index] { return write_index <= (reader_index_.load(std::memory_order_relaxed) + index_mask_); });
                    } else if constexpr (wait_type == WaitSpin) {
                        cpu_pause();
                    }
                } else {
                    return false;
                }
            }
        } while (!next_free_index_.compare_exchange_strong(write_index, write_index + 1, std::memory_order_acq_rel, std::memory_order_acquire));

        contents_[write_index & index_mask_] = value;

        // commit in the correct order to avoid problems
        while (last_committed_index_.load(std::memory_order_relaxed) != write_index) {
            if constexpr (wait_type == WaitYield) {
                std::this_thread::yield();
            } else if constexpr (wait_type == WaitSpin) {
                cpu_pause();
            }
        }

        last_committed_index_.store(++write_index, std::memory_order_release);

        if constexpr (wait_type == WaitCondition) {
            std::lock_guard<std::mutex> lock(get_mutex_);
            get_cv_.notify_one();
        }

        if constexpr (blocking_type != BlockingPutBlockingGet && blocking_type != BlockingPutNonBlockingGet) {
            return true;
        }
    }

    get_t get() noexcept {
        while (reader_index_2_ >= last_committed_index_.load(std::memory_order_relaxed)) {
            if constexpr (blocking_type == BlockingPutBlockingGet || blocking_type == NonBlockingPutBlockingGet) {
                if constexpr (wait_type == WaitYield) {
                    std::this_thread::yield();
                } else if constexpr (wait_type == WaitCondition) {
                    std::unique_lock<std::mutex> lock(get_mutex_);
                    get_cv_.wait(lock, [this] { return reader_index_2_ < last_committed_index_.load(std::memory_order_relaxed); });
                } else if constexpr (wait_type == WaitSpin) {
                    cpu_pause();
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
        next_free_index_.store(0, std::memory_order_release);
        last_committed_index_.store(0, std::memory_order_release);
        reader_index_.store(0, std::memory_order_release);
    }

    std::size_t size() const noexcept { return last_committed_index_.load(std::memory_order_acquire) - reader_index_.load(std::memory_order_acquire); }

    bool isEmpty() const noexcept { return reader_index_.load(std::memory_order_acquire) >= last_committed_index_.load(std::memory_order_acquire); }

    bool isFull() const noexcept {
        // this isFull is about whether there's all writer slots to the buffer are taken rather than whether those
        // changes have actually been committed
        return next_free_index_.load(std::memory_order_acquire) > (reader_index_.load(std::memory_order_acquire) + index_mask_);
    }

   private:
    const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    alignas(64) std::size_t reader_index_2_{0};
    alignas(64) std::atomic<std::size_t> reader_index_{0};
    alignas(64) std::atomic<std::size_t> next_free_index_{0};
    alignas(64) std::atomic<std::size_t> last_committed_index_{0};

    alignas(64) std::condition_variable put_cv_;
    alignas(64) std::condition_variable get_cv_;
    alignas(64) std::mutex put_mutex_;
    alignas(64) std::mutex get_mutex_;

    alignas(64) std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;
};

}  // namespace fastchan
