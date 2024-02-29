#include <array>
#include <atomic>
#include <condition_variable>
#include <cwctype>
#include <mutex>
#include <optional>
#include <thread>
#include <type_traits>

#include "common.hpp"
#include "wait_strategy.hpp"

namespace fastchan {

template <typename T, size_t min_size, class PutWaitStrategy = YieldWaitStrategy, class GetWaitStrategy = YieldWaitStrategy>
class SPSC {
   public:
    using put_t = typename std::conditional<!std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value, void, bool>::type;
    using get_t = typename std::conditional<!std::is_same<GetWaitStrategy, ReturnImmediateStrategy>::value, T, std::optional<T>>::type;

    SPSC() : next_free_index_(0), reader_index_(0) {}

    put_t put(const T &value) noexcept {
        while (next_free_index_2_ > (reader_index_cache_ + index_mask_)) {
            reader_index_cache_ = reader_index_.load(std::memory_order_acquire);
            if constexpr (std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value) {
                return false;
            } else {
                put_wait_.wait([this] { return next_free_index_2_ <= (reader_index_.load(std::memory_order_acquire) + index_mask_); });
            }
        }

        contents_[next_free_index_2_ & index_mask_] = value;
        next_free_index_.store(++next_free_index_2_, std::memory_order_release);

        get_wait_.notify();

        if constexpr (std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value) {
            return true;
        }
    }

    get_t get() noexcept {
        while (reader_index_2_ >= next_free_index_cache_) {
            next_free_index_cache_ = next_free_index_.load(std::memory_order_acquire);
            if constexpr (std::is_same<GetWaitStrategy, ReturnImmediateStrategy>::value) {
                return std::nullopt;
            } else {
                get_wait_.wait([this] { return reader_index_2_ < next_free_index_.load(std::memory_order_acquire); });
            }
        }

        auto contents = contents_[reader_index_2_ & index_mask_];
        reader_index_.store(++reader_index_2_, std::memory_order_release);

        put_wait_.notify();

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

    struct alignas(64) {
        GetWaitStrategy get_wait_{};
        PutWaitStrategy put_wait_{};
        const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    };

    struct alignas(64) {
        std::size_t reader_index_cache_{0};
        std::size_t next_free_index_2_{0};
        std::atomic<std::size_t> next_free_index_;
    };

    struct alignas(64) {
        std::size_t next_free_index_cache_{0};
        std::size_t reader_index_2_{0};
        std::atomic<std::size_t> reader_index_;
    };
};
}  // namespace fastchan

