#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace ICE {

// A work-stealing thread pool for data-parallel, fork-join style work (e.g. per-frame render
// culling). Each worker owns a deque: it runs its own tasks LIFO and steals from other workers'
// deques FIFO when idle. dispatch()/parallelRanges() split work into tasks and block until all
// complete -- and the calling thread participates as an extra worker, so it is never left idle and
// completion is guaranteed even if a worker misses a wakeup.
//
// The deques are guarded by per-worker mutexes (correct rather than lock-free); for coarse chunked
// jobs contention is negligible. Not re-entrant: do not call dispatch() from inside a task running
// on this scheduler.
class JobScheduler {
   public:
    // worker_count == 0 -> hardware_concurrency() - 1 (leaving a core for the calling thread),
    // clamped to at least 1.
    explicit JobScheduler(std::size_t worker_count = 0) {
        if (worker_count == 0) {
            unsigned hc = std::thread::hardware_concurrency();
            worker_count = hc > 1 ? static_cast<std::size_t>(hc - 1) : 1;
        }
        m_workers.reserve(worker_count);
        for (std::size_t i = 0; i < worker_count; ++i) {
            m_workers.push_back(std::make_unique<Worker>());
        }
        m_threads.reserve(worker_count);
        for (std::size_t i = 0; i < worker_count; ++i) {
            m_threads.emplace_back([this, i] { workerLoop(i); });
        }
    }

    ~JobScheduler() {
        m_stop.store(true, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lock(m_wake_mutex);
            m_wake.notify_all();
        }
        for (auto& t : m_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    JobScheduler(const JobScheduler&) = delete;
    JobScheduler& operator=(const JobScheduler&) = delete;

    std::size_t workerCount() const { return m_workers.size(); }

    // Run job(i) for i in [0, count), blocking until all complete. Small counts (or an empty pool)
    // run inline on the calling thread. An exception thrown by any job is re-thrown here after all
    // tasks finish.
    void dispatch(std::size_t count, const std::function<void(std::size_t)>& job) {
        if (count == 0) {
            return;
        }
        if (m_workers.empty() || count == 1) {
            for (std::size_t i = 0; i < count; ++i) {
                job(i);
            }
            return;
        }

        std::mutex ex_mutex;
        std::exception_ptr captured_ex;
        m_pending.store(count, std::memory_order_relaxed);

        for (std::size_t i = 0; i < count; ++i) {
            auto& worker = *m_workers[i % m_workers.size()];
            std::lock_guard<std::mutex> lock(worker.mutex);
            worker.queue.push_back([this, &job, i, &ex_mutex, &captured_ex] {
                try {
                    job(i);
                } catch (...) {
                    std::lock_guard<std::mutex> lk(ex_mutex);
                    if (!captured_ex) {
                        captured_ex = std::current_exception();
                    }
                }
                // Always decrement so a throwing job can never wedge the wait below.
                if (m_pending.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    std::lock_guard<std::mutex> lk(m_wake_mutex);
                    m_wake.notify_all();
                }
            });
        }

        {
            std::lock_guard<std::mutex> lock(m_wake_mutex);
            m_wake.notify_all();
        }

        // The calling thread helps drain tasks until the batch is done (guarantees progress even
        // if every worker missed the wakeup).
        std::function<void()> task;
        while (m_pending.load(std::memory_order_acquire) > 0) {
            if (stealAny(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }

        if (captured_ex) {
            std::rethrow_exception(captured_ex);
        }
    }

    // Split [0, count) into contiguous ranges (~workerCount * chunks_per_worker of them) and call
    // fn(begin, end) for each in parallel, blocking until all complete.
    void parallelRanges(std::size_t count, const std::function<void(std::size_t begin, std::size_t end)>& fn,
                        std::size_t chunks_per_worker = 4) {
        if (count == 0) {
            return;
        }
        std::size_t target = m_workers.empty() ? 1 : m_workers.size() * (chunks_per_worker == 0 ? 1 : chunks_per_worker);
        std::size_t nchunks = target < count ? target : count;
        const std::size_t base = count / nchunks;
        const std::size_t rem = count % nchunks;
        dispatch(nchunks, [&](std::size_t c) {
            std::size_t begin = c * base + (c < rem ? c : rem);
            std::size_t end = begin + base + (c < rem ? 1 : 0);
            fn(begin, end);
        });
    }

   private:
    struct Worker {
        std::deque<std::function<void()>> queue;
        std::mutex mutex;
    };

    void workerLoop(std::size_t index) {
        while (!m_stop.load(std::memory_order_acquire)) {
            std::function<void()> task;
            if (getTask(index, task)) {
                task();
                continue;
            }
            // No work: sleep until a batch is dispatched or we are stopping. The short timeout is
            // a backstop against a missed wakeup (the calling thread is the real guarantee).
            std::unique_lock<std::mutex> lock(m_wake_mutex);
            m_wake.wait_for(lock, std::chrono::milliseconds(2), [this] {
                return m_stop.load(std::memory_order_acquire) || m_pending.load(std::memory_order_acquire) > 0;
            });
        }
    }

    // A worker takes from its own deque (LIFO, cache-friendly) then steals from others (FIFO).
    bool getTask(std::size_t index, std::function<void()>& out) {
        {
            auto& own = *m_workers[index];
            std::lock_guard<std::mutex> lock(own.mutex);
            if (!own.queue.empty()) {
                out = std::move(own.queue.back());
                own.queue.pop_back();
                return true;
            }
        }
        return steal(index, out);
    }

    bool steal(std::size_t skip_index, std::function<void()>& out) {
        for (std::size_t n = 0; n < m_workers.size(); ++n) {
            if (n == skip_index) {
                continue;
            }
            auto& w = *m_workers[n];
            std::lock_guard<std::mutex> lock(w.mutex);
            if (!w.queue.empty()) {
                out = std::move(w.queue.front());
                w.queue.pop_front();
                return true;
            }
        }
        return false;
    }

    // For the calling thread, which owns no deque: steal from any worker.
    bool stealAny(std::function<void()>& out) {
        for (std::size_t n = 0; n < m_workers.size(); ++n) {
            auto& w = *m_workers[n];
            std::lock_guard<std::mutex> lock(w.mutex);
            if (!w.queue.empty()) {
                out = std::move(w.queue.front());
                w.queue.pop_front();
                return true;
            }
        }
        return false;
    }

    std::vector<std::unique_ptr<Worker>> m_workers;
    std::vector<std::thread> m_threads;
    std::mutex m_wake_mutex;
    std::condition_variable m_wake;
    std::atomic<bool> m_stop{false};
    std::atomic<std::size_t> m_pending{0};
};
}  // namespace ICE
