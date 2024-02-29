#include <chrono>
#include <condition_variable>
#include <ratio>
#include <thread>

#include "common.hpp"

#ifndef FASTCHANWAIT_HPP
#define FASTCHANWAIT_HPP

namespace fastchan {

// WaitStrategyInterface is the interface for actual implementation of a wait strategy handler
template <typename Implementation>
class WaitStrategyInterface {
   public:
    template <class Predicate>
    inline void wait(Predicate p) {}
    inline void notify() {}
};

class ReturnImmediateStrategy : public WaitStrategyInterface<ReturnImmediateStrategy> {
   public:
    template <class Predicate>
    inline void wait(Predicate p) {}
    inline void notify() {}
};

class NoOpWaitStrategy : public WaitStrategyInterface<NoOpWaitStrategy> {
   public:
    template <class Predicate>
    inline void wait(Predicate p) {}
    inline void notify() {}
};

class PauseWaitStrategy : public WaitStrategyInterface<PauseWaitStrategy> {
   public:
    template <class Predicate>
    inline void wait(Predicate p) {
        cpu_pause();
    }
    inline void notify() {}
};

class YieldWaitStrategy : public WaitStrategyInterface<YieldWaitStrategy> {
   public:
    template <class Predicate>
    inline void wait(Predicate p) {
        std::this_thread::yield();
    }
    inline void notify() {}
};

class CVWaitStrategy : public WaitStrategyInterface<PauseWaitStrategy> {
   public:
    template <class Predicate>
    inline void wait(Predicate p) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::nanoseconds(100), p);
    }

    inline void notify() { cv_.notify_all(); }

   private:
    std::condition_variable cv_;
    std::mutex mutex_;
};

}  // namespace fastchan

#endif
