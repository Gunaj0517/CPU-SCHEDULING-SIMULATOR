#include <vector>
#include <algorithm>
#include <limits>
#include "Process.h"
#include "ProcessGrantInfo.h"

class SJF {
public:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    int current_cpu_time = 0;
    std::string ClassName = "SJF";

    SJF(const std::vector<Process>& procs) : processes(procs) {}

    std::vector<ProcessGrantInfo> cpu_process() {
        std::vector<Process> processes_copy = processes;
        std::sort(processes_copy.begin(), processes_copy.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        std::vector<Process> ready_queue;
        int current_time = 0;

        while (!processes_copy.empty() || !ready_queue.empty()) {
            // Add newly arrived processes to ready queue
            while (!processes_copy.empty() && processes_copy.front().arrival_time <= current_time) {
                ready_queue.push_back(processes_copy.front());
                processes_copy.erase(processes_copy.begin());
            }

            if (ready_queue.empty()) {
                // Jump to next arrival time if no processes are ready
                current_time = processes_copy.front().arrival_time;
                continue;
            }

            // Find process with shortest burst time
            int shortest_idx = 0;
            int shortest_burst = std::numeric_limits<int>::max();
            for (size_t i = 0; i < ready_queue.size(); i++) {
                int total_burst = ready_queue[i].cpu_burst_time1 + ready_queue[i].io_time + ready_queue[i].cpu_burst_time2;
                if (total_burst < shortest_burst) {
                    shortest_burst = total_burst;
                    shortest_idx = i;
                }
            }

            // Process the selected process
            Process current_process = ready_queue[shortest_idx];
            ready_queue.erase(ready_queue.begin() + shortest_idx);

            // Create grant info for this process
            ProcessGrantInfo info(current_process, current_time, 0, 0, 0, 0, 0);
            
            // First CPU burst
            info.cpu_start_time1 = current_time;
            current_time += current_process.cpu_burst_time1;
            info.cpu_end_time1 = current_time;
            
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

        return grantt_chart;
    }
};