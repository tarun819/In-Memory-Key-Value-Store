#include "lfu_policy.h"

// ═════════════════════════════════════════════════════════════════════════════
// FreqList — doubly linked list for one frequency bucket
// Layout:  head <-> [most recent ... least recent] <-> tail
// ═════════════════════════════════════════════════════════════════════════════

FreqList::FreqList() : size(0) {
    head = new LFUNode("", 0);  // dummy head
    tail = new LFUNode("", 0);  // dummy tail
    head->next = tail;
    tail->prev = head;
}

FreqList::~FreqList() {
    delete head;
    delete tail;
    // Note: nodes themselves are owned by key_map in LFUPolicy
}

void FreqList::push_front(LFUNode* node) {
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
    size++;
}

void FreqList::unlink(LFUNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = nullptr;
    size--;
}

LFUNode* FreqList::pop_tail() {
    // Returns the LRU node in this bucket (just before dummy tail)
    if (is_empty()) return nullptr;
    LFUNode* lru = tail->prev;
    unlink(lru);
    return lru;
}

bool FreqList::is_empty() const {
    return head->next == tail;
}

// ═════════════════════════════════════════════════════════════════════════════
// LFUPolicy
// ═════════════════════════════════════════════════════════════════════════════

LFUPolicy::LFUPolicy() : min_freq(0) {}

LFUPolicy::~LFUPolicy() {
    // Delete all nodes still in key_map
    for (auto& [key, node] : key_map) {
        delete node;
    }
    // Delete all frequency bucket lists
    for (auto& [freq, list] : freq_map) {
        delete list;
    }
}

// ─── private helper ───────────────────────────────────────────────────────────

void LFUPolicy::increment_freq(LFUNode* node) {
    int old_freq = node->freq;

    // Remove from old frequency bucket
    freq_map[old_freq]->unlink(node);

    // If old bucket is now empty and it was the minimum, bump min_freq
    if (freq_map[old_freq]->is_empty() && old_freq == min_freq) {
        min_freq++;
    }

    // Move to new frequency bucket
    node->freq++;
    int new_freq = node->freq;

    if (freq_map.find(new_freq) == freq_map.end()) {
        freq_map[new_freq] = new FreqList();
    }
    freq_map[new_freq]->push_front(node);
}

// ─── EvictionPolicy interface ─────────────────────────────────────────────────

bool LFUPolicy::on_get(const std::string& key) {
    auto it = key_map.find(key);
    if (it == key_map.end()) return false;  // key not tracked
    increment_freq(it->second);
    return true;
}

void LFUPolicy::on_set(const std::string& key) {
    LFUNode* node = new LFUNode(key, 1);
    key_map[key] = node;

    if (freq_map.find(1) == freq_map.end()) {
        freq_map[1] = new FreqList();
    }
    freq_map[1]->push_front(node);

    min_freq = 1;  // new keys always start at freq 1
}

std::string LFUPolicy::evict() {
    // Pop the LRU node from the minimum frequency bucket
    FreqList* list = freq_map[min_freq];
    LFUNode* lru = list->pop_tail();

    std::string evicted_key = lru->key;
    key_map.erase(evicted_key);
    delete lru;

    return evicted_key;
}

void LFUPolicy::remove(const std::string& key) {
    auto it = key_map.find(key);
    if (it == key_map.end()) return;

    LFUNode* node = it->second;
    freq_map[node->freq]->unlink(node);
    key_map.erase(it);
    delete node;
}
