#ifndef PROCESS_H
#define PROCESS_H

#include <string>

struct Process {
    std::string pid;                 // Process ID
    int process_id;                  // Numeric Process ID (for compatibility)
    int arrival_time;                // Arrival time
    int cpu_burst_time1;             // First CPU burst
    int io_time;                     // I/O time
    int cpu_burst_time2;             // Second CPU burst
    int priority;                    // Priority (for priority scheduling)

    Process() : pid(""), process_id(0), arrival_time(0), cpu_burst_time1(0), io_time(0), cpu_burst_time2(0), priority(0) {}
    
    Process(std::string id, int at, int cpu1, int io, int cpu2, int prio = 0)
        : pid(id), process_id(0), arrival_time(at),
          cpu_burst_time1(cpu1), io_time(io), cpu_burst_time2(cpu2), priority(prio) {
        // Extract numeric ID from string ID (assuming format like "P1")
        if (id.length() > 1 && id[0] == 'P') {
            try {
                process_id = std::stoi(id.substr(1));
            } catch (...) {
                process_id = 0;
            }
        }
    }
};

#endif