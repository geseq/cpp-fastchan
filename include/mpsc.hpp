#include <array>
#include <atomic>
#include <optional>
#include <thread>

#include "common.hpp"

namespace fastchan {

template <typename T, size_t min_size>
class MPSC {
   public:
    MPSC() = default;

    void put(const T &value) noexcept {
        auto index = next_free_index_.fetch_add(1, std::memory_order_acq_rel);
        while (index > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            std::this_thread::yield();
        }

        auto write_index = writer_index_.fetch_add(1, std::memory_order_acq_rel);
        contents_[write_index & index_mask_] = value;
        while (!last_committed_index_.compare_exchange_weak(write_index, write_index + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            // commit in the correct order to avoid problems
        }
    }

    bool putWithoutBlocking(const T &value) noexcept {
        auto index = next_free_index_.fetch_add(1, std::memory_order_acq_rel);
        if (index > (reader_index_.load(std::memory_order_acquire) + index_mask_)) {
            next_free_index_.fetch_sub(1, std::memory_order_acq_rel);
            return false;
        }

        auto write_index = writer_index_.fetch_add(1, std::memory_order_acq_rel);
        contents_[write_index & index_mask_] = value;
        while (!last_committed_index_.compare_exchange_weak(write_index, write_index + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            // commit in the correct order to avoid problems
        }
        return true;
    }

    T get() noexcept {
        while (reader_index_2_ >= last_committed_index_.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        auto contents = contents_[reader_index_2_ & index_mask_];
        reader_index_.store(++reader_index_2_, std::memory_order_release);
        return contents;
    }

    std::optional<T> getWithoutBlocking() noexcept {
        if (reader_index_2_ >= last_committed_index_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        auto contents = contents_[reader_index_2_ & index_mask_];
        reader_index_.store(++reader_index_2_, std::memory_order_release);
        return contents;
    }

    void empty() noexcept {
        reader_index_2_ = 0;
        next_free_index_.store(0, std::memory_order_release);
        writer_index_.store(0, std::memory_order_release);
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
    alignas(64) std::atomic<std::size_t> writer_index_{0};
    alignas(64) std::atomic<std::size_t> last_committed_index_{0};
    alignas(64) std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;
};

}  // namespace fastchan
