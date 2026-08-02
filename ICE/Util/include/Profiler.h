#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace ICE {

// Minimal per-frame CPU profiler. Scoped timers accumulate wall-clock time per named
// section; beginFrame() snapshots the accumulated results and starts a fresh frame, so
// lastFrame() always exposes a complete, stable set of timings (in milliseconds) for a UI
// or logging to read. Single-threaded (the engine's main loop); not synchronized.
class Profiler {
   public:
    static Profiler& get() {
        static Profiler instance;
        return instance;
    }

    void beginFrame() {
        m_last = std::move(m_current);
        m_current.clear();
    }

    void addSample(const std::string& name, double ms) { m_current[name] += ms; }

    const std::unordered_map<std::string, double>& lastFrame() const { return m_last; }

   private:
    std::unordered_map<std::string, double> m_current;
    std::unordered_map<std::string, double> m_last;
};

// RAII: records the elapsed time of its scope into the Profiler under `name`.
class ScopedCPUTimer {
   public:
    explicit ScopedCPUTimer(std::string name) : m_name(std::move(name)), m_start(std::chrono::steady_clock::now()) {}
    ~ScopedCPUTimer() {
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_start).count();
        Profiler::get().addSample(m_name, ms);
    }

    ScopedCPUTimer(const ScopedCPUTimer&) = delete;
    ScopedCPUTimer& operator=(const ScopedCPUTimer&) = delete;

   private:
    std::string m_name;
    std::chrono::steady_clock::time_point m_start;
};

#define ICE_PROFILE_CONCAT_(a, b) a##b
#define ICE_PROFILE_CONCAT(a, b) ICE_PROFILE_CONCAT_(a, b)
// Times the enclosing scope under `name` (a string literal or std::string).
#define ICE_PROFILE_SCOPE(name) ICE::ScopedCPUTimer ICE_PROFILE_CONCAT(ice_scoped_timer_, __LINE__)(name)

}  // namespace ICE
