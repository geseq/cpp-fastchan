#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>

#include "common.hpp"
#include "wait_strategy.hpp"

namespace fastchan {

template <typename T, size_t min_size, class PutWaitStrategy = YieldWaitStrategy, class GetWaitStrategy = YieldWaitStrategy>
class MPSC {
   public:
    using put_t = typename std::conditional<!std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value, void, bool>::type;
    using get_t = typename std::conditional<!std::is_same<GetWaitStrategy, ReturnImmediateStrategy>::value, T, std::optional<T>>::type;

    MPSC() = default;

    put_t put(const T &value) noexcept {
        auto write_index = next_free_index_.load(std::memory_order_acquire);
        do {
            while (write_index > (consumer_.reader_index_.load(std::memory_order_relaxed) + common_.index_mask_)) {
                if constexpr (std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value) {
                    return false;
                } else {
                    common_.put_wait_.wait(
                        [this, write_index] { return write_index <= (consumer_.reader_index_.load(std::memory_order_relaxed) + common_.index_mask_); });
                }
            }
        } while (!next_free_index_.compare_exchange_strong(write_index, write_index + 1, std::memory_order_acq_rel, std::memory_order_acquire));

        contents_[write_index & common_.index_mask_] = value;

        // commit in the correct order to avoid problems
        while (last_committed_index_.load(std::memory_order_relaxed) != write_index) {
            // we don't return at this point even in case of ReturnImmediatelyStrategy as we've already taken the token
            common_.put_wait_.wait([this, write_index] { return last_committed_index_.load(std::memory_order_relaxed) == write_index; });
        }

        last_committed_index_.store(++write_index, std::memory_order_release);

        common_.get_wait_.notify();
        common_.put_wait_.notify();

        if constexpr (std::is_same<PutWaitStrategy, ReturnImmediateStrategy>::value) {
            return true;
        }
    }

    get_t get() noexcept {
        while (consumer_.reader_index_2_ >= last_committed_index_.load(std::memory_order_relaxed)) {
            if constexpr (std::is_same<GetWaitStrategy, ReturnImmediateStrategy>::value) {
                return std::nullopt;
            } else {
                common_.get_wait_.wait([this] { return consumer_.reader_index_2_ < last_committed_index_.load(std::memory_order_relaxed); });
            }
        }

        auto contents = contents_[consumer_.reader_index_2_ & common_.index_mask_];
        consumer_.reader_index_.store(++consumer_.reader_index_2_, std::memory_order_release);

        common_.put_wait_.notify();

        return contents;
    }

    void empty() noexcept {
        consumer_.reader_index_2_ = 0;
        next_free_index_.store(0, std::memory_order_release);
        last_committed_index_.store(0, std::memory_order_release);
        consumer_.reader_index_.store(0, std::memory_order_release);
    }

    std::size_t size() const noexcept {
        return last_committed_index_.load(std::memory_order_acquire) - consumer_.reader_index_.load(std::memory_order_acquire);
    }

    bool isEmpty() const noexcept { return consumer_.reader_index_.load(std::memory_order_acquire) >= last_committed_index_.load(std::memory_order_acquire); }

    bool isFull() const noexcept {
        // this isFull is about whether there's all writer slots to the buffer are taken rather than whether those
        // changes have actually been committed
        return next_free_index_.load(std::memory_order_acquire) > (consumer_.reader_index_.load(std::memory_order_acquire) + common_.index_mask_);
    }

   private:
    std::array<T, roundUpNextPowerOfTwo(min_size)> contents_;

    alignas(64) std::atomic<std::size_t> next_free_index_{0};
    alignas(64) std::atomic<std::size_t> last_committed_index_{0};

    struct alignas(64) Common {
        GetWaitStrategy get_wait_{};
        PutWaitStrategy put_wait_{};
        const std::size_t index_mask_ = roundUpNextPowerOfTwo(min_size) - 1;
    };

    struct alignas(64) Consumer {
        std::size_t next_free_index_cache_{0};
        std::size_t reader_index_2_{0};
        std::atomic<std::size_t> reader_index_{0};
    };

    Common common_;
    Consumer consumer_;
};

}  // namespace fastchan
