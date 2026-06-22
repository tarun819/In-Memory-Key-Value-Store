// Concurrency stress test for KVCache
// Spawns multiple threads hammering the same cache simultaneously
// If locks are broken → crash / hang / corrupted output

#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include "cache.h"
#include "lru_policy.h"
#include "lfu_policy.h"

void worker(KVCache& cache, int id) {
    for (int i = 0; i < 10000; i++) {
        // All threads use overlapping keys (key0 to key49) → forces contention
        std::string key = "key" + std::to_string(i % 50);
        std::string val = "thread" + std::to_string(id) + "_val" + std::to_string(i);

        cache.set(key, val);
        cache.get(key);
        cache.exists(key);

        // Every 3rd iteration, delete the key (while other threads might be reading it)
        if (i % 3 == 0) {
            cache.del(key);
        }
    }
}

int main() {
    int num_threads = 4;

    // ── Test 1: LRU ──────────────────────────────────────────────
    std::cout << "=== Test 1: LRU policy, capacity=100, " << num_threads << " threads ===\n";
    {
        KVCache cache(100, std::make_unique<LRUPolicy>());

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back(worker, std::ref(cache), t);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "[PASS] LRU — all threads finished, no crash!\n\n";
    }

    // ── Test 2: LFU ──────────────────────────────────────────────
    std::cout << "=== Test 2: LFU policy, capacity=100, " << num_threads << " threads ===\n";
    {
        KVCache cache(100, std::make_unique<LFUPolicy>());

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back(worker, std::ref(cache), t);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "[PASS] LFU — all threads finished, no crash!\n\n";
    }

    // ── Test 3: TTL expiry under contention ──────────────────────
    std::cout << "=== Test 3: TTL expiry under contention ===\n";
    {
        KVCache cache(100, std::make_unique<LRUPolicy>());

        auto ttl_worker = [&cache](int id) {
            for (int i = 0; i < 5000; i++) {
                std::string key = "ttlkey" + std::to_string(i % 20);
                // Set with 1-second TTL
                cache.set(key, "val" + std::to_string(id), 1);
                cache.get(key);
                cache.exists(key);
            }
        };

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back(ttl_worker, t);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "[PASS] TTL — all threads finished, no crash!\n\n";
    }

    std::cout << "══════════════════════════════════════════\n";
    std::cout << "  ALL CONCURRENCY TESTS PASSED!\n";
    std::cout << "══════════════════════════════════════════\n";
    return 0;
}
