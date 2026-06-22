CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -O2

SRC = src/main.cpp src/server.cpp src/cache.cpp src/lru_policy.cpp src/lfu_policy.cpp
TARGET = kv-store

# Windows needs Winsock2
ifeq ($(OS),Windows_NT)
  LDFLAGS = -lws2_32
else
  LDFLAGS =
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Phase 2 standalone test (no server, no sockets)
phase2_test:
	$(CXX) $(CXXFLAGS) -o phase2_test src/phase2_main.cpp src/cache.cpp src/lru_policy.cpp src/lfu_policy.cpp

clean:
	rm -f $(TARGET) phase2_test *.exe

.PHONY: all clean phase2_test
