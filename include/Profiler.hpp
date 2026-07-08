#ifndef PROFILER_H
#define PROFILER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>

class Profiler {
public:
    static Profiler& getInstance() {
        static Profiler instance;
        return instance;
    }

    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    void start(const std::string& name) {
        auto& timer_info = active_timers[name];
        r_counter[name]++;

        if (timer_info.depth == 0) {
            c_counter[name]++;
            timer_info.start_time = std::chrono::steady_clock::now();
        }

        timer_info.depth++;
    }

    void stop(const std::string& name) {
        auto it = active_timers.find(name);
        
        if (it != active_timers.end()) {
            it->second.depth--;

            if (it->second.depth == 0) {
                auto end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - it->second.start_time).count();

                results[name] += duration;

                active_timers.erase(it);
            }
        }
    }

    void logAll() {
        std::cerr << "\n========== PERFORMANCE LOGS ==========\n";
        double total_time = 0.0;
        for(const auto& pair: results) {
            total_time = std::max(static_cast<double>(pair.second), total_time);
        }

        for (const auto& pair : results) {
            int calls = c_counter.at(pair.first);
            int rcalls = r_counter.at(pair.first);
            double r_per_c = (calls > 0) ? (double)rcalls / calls : 0.0;

            double average = (calls > 0) ? (static_cast<double>(pair.second) / calls) : 0.0;

            double time_share = (total_time > 0) ? ((pair.second / total_time) * 100.0) : 0.0;

            std::cerr << std::fixed << std::setprecision(1);

            std::cerr << "Function: " << pair.first << "\n"
                      << "* function calls  : " << calls << "\n"
                      << "* recursive calls : " << r_per_c << "\n"
                      << "* Time share   (%): " << time_share << "\n"
                      << "* Total time   (s): " << pair.second / 1e6 << "\n"
                      << "* average time(mu): " << average << "\n\n";
        }
        std::cerr << "======================================\n";

        active_timers.clear();
        results.clear();
        c_counter.clear();
        r_counter.clear();
    }

private:
    Profiler() = default; 
    ~Profiler() = default;

    struct TimerInfo {
        std::chrono::steady_clock::time_point start_time;
        int depth = 0;
    };

    std::unordered_map<std::string, TimerInfo> active_timers;
    std::unordered_map<std::string, long long> results;
    std::unordered_map<std::string, int> c_counter; // calls counter
    std::unordered_map<std::string, int> r_counter; // recursive counter
};

#endif
