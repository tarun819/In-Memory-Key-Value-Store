#include "lru_policy.h"

// ═════════════════════════════════════════════════════════════════════════════
// LRUPolicy — hashmap + doubly linked list
// Layout:  head <-> [MRU ... LRU] <-> tail   (both head & tail are dummies)
// ═════════════════════════════════════════════════════════════════════════════

// ─── constructor / destructor ─────────────────────────────────────────────────

LRUPolicy::LRUPolicy() {
    head = new ListNode("");  // dummy head — MRU side
    tail = new ListNode("");  // dummy tail — LRU side
    head->next = tail;
    tail->prev = head;
}

LRUPolicy::~LRUPolicy() {
    ListNode* cur = head;
    while (cur) {
        ListNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
}

// ─── private helpers ──────────────────────────────────────────────────────────

void LRUPolicy::push_front(ListNode* node) {
    // Insert right after dummy head (MRU position)
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}
 //we are using the LRUpolciy:: cause we want to tells the complier where does these functions belong thats why 
void LRUPolicy::unlink(ListNode* node) {
    // Detach node from wherever it currently sits
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = nullptr;
}

// ─── EvictionPolicy interface ─────────────────────────────────────────────────

bool LRUPolicy::on_get(const std::string& key) {
    auto it = map.find(key);
    if (it == map.end()) return false;  // key not tracked
    ListNode* node = it->second;
    unlink(node);
    push_front(node);
    return true;
}

void LRUPolicy::on_set(const std::string& key) {
    // New key — create node and insert at front (MRU position)
    ListNode* node = new ListNode(key);
    map[key] = node;
    push_front(node);
}

std::string LRUPolicy::evict() {
    // Evict from just before dummy tail (LRU position)
    ListNode* lru = tail->prev;
    std::string evicted_key = lru->key;
    unlink(lru);
    map.erase(evicted_key);
    delete lru;
    return evicted_key;
}

void LRUPolicy::remove(const std::string& key) {
    // Called on DEL or TTL expiry
    auto it = map.find(key);
    if (it == map.end()) return;
    ListNode* node = it->second;
    unlink(node);
    map.erase(it);
    delete node;
}
