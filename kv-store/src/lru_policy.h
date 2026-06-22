#pragma once
#include "eviction_policy.h"
#include <string>
#include <unordered_map>

struct ListNode {
    std::string key;
    ListNode* prev;
    ListNode* next;

    ListNode(const std::string& k)
        : key(k), prev(nullptr), next(nullptr) {}
};

class LRUPolicy : public EvictionPolicy {
private:
    std::unordered_map<std::string, ListNode*> map;
    ListNode* head;  // dummy head — MRU side
    ListNode* tail;  // dummy tail — LRU side

    void push_front(ListNode* node);
    void unlink(ListNode* node);

public:
    LRUPolicy();
    ~LRUPolicy();
      // we use override cause it tells complier that these functions are from parent class
    bool on_get(const std::string& key) override;  
    void on_set(const std::string& key) override;
    std::string evict() override;
    void remove(const std::string& key) override;
};