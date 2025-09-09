#include <vector>
#include <algorithm>
#include <unordered_map>
#include "Process.h"
#include "ProcessGrantInfo.h"

class RoundRobin {
public:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    int current_cpu_time = 0;
    std::string ClassName = "RoundRobin";

    RoundRobin(const std::vector<Process>& procs) : processes(procs) {}

    std::unordered_map<int, int> get_arrival_times() {
        std::unordered_map<int, int> ret;
        for (const auto& p : processes) ret[p.process_id] = p.arrival_time;
        return ret;
    }

    std::vector<ProcessGrantInfo> cpu_process(int time_quantum = 4) {
        std::vector<Process> processes_copy = processes;
        std::sort(processes_copy.begin(), processes_copy.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        std::vector<std::pair<Process, int>> ready_processes_queue; // Process and sub_count
        int prev_cpu_time = -1;
        bool started = false;

        while (true) {
            // Initialize with first process and any others arriving at the same time
            if (!started) {
                if (processes_copy.empty()) break;
                Process first_process = processes_copy.front();
                processes_copy.erase(processes_copy.begin());
                ready_processes_queue.emplace_back(first_process, 0);
                
                // Add any other processes arriving at the same time
                for (auto it = processes_copy.begin(); it != processes_copy.end();) {
                    if (it->arrival_time == first_process.arrival_time) {
                        ready_processes_queue.emplace_back(*it, 0);
                        it = processes_copy.erase(it);
                    } else {
                        ++it;
                    }
                }
                started = true;
            }

            // If no processes in ready queue, advance time or break if done
            if (ready_processes_queue.empty()) {
                if (processes_copy.empty()) break;
                current_cpu_time += 1;
                continue;
            }

            // Get the front process from the ready queue
            Process& current_process = ready_processes_queue.front().first;
            int& sub_count = ready_processes_queue.front().second;

            // Process the current process for time quantum or until completion
            int pre_current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;
            
            // Create grant info if this is a new process or continuing after I/O
            if (sub_count == 0 && current_process.cpu_burst_time1 > 0) {
                ProcessGrantInfo info(current_process, current_cpu_time, 0, 0, 0, 0, 0);
                info.cpu_start_time1 = current_cpu_time;
                
                // Calculate how much CPU time to allocate in this quantum
                int cpu_time = std::min(time_quantum, current_process.cpu_burst_time1);
                current_process.cpu_burst_time1 -= cpu_time;
                current_cpu_time += cpu_time;
                info.cpu_end_time1 = current_cpu_time;
                
                // If first CPU burst is complete, start I/O
                if (current_process.cpu_burst_time1 == 0 && current_process.io_time > 0) {
                    info.io_start_time = current_cpu_time;
                    current_process.io_time = 0; // Complete I/O
                    info.io_end_time = current_cpu_time + current_process.io_time;
                    sub_count = 1; // Mark as ready for second CPU burst
                }
                
                grantt_chart.push_back(info);
            }
            // Handle second CPU burst
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

            // Check if process is complete or needs to be moved to the back of the queue
            int current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;
            if (pre_current_process_value != current_process_value) {
                // Rotate: pop and reappend if still has remaining work
                auto finished_pair = ready_processes_queue.front();
                ready_processes_queue.erase(ready_processes_queue.begin());
                if (finished_pair.first.cpu_burst_time2 > 0 || finished_pair.first.cpu_burst_time1 > 0 || finished_pair.first.io_time > 0) {
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
                } else {
                    ++it;
                }
            }

            // Insert new arrivals at the front of the ready queue
            if (!temp.empty()) {
                for (auto& pi : temp) {
                    ready_processes_queue.insert(ready_processes_queue.begin(), pi);
                }
            }
        }

        return grantt_chart;
    }
};