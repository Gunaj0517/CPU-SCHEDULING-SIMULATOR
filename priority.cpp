#include <iostream>
#include <vector>
#include <algorithm>
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
    std::vector<ProcessGrantInfo> non_preemptive_priority() {
        std::vector<Process> ready_queue;
        int current_time = 0;
        
        // Sort processes by arrival time initially
        std::sort(processes.begin(), processes.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        while (!processes.empty() || !ready_queue.empty()) {
            // Add newly arrived processes to ready queue
            while (!processes.empty() && processes.front().arrival_time <= current_time) {
                ready_queue.push_back(processes.front());
                processes.erase(processes.begin());
            }
            
            if (ready_queue.empty()) {
                // Jump to next process arrival time if no process in ready queue
                if (!processes.empty()) {
                    current_time = processes.front().arrival_time;
                    continue;
                } else {
                    break;
                }
            }
            
            // Find process with highest priority (lowest number = highest priority)
            auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Process& a, const Process& b) {
                    return a.priority < b.priority;
                });
            
            Process current_process = *highest_priority;
            ready_queue.erase(highest_priority);
            
            // Execute first CPU burst
            int first_cpu_start = current_time;
            int first_cpu_end = first_cpu_start + current_process.cpu_burst_time1;
            
            // Execute IO
            int io_start = first_cpu_end;
            int io_end = io_start + current_process.io_time;
            
            // Execute second CPU burst
            int second_cpu_start = io_end;
            int second_cpu_end = second_cpu_start + current_process.cpu_burst_time2;
            
            // Update current time
            current_time = first_cpu_end;
            
            // Add to Gantt chart
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
    
    std::vector<ProcessGrantInfo> preemptive_priority() {
        std::vector<Process> ready_queue;
        std::vector<std::pair<Process, int>> running_processes; // Process and remaining time
        std::vector<std::pair<Process, std::pair<int, int>>> io_processes; // Process, start time, remaining time
        std::vector<std::pair<Process, std::pair<int, bool>>> process_status; // Process, start time, is_first_burst
        
        int current_time = 0;
        
        // Sort processes by arrival time initially
        std::sort(processes.begin(), processes.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        while (!processes.empty() || !ready_queue.empty() || !running_processes.empty() || !io_processes.empty()) {
            // Add newly arrived processes to ready queue
            while (!processes.empty() && processes.front().arrival_time <= current_time) {
                ready_queue.push_back(processes.front());
                processes.erase(processes.begin());
            }
            
            // Check if IO processes are done
            for (auto it = io_processes.begin(); it != io_processes.end();) {
                if (current_time >= it->second.first + it->first.io_time) {
                    // IO finished, add to ready queue for second CPU burst
                    Process p = it->first;
                    p.cpu_burst_time1 = 0; // First burst is done
                    ready_queue.push_back(p);
                    it = io_processes.erase(it);
                } else {
                    ++it;
                }
            }
            
            // If no process is running, get highest priority process from ready queue
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
                
                // Record start time and which burst
                bool is_first_burst = current_process.cpu_burst_time1 > 0;
                process_status.push_back({current_process, {current_time, is_first_burst}});
            }
            // Check if a higher priority process arrived
            else if (!running_processes.empty() && !ready_queue.empty()) {
                auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                    [](const Process& a, const Process& b) {
                        return a.priority < b.priority;
                    });
                
                if (highest_priority->priority < running_processes[0].first.priority) {
                    // Preempt current process
                    Process current_process = running_processes[0].first;
                    int remaining_time = running_processes[0].second;
                    
                    // Update current process's remaining time
                    if (current_process.cpu_burst_time1 > 0) {
                        current_process.cpu_burst_time1 = remaining_time;
                    } else {
                        current_process.cpu_burst_time2 = remaining_time;
                    }
                    
                    // Add back to ready queue
                    ready_queue.push_back(current_process);
                    
                    // Record end time for current process
                    for (auto it = process_status.rbegin(); it != process_status.rend(); ++it) {
                        if (it->first.pid == current_process.pid && 
                            ((it->second.second && current_process.cpu_burst_time1 > 0) || 
                             (!it->second.second && current_process.cpu_burst_time1 == 0))) {
                            
                            // Add to Gantt chart
                            if (it->second.second) { // First burst
                                grantt_chart.emplace_back(
                                    current_process,
                                    it->second.first, // Start time
                                    -1, // IO start (will be filled later)
                                    -1, // Second CPU start (will be filled later)
                                    current_time, // End time of first burst
                                    -1, // IO end (will be filled later)
                                    -1  // Second CPU end (will be filled later)
                                );
                            } else { // Second burst
                                // Find the corresponding first burst entry
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
                    
                    // Start new process
                    Process new_process = *highest_priority;
                    ready_queue.erase(highest_priority);
                    
                    int new_remaining_time = (new_process.cpu_burst_time1 > 0) ? 
                                           new_process.cpu_burst_time1 : 
                                           new_process.cpu_burst_time2;
                    
                    running_processes[0] = {new_process, new_remaining_time};
                    
                    // Record start time and which burst
                    bool is_first_burst = new_process.cpu_burst_time1 > 0;
                    process_status.push_back({new_process, {current_time, is_first_burst}});
                }
            }
            
            // Execute current process for 1 time unit
            if (!running_processes.empty()) {
                running_processes[0].second--;
                
                // Check if current burst is complete
                if (running_processes[0].second == 0) {
                    Process completed_process = running_processes[0].first;
                    running_processes.clear();
                    
                    // Record end time
                    for (auto it = process_status.rbegin(); it != process_status.rend(); ++it) {
                        if (it->first.pid == completed_process.pid) {
                            if (it->second.second) { // First burst completed
                                // Add to IO queue
                                if (completed_process.io_time > 0) {
                                    io_processes.push_back({completed_process, {current_time + 1, completed_process.io_time}});
                                    
                                    // Add to Gantt chart
                                    grantt_chart.emplace_back(
                                        completed_process,
                                        it->second.first, // Start time
                                        current_time + 1, // IO start
                                        -1, // Second CPU start (will be filled later)
                                        current_time + 1, // End time of first burst
                                        current_time + 1 + completed_process.io_time, // IO end
                                        -1  // Second CPU end (will be filled later)
                                    );
                                } else if (completed_process.cpu_burst_time2 > 0) {
                                    // No IO but has second burst, add directly to ready queue
                                    completed_process.cpu_burst_time1 = 0;
                                    ready_queue.push_back(completed_process);
                                    
                                    // Add partial entry to Gantt chart
                                    grantt_chart.emplace_back(
                                        completed_process,
                                        it->second.first, // Start time
                                        -1, // No IO
                                        -1, // Second CPU start (will be filled later)
                                        current_time + 1, // End time of first burst
                                        -1, // No IO
                                        -1  // Second CPU end (will be filled later)
                                    );
                                }
                            } else { // Second burst completed
                                // Find the corresponding first burst entry
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
            
            // If nothing left to do, break
            if (processes.empty() && ready_queue.empty() && running_processes.empty() && io_processes.empty()) {
                break;
            }
            
            // If no process is running and ready queue is empty, jump to next event
            if (running_processes.empty() && ready_queue.empty()) {
                int next_time = INT_MAX;
                
                if (!processes.empty()) {
                    next_time = std::min(next_time, processes.front().arrival_time);
                }
                
                if (!io_processes.empty()) {
                    for (const auto& io : io_processes) {
                        next_time = std::min(next_time, io.second.first + io.first.io_time);
                    }
                }
                
                if (next_time != INT_MAX) {
                    current_time = next_time;
                }
            }
        }
        
        return grantt_chart;
    }
};