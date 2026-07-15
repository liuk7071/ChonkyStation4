#pragma once

#include <Common.hpp>
#include <unordered_map>
#include <chrono>


namespace Profiler {

struct Entry {
    u64 calls = 0;
    u64 total_us = 0;
};

inline std::unordered_map<std::string, Entry> entries;

inline void add(const std::string& name, u64 us) {
    auto& entry = entries[name];
    entry.calls++;
    entry.total_us += us;
}

inline void printAndReset() {
    printf("------ Profiler ------\n");
    for (auto& [name, entry] : entries) {
        printf("%-40s %8llu us (%llu calls, avg %.2f us per call)\n", name.c_str(), entry.total_us, entry.calls, (float)entry.total_us / entry.calls);
    }
    entries.clear();
}

class Scope {
public:
    Scope(const char* name) : name(name) {
        start = std::chrono::high_resolution_clock::now();
    }

    ~Scope() {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        Profiler::add(name, elapsed);
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start;
};

class Timer {
public:
    Timer(const char* name) : name(name) {}

    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start_time).count();
        Profiler::add(name, elapsed);
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time;
};

}   // End namespace Profiler