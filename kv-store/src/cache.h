#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <chrono>
#include "eviction_policy.h"

class KVCache {
private:
    int capacity;
    std::unique_ptr<EvictionPolicy> policy;
    std::unordered_map<std::string, std::string> store;            // key → value
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> ttl_map;  // key → expiry
    mutable std::shared_mutex mtx;

    // Check if a key has expired (caller must hold at least shared_lock)
    bool is_expired(const std::string& key) const;

    // Remove a key from store, ttl_map, and policy (caller must hold unique_lock)
    void evict_key(const std::string& key);

public:
    KVCache(int cap, std::unique_ptr<EvictionPolicy> p);

    // Returns the value or "NIL" if key not found / expired
    std::string get(const std::string& key);

    // Set key=value, optional TTL in seconds (-1 means no expiry)
    void set(const std::string& key, const std::string& val, int ttl_secs = -1);

    // Delete a key. Returns true if key existed, false otherwise
    bool del(const std::string& key);

    // Check if key exists (and is not expired). Returns true/false
    bool exists(const std::string& key);
};
