#include <gtest/gtest.h>

#include <atomic>
#include <numeric>
#include <vector>

#include "JobScheduler.h"

using namespace ICE;

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
