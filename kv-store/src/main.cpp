// Phase 2 interactive test driver
// Tests KVCache with eviction, TTL, and DEL
// Will be replaced with TCP server in Phase 3

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include "cache.h"
#include "lru_policy.h"
#include "lfu_policy.h"

void print_menu() {
    std::cout << "\nCommands:\n"
              << "  SET <key> <value>          - store key=value\n"
              << "  SET <key> <value> EX <sec> - store with TTL\n"
              << "  GET <key>                  - retrieve value\n"
              << "  DEL <key>                  - delete key\n"
              << "  EXISTS <key>               - check if key exists\n"
              << "  QUIT                       - exit\n"
              << "> ";
}

int main() {
    // Select policy
    std::cout << "Select eviction policy:\n"
              << "  1 - LRU\n"
              << "  2 - LFU\n"
              << "Choice: ";

    int choice;
    std::cin >> choice;

    // Select capacity
    std::cout << "Enter cache capacity: ";
    int cap;
    std::cin >> cap;
    std::cin.ignore();  // consume leftover newline

    std::unique_ptr<EvictionPolicy> policy;
    if (choice == 1) {
        policy = std::make_unique<LRUPolicy>();
        std::cout << "[LRU policy, capacity=" << cap << "]\n";
    } else if (choice == 2) {
        policy = std::make_unique<LFUPolicy>();
        std::cout << "[LFU policy, capacity=" << cap << "]\n";
    } else {
        std::cout << "Invalid choice.\n";
        return 1;
    }

    KVCache cache(cap, std::move(policy));

    std::string line;
    print_menu();
   // end of file -> ctrl +d then this while loopwilnot run
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            print_menu();
            continue;
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "SET" || cmd == "set") {
            std::string key, val;
            iss >> key >> val;

            if (key.empty() || val.empty()) {
                std::cout << "  Usage: SET <key> <value> [EX <seconds>]\n";
                print_menu();
                continue;
            }

            // Check for optional EX <seconds>
            int ttl = -1;
            std::string ex;
            if (iss >> ex) {
                if (ex == "EX" || ex == "ex") {
                    iss >> ttl;
                }
            }

            cache.set(key, val, ttl);
            std::cout << "  OK\n";
        }
        else if (cmd == "GET" || cmd == "get") {
            std::string key;
            iss >> key;
            std::string result = cache.get(key);
            std::cout << "  " << result << "\n";
        }
        else if (cmd == "DEL" || cmd == "del") {
            std::string key;
            iss >> key;
            bool deleted = cache.del(key);
            std::cout << "  " << (deleted ? "1" : "0") << "\n";
        }
        else if (cmd == "EXISTS" || cmd == "exists") {
            std::string key;
            iss >> key;
            bool found = cache.exists(key);
            std::cout << "  " << (found ? "1" : "0") << "\n";
        }
        else if (cmd == "QUIT" || cmd == "quit") {
            break;
        }
        else {
            std::cout << "  ERROR unknown command\n";
        }

        print_menu();
    }

    std::cout << "Bye.\n";
    return 0;
}
