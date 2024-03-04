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

    SPSC() = default;

    put_t put(const T &value) noexcept {
        while (producer_.next_free_index_2_ > (producer_.reader_index_cache_ + common_.index_mask_)) {
            producer_.reader_index_cache_ = consumer_.reader_index_.load(std::memory_order_acquire);
            if constexpr (std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value) {
                return false;
            } else {
                common_.put_wait_.wait(
                    [this] { return producer_.next_free_index_2_ <= (consumer_.reader_index_.load(std::memory_order_acquire) + common_.index_mask_); });
            }
        }

        contents_[producer_.next_free_index_2_ & common_.index_mask_] = value;
        producer_.next_free_index_.store(++producer_.next_free_index_2_, std::memory_order_release);

        common_.get_wait_.notify();

        if constexpr (std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value) {
            return true;
        }
    }

    get_t get() noexcept {
        while (consumer_.reader_index_2_ >= consumer_.next_free_index_cache_) {
            consumer_.next_free_index_cache_ = producer_.next_free_index_.load(std::memory_order_acquire);
            if constexpr (std::is_same<GetWaitStrategy, ReturnImmediateStrategy>::value) {
                return std::nullopt;
            } else {
                common_.get_wait_.wait([this] { return consumer_.reader_index_2_ < producer_.next_free_index_.load(std::memory_order_acquire); });
            }
        }

        auto contents = contents_[consumer_.reader_index_2_ & common_.index_mask_];
        consumer_.reader_index_.store(++consumer_.reader_index_2_, std::memory_order_release);

        common_.put_wait_.notify();

        return contents;
    }

    std::size_t size() const noexcept {
        return producer_.next_free_index_.load(std::memory_order_acquire) - consumer_.reader_index_.load(std::memory_order_acquire);
    }

    bool isEmpty() const noexcept {
        return consumer_.reader_index_.load(std::memory_order_acquire) >= producer_.next_free_index_.load(std::memory_order_acquire);
    }

    bool isFull() const noexcept {
        return producer_.next_free_index_.load(std::memory_order_relaxed) > (consumer_.reader_index_.load(std::memory_order_acquire) + common_.index_mask_);
    }

   private:
    std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;

    struct alignas(hardware_destructive_interference_size) Common {
        GetWaitStrategy get_wait_{};
        PutWaitStrategy put_wait_{};
        const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    };

    struct alignas(hardware_destructive_interference_size) Producer {
        std::size_t reader_index_cache_{0};
        std::size_t next_free_index_2_{0};
        std::atomic<std::size_t> next_free_index_{0};
    };

    struct alignas(hardware_destructive_interference_size) Consumer {
        std::size_t next_free_index_cache_{0};
        std::size_t reader_index_2_{0};
        std::atomic<std::size_t> reader_index_{0};
    };

    Common common_;
    Producer producer_;
    Consumer consumer_;
};
}  // namespace fastchan

