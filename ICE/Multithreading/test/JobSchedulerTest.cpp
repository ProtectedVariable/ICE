#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <numeric>
#include <thread>
#include <vector>

#include "JobScheduler.h"

using namespace ICE;

namespace {
// Spin-wait until `pred` holds or the timeout elapses; returns whether it became true. Used to wait
// on detached (submit) completion, which has no built-in join.
template<typename Pred>
bool waitFor(Pred pred, std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (pred()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return pred();
}
}  // namespace

// Every index in [0, count) runs exactly once.
TEST(JobSchedulerTest, DispatchRunsEachIndexOnce) {
    JobScheduler scheduler;
    constexpr std::size_t count = 10000;
    std::vector<std::atomic<int>> hits(count);
    for (auto& h : hits) h.store(0);

    scheduler.dispatch(count, [&](std::size_t i) { hits[i].fetch_add(1, std::memory_order_relaxed); });

    for (std::size_t i = 0; i < count; ++i) {
        ASSERT_EQ(hits[i].load(), 1) << "index " << i << " ran " << hits[i].load() << " times";
    }
}

// Concurrent accumulation matches the serial sum (no lost updates).
TEST(JobSchedulerTest, DispatchParallelSumIsCorrect) {
    JobScheduler scheduler;
    constexpr std::size_t count = 100000;
    std::atomic<long long> sum{0};

    scheduler.dispatch(count, [&](std::size_t i) { sum.fetch_add(static_cast<long long>(i), std::memory_order_relaxed); });

    const long long expected = static_cast<long long>(count - 1) * static_cast<long long>(count) / 2;
    ASSERT_EQ(sum.load(), expected);
}

// parallelRanges partitions [0, count) into contiguous, non-overlapping ranges covering everything.
TEST(JobSchedulerTest, ParallelRangesPartitionsExactly) {
    JobScheduler scheduler;
    constexpr std::size_t count = 12345;
    std::vector<std::atomic<int>> hits(count);
    for (auto& h : hits) h.store(0);

    scheduler.parallelRanges(count, [&](std::size_t begin, std::size_t end) {
        for (std::size_t i = begin; i < end; ++i) hits[i].fetch_add(1, std::memory_order_relaxed);
    });

    for (std::size_t i = 0; i < count; ++i) {
        ASSERT_EQ(hits[i].load(), 1) << "index " << i;
    }
}

// A single-worker scheduler still runs everything correctly.
TEST(JobSchedulerTest, SingleWorkerWorks) {
    JobScheduler scheduler(1);
    std::atomic<int> counter{0};
    scheduler.dispatch(1000, [&](std::size_t) { counter.fetch_add(1, std::memory_order_relaxed); });
    ASSERT_EQ(counter.load(), 1000);
}

// Zero-count dispatch is a no-op and doesn't hang.
TEST(JobSchedulerTest, EmptyDispatchIsNoop) {
    JobScheduler scheduler;
    int calls = 0;
    scheduler.dispatch(0, [&](std::size_t) { ++calls; });
    ASSERT_EQ(calls, 0);
}

// An exception thrown in a job propagates to the caller (and doesn't deadlock).
TEST(JobSchedulerTest, JobExceptionPropagates) {
    JobScheduler scheduler;
    auto run = [&] {
        scheduler.dispatch(64, [](std::size_t i) {
            if (i == 17) throw std::runtime_error("boom");
        });
    };
    ASSERT_THROW(run(), std::runtime_error);
}

// Every detached (submit) job runs exactly once.
TEST(JobSchedulerTest, SubmitRunsAllDetachedJobs) {
    JobScheduler scheduler;
    constexpr int count = 500;
    std::atomic<int> ran{0};
    for (int i = 0; i < count; ++i) {
        scheduler.submit([&] { ran.fetch_add(1, std::memory_order_relaxed); });
    }
    ASSERT_TRUE(waitFor([&] { return ran.load() == count; })) << "only " << ran.load() << " of " << count << " ran";
}

// Interleaving submit() with dispatch() batches: every batch completes correctly (never hangs or
// returns early) and all detached jobs eventually run. This targets the m_pending / m_pending_detached
// separation directly.
TEST(JobSchedulerTest, SubmitInterleavedWithDispatchDoesNotHang) {
    JobScheduler scheduler;
    std::atomic<int> detached_ran{0};
    constexpr int rounds = 50;
    constexpr int detached_per_round = 10;

    for (int r = 0; r < rounds; ++r) {
        for (int d = 0; d < detached_per_round; ++d) {
            scheduler.submit([&] { detached_ran.fetch_add(1, std::memory_order_relaxed); });
        }
        constexpr std::size_t batch = 2000;
        std::atomic<long long> sum{0};
        scheduler.dispatch(batch, [&](std::size_t i) { sum.fetch_add(static_cast<long long>(i), std::memory_order_relaxed); });
        const long long expected = static_cast<long long>(batch - 1) * static_cast<long long>(batch) / 2;
        ASSERT_EQ(sum.load(), expected) << "batch " << r << " completed early or double-counted";
    }

    ASSERT_TRUE(waitFor([&] { return detached_ran.load() == rounds * detached_per_round; }))
        << "detached ran " << detached_ran.load();
}

// An exception escaping a detached job is swallowed: the scheduler keeps running and later jobs
// still execute (no std::terminate, no wedged worker).
TEST(JobSchedulerTest, SubmitJobExceptionIsSwallowed) {
    JobScheduler scheduler;
    std::atomic<int> ran{0};
    scheduler.submit([] { throw std::runtime_error("detached boom"); });
    for (int i = 0; i < 10; ++i) {
        scheduler.submit([&] { ran.fetch_add(1, std::memory_order_relaxed); });
    }
    ASSERT_TRUE(waitFor([&] { return ran.load() == 10; })) << "only " << ran.load() << " ran after a throwing job";
}

// Destroying a scheduler with detached jobs still queued must not hang or crash (queued-but-unstarted
// jobs are dropped; in-flight jobs finish before join).
TEST(JobSchedulerTest, CleanShutdownWithQueuedDetachedJobs) {
    std::atomic<int> ran{0};
    {
        JobScheduler scheduler(1);  // single worker: many jobs will still be queued at teardown
        for (int i = 0; i < 1000; ++i) {
            scheduler.submit([&] {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                ran.fetch_add(1, std::memory_order_relaxed);
            });
        }
        // Let it drain briefly, then destruct while jobs almost certainly remain queued.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // No assertion on the exact count: the contract is a clean shutdown, not that every queued job
    // ran. Reaching here without hang/crash is the pass condition.
    SUCCEED();
}
