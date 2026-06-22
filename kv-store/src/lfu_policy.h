#pragma once
#include "eviction_policy.h"
#include <string>
#include <unordered_map>

// ─── Node for the doubly linked list inside each frequency bucket ─────────────

struct LFUNode {
    std::string key;
    int freq;
    LFUNode* prev;
    LFUNode* next;

    LFUNode(const std::string& k, int f)
        : key(k), freq(f), prev(nullptr), next(nullptr) {}
};

// ─── A doubly linked list representing one frequency bucket ──────────────────
// Head = most recently used at this frequency (insert here)
// Tail = least recently used at this frequency (evict from here)

struct FreqList {
    LFUNode* head;  // dummy head
    LFUNode* tail;  // dummy tail
    int size;

    FreqList();
    ~FreqList();

    void push_front(LFUNode* node);
    void unlink(LFUNode* node);
    LFUNode* pop_tail();   // removes and returns the LRU node at this frequency
    
    bool is_empty() const;  //here const means that this function will not modify the object
};

// ─── LFU Policy class ─────────────────────────────────────────────────────────

class LFUPolicy : public EvictionPolicy {
private:
    std::unordered_map<std::string, LFUNode*> key_map;   // key → node*
    std::unordered_map<int, FreqList*> freq_map;          // freq → bucket list
    int min_freq;

    void increment_freq(LFUNode* node);   // moves node to freq+1 bucket

public:
    LFUPolicy();
    ~LFUPolicy();

    bool on_get(const std::string& key) override;
    void on_set(const std::string& key) override;
    std::string evict() override;
    void remove(const std::string& key) override;
};
