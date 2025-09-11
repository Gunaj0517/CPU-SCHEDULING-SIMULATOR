#ifndef PRIORITYSCHEDULER_H
#define PRIORITYSCHEDULER_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include "Process.h"
#include "ProcessGrantInfo.h"

class PriorityScheduler {
private:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    bool preemptive;

public:
    PriorityScheduler(std::vector<Process> procs, bool is_preemptive = false) 
        : processes(procs), preemptive(is_preemptive) {}

    std::vector<ProcessGrantInfo> cpu_process() {
        if (preemptive) {
            return preemptive_priority();
        } else {
            return non_preemptive_priority();
        }
    }

private:
    // ---------------- NON-PREEMPTIVE ----------------
    std::vector<ProcessGrantInfo> non_preemptive_priority() {
        std::vector<Process> ready_queue;
        int current_time = 0;
        
        std::sort(processes.begin(), processes.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        while (!processes.empty() || !ready_queue.empty()) {
            while (!processes.empty() && processes.front().arrival_time <= current_time) {
                ready_queue.push_back(processes.front());
                processes.erase(processes.begin());
            }
            
            if (ready_queue.empty()) {
                if (!processes.empty()) {
                    current_time = processes.front().arrival_time;
                    continue;
                } else break;
            }
            
            auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Process& a, const Process& b) {
                    return a.priority < b.priority;
                });
            
            Process current_process = *highest_priority;
            ready_queue.erase(highest_priority);
            
            int first_cpu_start = current_time;
            int first_cpu_end = first_cpu_start + current_process.cpu_burst_time1;
            int io_start = first_cpu_end;
            int io_end = io_start + current_process.io_time;
            int second_cpu_start = io_end;
            int second_cpu_end = second_cpu_start + current_process.cpu_burst_time2;
            
            current_time = first_cpu_end;
            
            grantt_chart.emplace_back(
                current_process,
                first_cpu_start,
                io_start,
                second_cpu_start,
                first_cpu_end,
                io_end,
                second_cpu_end
            );
        }
        
        return grantt_chart;
    }

    // ---------------- PREEMPTIVE ----------------
    std::vector<ProcessGrantInfo> preemptive_priority() {
        std::vector<Process> ready_queue;
        std::vector<std::pair<Process, int>> running_processes;
        std::vector<std::pair<Process, std::pair<int, int>>> io_processes;
        std::vector<std::pair<Process, std::pair<int, bool>>> process_status;
        
        int current_time = 0;
        
        std::sort(processes.begin(), processes.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        while (!processes.empty() || !ready_queue.empty() || !running_processes.empty() || !io_processes.empty()) {
            while (!processes.empty() && processes.front().arrival_time <= current_time) {
                ready_queue.push_back(processes.front());
                processes.erase(processes.begin());
            }
            
            for (auto it = io_processes.begin(); it != io_processes.end();) {
                if (current_time >= it->second.first + it->first.io_time) {
                    Process p = it->first;
                    p.cpu_burst_time1 = 0;
                    ready_queue.push_back(p);
                    it = io_processes.erase(it);
                } else ++it;
            }
            
            if (running_processes.empty() && !ready_queue.empty()) {
                auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                    [](const Process& a, const Process& b) {
                        return a.priority < b.priority;
                    });
                
                Process current_process = *highest_priority;
                ready_queue.erase(highest_priority);
                
                int remaining_time = (current_process.cpu_burst_time1 > 0) ? 
                                     current_process.cpu_burst_time1 : 
                                     current_process.cpu_burst_time2;
                
                running_processes.push_back({current_process, remaining_time});
                
                bool is_first_burst = current_process.cpu_burst_time1 > 0;
                process_status.push_back({current_process, {current_time, is_first_burst}});
            }
            else if (!running_processes.empty() && !ready_queue.empty()) {
                auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                    [](const Process& a, const Process& b) {
                        return a.priority < b.priority;
                    });
                
                if (highest_priority->priority < running_processes[0].first.priority) {
                    Process current_process = running_processes[0].first;
                    int remaining_time = running_processes[0].second;
                    
                    if (current_process.cpu_burst_time1 > 0)
                        current_process.cpu_burst_time1 = remaining_time;
                    else
                        current_process.cpu_burst_time2 = remaining_time;
                    
                    ready_queue.push_back(current_process);
                    
                    for (auto it = process_status.rbegin(); it != process_status.rend(); ++it) {
                        if (it->first.pid == current_process.pid && 
                            ((it->second.second && current_process.cpu_burst_time1 > 0) || 
                             (!it->second.second && current_process.cpu_burst_time1 == 0))) {
                            
                            if (it->second.second) {
                                grantt_chart.emplace_back(
                                    current_process,
                                    it->second.first,
                                    -1,
                                    -1,
                                    current_time,
                                    -1,
                                    -1
                                );
                            } else {
                                for (auto& entry : grantt_chart) {
                                    if (entry.process.pid == current_process.pid) {
                                        entry.cpu_start_time2 = it->second.first;
                                        entry.cpu_end_time2 = current_time;
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                    
                    Process new_process = *highest_priority;
                    ready_queue.erase(highest_priority);
                    
                    int new_remaining_time = (new_process.cpu_burst_time1 > 0) ? 
                                           new_process.cpu_burst_time1 : 
                                           new_process.cpu_burst_time2;
                    
                    running_processes[0] = {new_process, new_remaining_time};
                    
                    bool is_first_burst = new_process.cpu_burst_time1 > 0;
                    process_status.push_back({new_process, {current_time, is_first_burst}});
                }
            }
            
            if (!running_processes.empty()) {
                running_processes[0].second--;
                
                if (running_processes[0].second == 0) {
                    Process completed_process = running_processes[0].first;
                    running_processes.clear();
                    
                    for (auto it = process_status.rbegin(); it != process_status.rend(); ++it) {
                        if (it->first.pid == completed_process.pid) {
                            if (it->second.second) {
                                if (completed_process.io_time > 0) {
                                    io_processes.push_back({completed_process, {current_time + 1, completed_process.io_time}});
                                    
                                    grantt_chart.emplace_back(
                                        completed_process,
                                        it->second.first,
                                        current_time + 1,
                                        -1,
                                        current_time + 1,
                                        current_time + 1 + completed_process.io_time,
                                        -1
                                    );
                                } else if (completed_process.cpu_burst_time2 > 0) {
                                    completed_process.cpu_burst_time1 = 0;
                                    ready_queue.push_back(completed_process);
                                    
                                    grantt_chart.emplace_back(
                                        completed_process,
                                        it->second.first,
                                        -1,
                                        -1,
                                        current_time + 1,
                                        -1,
                                        -1
                                    );
                                }
                            } else {
                                for (auto& entry : grantt_chart) {
                                    if (entry.process.pid == completed_process.pid) {
                                        entry.cpu_start_time2 = it->second.first;
                                        entry.cpu_end_time2 = current_time + 1;
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
            
            current_time++;
            
            if (processes.empty() && ready_queue.empty() && running_processes.empty() && io_processes.empty())
                break;
            
            if (running_processes.empty() && ready_queue.empty()) {
                int next_time = INT_MAX;
                if (!processes.empty()) next_time = std::min(next_time, processes.front().arrival_time);
                if (!io_processes.empty()) {
                    for (const auto& io : io_processes) {
                        next_time = std::min(next_time, io.second.first + io.first.io_time);
                    }
                }
                if (next_time != INT_MAX) current_time = next_time;
            }
        }
        
        return grantt_chart;
    }
};

#endif // PRIORITYSCHEDULER_H
