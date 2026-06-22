#pragma once
#include <string>

class EvictionPolicy {
public:
    virtual bool on_get(const std::string& key) = 0;
    virtual void on_set(const std::string& key) = 0;
    virtual std::string evict() = 0;
    virtual void remove(const std::string& key) = 0;
    virtual ~EvictionPolicy() = default;
};
