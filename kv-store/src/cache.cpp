#include "cache.h"
#include <iostream>
#include <mutex>

// ═════════════════════════════════════════════════════════════════════════════
// KVCache — In-memory key-value store with TTL and pluggable eviction
// ═════════════════════════════════════════════════════════════════════════════

KVCache::KVCache(int cap, std::unique_ptr<EvictionPolicy> p)
    : capacity(cap), policy(std::move(p)) {} 

// ─── private helpers ──────────────────────────────────────────────────────────

bool KVCache::is_expired(const std::string& key) const {
    auto it = ttl_map.find(key);
    if (it == ttl_map.end()) return false;  // no TTL set → never expires
    return std::chrono::steady_clock::now() > it->second;
}

void KVCache::evict_key(const std::string& key) {
    store.erase(key);
    ttl_map.erase(key);
    policy->remove(key);
}

// ─── public interface ─────────────────────────────────────────────────────────

std::string KVCache::get(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mtx);

    auto it = store.find(key);
    if (it == store.end()) return "NIL";

    // Lazy TTL expiry — if expired, delete it now
    if (is_expired(key)) {
        evict_key(key);
        return "NIL";
    }

    // Key exists and not expired — update policy and return value
    policy->on_get(key);
    return "VALUE " + it->second;
}

void KVCache::set(const std::string& key, const std::string& val, int ttl_secs) {
    std::unique_lock<std::shared_mutex> write_lock(mtx);

    // If key already exists, update it in place
    auto it = store.find(key);
    if (it != store.end()) {
        it->second = val;
        policy->on_get(key);  // refresh position in eviction order

        // Update or remove TTL
        if (ttl_secs > 0) {
            ttl_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_secs);
        } else {
            ttl_map.erase(key);
        }
        return;
    }

    // New key — check if we need to evict
    if ((int)store.size() >= capacity) {
        std::string evicted = policy->evict();
        store.erase(evicted);
        ttl_map.erase(evicted);
        std::cout << "  [EVICTED] \"" << evicted << "\"\n";
    }

    // Insert new key
    store[key] = val;
    policy->on_set(key);

    // Set TTL if requested
    if (ttl_secs > 0) {
        ttl_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_secs);
    }
}

bool KVCache::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> write_lock(mtx);

    auto it = store.find(key);
    if (it == store.end()) return false;

    evict_key(key);
    return true;
}

bool KVCache::exists(const std::string& key) {
    // First try with shared_lock
    {
        std::shared_lock<std::shared_mutex> read_lock(mtx);

        auto it = store.find(key);
        if (it == store.end()) return false;

        if (!is_expired(key)) return true;
        // Expired — fall through to delete
    }

    // Acquire exclusive lock to lazy-delete expired key
    {
        std::unique_lock<std::shared_mutex> write_lock(mtx);

        if (store.find(key) != store.end() && is_expired(key)) {
            evict_key(key);
        }
    }

    return false;
}
