#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include "Process.h"
#include "ProcessGrantInfo.h"

class MLFQ {
public:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    int current_cpu_time = 0;
    std::string ClassName = "MLFQ";

    MLFQ(const std::vector<Process>& procs) : processes(procs) {}

    std::unordered_map<int, int> get_arrival_times() {
        std::unordered_map<int, int> ret;
        for (const auto& p : processes) ret[p.process_id] = p.arrival_time;
        return ret;
    }

    std::vector<ProcessGrantInfo> cpu_process() {
        int first_time_quantum = 8;
        int sec_time_quantum = 16;
        
        // First run the two-level queue with different time quantums
        two_queue_with_diff_quantum(first_time_quantum, sec_time_quantum);
        
        // Then run the FCFS queue for any remaining processes
        fcfs_queue();
        
        return grantt_chart;
    }

private:
    void two_queue_with_diff_quantum(int first_time_quantum, int second_time_quantum) {
        std::vector<int> done_first_time_quantum;
        bool is_sec_burst_allowed = false;
        int processes_count = (int)processes.size();
        int prev_cpu_time = -1;
        bool started = false;
        std::vector<std::pair<Process, int>> ready_processes_queue;
        auto processes_next_ready_queue = get_arrival_times();
        int cycle = 1;
        int total_counter = 0;
        std::vector<Process> processes_copy = processes;
        
        // Sort processes by arrival time
        std::sort(processes_copy.begin(), processes_copy.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        while (true) {
            int time_quantum = (cycle == 1) ? first_time_quantum : second_time_quantum;

            if (!started) {
                if (processes_copy.empty()) break;
                Process first_process = processes_copy.front();
                processes_copy.erase(processes_copy.begin());
                ready_processes_queue.emplace_back(first_process, 0);
                for (auto it = processes_copy.begin(); it != processes_copy.end();) {
                    if (it->arrival_time == first_process.arrival_time) {
                        ready_processes_queue.emplace_back(*it, 0);
                        it = processes_copy.erase(it);
                    } else ++it;
                }
                started = true;
            }

            if (ready_processes_queue.empty() && !is_sec_burst_allowed) {
                current_cpu_time += 1;
                continue;
            } else if (ready_processes_queue.empty() && is_sec_burst_allowed) {
                if (processes_copy.empty()) break;
                ready_processes_queue.emplace_back(processes_copy.front(), 0);
                processes_copy.erase(processes_copy.begin());
            }

            auto& front_pair = ready_processes_queue.front();
            Process& current_process = front_pair.first;
            int& sub_count = front_pair.second;

            int pre_current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;

            // First CPU burst
            if (sub_count == 0 && current_process.cpu_burst_time1 > 0) {
                ProcessGrantInfo info(current_process, current_cpu_time, 0, 0, 0, 0, 0);
                info.cpu_start_time1 = current_cpu_time;
                
                int cpu_time = std::min(time_quantum, current_process.cpu_burst_time1);
                current_process.cpu_burst_time1 -= cpu_time;
                current_cpu_time += cpu_time;
                info.cpu_end_time1 = current_cpu_time;
                
                // If first CPU burst is complete, start I/O
                if (current_process.cpu_burst_time1 == 0 && current_process.io_time > 0) {
                    info.io_start_time = current_cpu_time;
                    int io_time = current_process.io_time;
                    current_process.io_time = 0;
                    info.io_end_time = current_cpu_time + io_time;
                    sub_count = 1; // Mark as ready for second CPU burst
                    
                    // Add to done first time quantum list
                    done_first_time_quantum.push_back(current_process.process_id);
                }
                
                grantt_chart.push_back(info);
            }
            // Second CPU burst
            else if (sub_count == 1 && current_process.cpu_burst_time2 > 0) {
                ProcessGrantInfo info(current_process, 0, 0, current_cpu_time, 0, 0, 0);
                info.cpu_start_time2 = current_cpu_time;
                
                int cpu_time = std::min(time_quantum, current_process.cpu_burst_time2);
                current_process.cpu_burst_time2 -= cpu_time;
                current_cpu_time += cpu_time;
                info.cpu_end_time2 = current_cpu_time;
                
                grantt_chart.push_back(info);
            }

            // Update the front entry's process and sub_count
            ready_processes_queue.front().first = current_process;
            ready_processes_queue.front().second = sub_count;

            int current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;
            if (pre_current_process_value != current_process_value) {
                // Rotate: pop and reappend if still has remaining work
                auto finished_pair = ready_processes_queue.front();
                ready_processes_queue.erase(ready_processes_queue.begin());
                
                // If process still has work, move it to the appropriate queue
                if (finished_pair.first.cpu_burst_time2 > 0 || finished_pair.first.cpu_burst_time1 > 0 || finished_pair.first.io_time > 0) {
                    // If process has completed first time quantum, move to second queue
                    if (std::find(done_first_time_quantum.begin(), done_first_time_quantum.end(), 
                                 finished_pair.first.process_id) != done_first_time_quantum.end()) {
                        cycle = 2; // Move to second queue
                    }
                    ready_processes_queue.push_back(finished_pair);
                }
            }

            // Break if all processes are complete
            if (processes_copy.empty() && ready_processes_queue.empty()) break;

            // Ensure time advances
            if (prev_cpu_time == current_cpu_time) current_cpu_time += 1;
            prev_cpu_time = current_cpu_time;

            // Add newly arrived processes to ready queue
            std::vector<std::pair<Process, int>> temp;
            for (auto it = processes_copy.begin(); it != processes_copy.end();) {
                if (it->arrival_time <= current_cpu_time) {
                    temp.insert(temp.begin(), {*it, 0});
                    it = processes_copy.erase(it);
                } else ++it;
            }

            // Insert new arrivals at the front of the ready queue
            if (!temp.empty()) {
                for (auto& pi : temp) {
                    ready_processes_queue.insert(ready_processes_queue.begin(), pi);
                }
            }
        }
    }

    void fcfs_queue() {
        // Sort remaining processes by arrival time
        std::vector<Process> remaining_processes;
        for (const auto& p : processes) {
            bool found = false;
            for (const auto& info : grantt_chart) {
                if (info.process.process_id == p.process_id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                remaining_processes.push_back(p);
            }
        }
        
        if (remaining_processes.empty()) return;
        
        std::sort(remaining_processes.begin(), remaining_processes.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        // Process remaining processes in FCFS order
        std::queue<Process> ready_queue;
        int current_time = current_cpu_time; // Continue from where two_queue left off
        size_t next_process_idx = 0;
        
        while (next_process_idx < remaining_processes.size() || !ready_queue.empty()) {
            // Add newly arrived processes to ready queue
            while (next_process_idx < remaining_processes.size() && 
                   remaining_processes[next_process_idx].arrival_time <= current_time) {
                ready_queue.push(remaining_processes[next_process_idx]);
                next_process_idx++;
            }
            
            // If no processes are ready, jump to next arrival time
            if (ready_queue.empty() && next_process_idx < remaining_processes.size()) {
                current_time = remaining_processes[next_process_idx].arrival_time;
                continue;
            }
            
            // Process the next process in the queue
            if (!ready_queue.empty()) {
                Process current_process = ready_queue.front();
                ready_queue.pop();
                
                // Create grant info for this process
                ProcessGrantInfo info(current_process, current_time, 0, 0, 0, 0, 0);
                
                // First CPU burst
                if (current_process.cpu_burst_time1 > 0) {
                    info.cpu_start_time1 = current_time;
                    current_time += current_process.cpu_burst_time1;
                    info.cpu_end_time1 = current_time;
                }
                
                // I/O burst
                if (current_process.io_time > 0) {
                    info.io_start_time = current_time;
                    current_time += current_process.io_time;
                    info.io_end_time = current_time;
                }
                
                // Second CPU burst
                if (current_process.cpu_burst_time2 > 0) {
                    info.cpu_start_time2 = current_time;
                    current_time += current_process.cpu_burst_time2;
                    info.cpu_end_time2 = current_time;
                }
                
                grantt_chart.push_back(info);
            }
        }
        
        // Update the current CPU time
        current_cpu_time = current_time;
    }
};