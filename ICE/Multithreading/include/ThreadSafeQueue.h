#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>

namespace ICE {
template<typename T>
class ThreadSafeQueue {
   public:
    ThreadSafeQueue() = default;
    virtual ~ThreadSafeQueue() = default;

    // Disable copying
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Push an item into the queue
    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);

        m_cond_var.notify_one();
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace(std::forward<Args>(args)...);
        m_cond_var.notify_one();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    // Pop an item from the queue (blocks until available)
    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_var.wait(lock, [this] { return !m_queue.empty() || m_stop; });
        if (m_stop && m_queue.empty())
            throw std::runtime_error("Queue stopped");
        T value = m_queue.front();
        m_queue.pop();
        return value;
    }

    // Try to pop an item (non-blocking)
    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    // Stop all waiting threads (useful for shutdown)
    void stop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;

        m_cond_var.notify_all();
    }

   private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond_var;
    bool m_stop = false;
};
}  // namespace ICE