//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"

#include <JobScheduler.h>
#include <Logger.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace ICE {

// One staged async result on its way from a worker to the main-thread pump().
struct StageResult {
    AssetUID uid = NO_ASSET_ID;
    std::shared_ptr<void> staged;  // opaque payload produced by the stage function
    bool ok = false;               // false if staging threw or returned null
};

// Shared worker<->main state for async imports. Held by shared_ptr so a worker that is still running
// when the bank is destroyed pushes into a state that outlives the bank, rather than dereferencing a
// dangling AssetBank -- this is why the stage function must never touch the bank.
struct AssetBankAsyncState {
    std::mutex mutex;
    std::vector<StageResult> completed;      // written by workers, drained by pump() (main thread)
    std::atomic<std::size_t> inflight{0};    // reserved but not yet finalized
};

// Loaders are registered by the composition layer (io::registerDefaultLoaders),
// not here, so that the `assets` module does not depend on `io`.
AssetBank::AssetBank() : m_async(std::make_shared<AssetBankAsyncState>()) {}

AssetBank::~AssetBank() {
    // Finalize outstanding async loads before members are torn down. Safe: the bank co-owns the
    // scheduler (kept alive until this returns), and workers push to the shared m_async state.
    if (m_scheduler && m_async && m_async->inflight.load(std::memory_order_acquire) > 0) {
        flushAsync();
    }
}

bool AssetBank::nameInUse(const AssetPath &name) {
    return !(nameMapping.find(name) == nameMapping.end());
}

void AssetBank::setScheduler(const std::shared_ptr<JobScheduler> &scheduler) {
    if (scheduler == m_scheduler) {
        return;
    }
    // Drain under the current scheduler before swapping it out, so no in-flight load is orphaned
    // (its worker would otherwise be dropped when the old scheduler is released).
    if (m_scheduler && m_async->inflight.load(std::memory_order_acquire) > 0) {
        flushAsync();
    }
    m_scheduler = scheduler;
}

AssetUID AssetBank::requestAssetImpl(const AssetPath &path, StageFn stage, CommitFn commit) {
    // De-dup: an already-requested/loaded name returns its existing UID (no reload).
    if (auto it = nameMapping.find(path); it != nameMapping.end()) {
        return it->second;
    }

    AssetUID uid = nextUID++;
    AssetBankEntry entry{path, nullptr};
    entry.state = AssetState::Loading;
    resources.emplace(uid, std::move(entry));
    nameMapping.emplace(path, uid);
    m_pending_commits.emplace(uid, std::move(commit));
    m_async->inflight.fetch_add(1, std::memory_order_relaxed);

    // Capture the shared async state (not `this`): the worker pushes its result there and never
    // touches the bank, so it is safe even if the bank is destroyed while the job runs.
    auto async = m_async;
    auto run_stage = [async, uid, stage = std::move(stage)]() {
        StageResult result;
        result.uid = uid;
        try {
            result.staged = stage();
            result.ok = (result.staged != nullptr);
        } catch (...) {
            result.ok = false;  // logged on the main thread in pump()
        }
        std::lock_guard<std::mutex> lock(async->mutex);
        async->completed.push_back(std::move(result));
    };

    if (m_scheduler) {
        m_scheduler->submit(std::move(run_stage));
    } else {
        // No scheduler: stage inline now. The result is still finalized by the next pump(), so the
        // Loading -> Ready transition and ordering are identical to the async path.
        run_stage();
    }
    return uid;
}

void AssetBank::pump() {
    std::vector<StageResult> ready;
    {
        std::lock_guard<std::mutex> lock(m_async->mutex);
        ready.swap(m_async->completed);
    }
    for (auto &result : ready) {
        auto commit_it = m_pending_commits.find(result.uid);
        if (commit_it == m_pending_commits.end()) {
            // Reservation was removed before finalization; drop the result and balance the count.
            m_async->inflight.fetch_sub(1, std::memory_order_acq_rel);
            continue;
        }

        std::shared_ptr<Asset> asset;
        if (result.ok) {
            try {
                asset = commit_it->second(result.staged);  // may add sub-assets (models) on the main thread
            } catch (...) {
                asset = nullptr;
            }
        }

        if (auto res_it = resources.find(result.uid); res_it != resources.end()) {
            if (asset) {
                res_it->second.asset = asset;
                res_it->second.state = AssetState::Ready;
            } else {
                res_it->second.state = AssetState::Failed;
                Logger::Log(Logger::ERROR, "Assets", "Async load failed for '%s'", res_it->second.path.toString().c_str());
            }
        }
        m_pending_commits.erase(commit_it);
        m_async->inflight.fetch_sub(1, std::memory_order_acq_rel);
    }
}

void AssetBank::flushAsync() {
    // Pump until every reserved load has been finalized. The staging scheduler is kept alive by the
    // bank's co-ownership, so queued jobs are guaranteed to run and this terminates.
    while (m_async->inflight.load(std::memory_order_acquire) > 0) {
        pump();
        std::this_thread::yield();
    }
}

std::size_t AssetBank::inFlight() const {
    return m_async ? m_async->inflight.load(std::memory_order_acquire) : 0;
}

AssetState AssetBank::getState(AssetUID uid) const {
    auto it = resources.find(uid);
    return it != resources.end() ? it->second.state : AssetState::Failed;
}
}  // namespace ICE
