#include <vector>
#include <algorithm>
#include <queue>
#include "Process.h"
#include "ProcessGrantInfo.h"

class FCFSPreemp {
public:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    int current_cpu_time = 0;
    std::string ClassName = "FCFSPreemp";

    FCFSPreemp(const std::vector<Process>& procs) : processes(procs) {}

    std::vector<ProcessGrantInfo> cpu_process() {
        std::vector<Process> processes_copy = processes;
        std::sort(processes_copy.begin(), processes_copy.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        std::queue<Process> ready_queue;
        int current_time = 0;
        Process* current_process = nullptr;
        ProcessGrantInfo current_info(Process(), 0, 0, 0, 0, 0, 0);
        bool processing = false;
        int remaining_burst = 0;
        int burst_phase = 0; // 0: first CPU burst, 1: I/O, 2: second CPU burst

        while (!processes_copy.empty() || !ready_queue.empty() || processing) {
            // Add newly arrived processes to ready queue
            while (!processes_copy.empty() && processes_copy.front().arrival_time <= current_time) {
                Process arrived = processes_copy.front();
                processes_copy.erase(processes_copy.begin());
                
                // If a process with higher priority arrives, preempt current process
                if (processing) {
                    // In FCFS preemptive, we preempt if a process with earlier arrival time arrives
                    // (which shouldn't happen in normal FCFS, but we're implementing preemption)
                    if (arrived.arrival_time < current_process->arrival_time) {
                        // Save current process state
                        if (burst_phase == 0) {
                            current_info.cpu_end_time1 = current_time;
                            current_process->cpu_burst_time1 = remaining_burst;
                        } else if (burst_phase == 1) {
                            current_info.io_end_time = current_time;
                            current_process->io_time = remaining_burst;
                        } else { // burst_phase == 2
                            current_info.cpu_end_time2 = current_time;
                            current_process->cpu_burst_time2 = remaining_burst;
                        }
                        
                        // Add current process info to Gantt chart if it ran for some time
                        if ((burst_phase == 0 && current_info.cpu_start_time1 < current_info.cpu_end_time1) ||
                            (burst_phase == 1 && current_info.io_start_time < current_info.io_end_time) ||
                            (burst_phase == 2 && current_info.cpu_start_time2 < current_info.cpu_end_time2)) {
                            grantt_chart.push_back(current_info);
                        }
                        
                        // Put preempted process back in ready queue
                        ready_queue.push(*current_process);
                        processing = false;
                    }
                }
                
                ready_queue.push(arrived);
            }

            // If not processing any process, get one from ready queue
            if (!processing && !ready_queue.empty()) {
                Process next_process = ready_queue.front();
                ready_queue.pop();
                
                current_process = new Process(next_process);
                // Initialize with default values, will update as execution progresses
                current_info = ProcessGrantInfo(*current_process, current_time, 0, 0, 0, 0, 0);
                
                
                // Determine which burst phase to start
                if (current_process->cpu_burst_time1 > 0) {
                    burst_phase = 0;
                    remaining_burst = current_process->cpu_burst_time1;
                    current_info.cpu_start_time1 = current_time;
                } else if (current_process->io_time > 0) {
                    burst_phase = 1;
                    remaining_burst = current_process->io_time;
                    current_info.io_start_time = current_time;
                } else if (current_process->cpu_burst_time2 > 0) {
                    burst_phase = 2;
                    remaining_burst = current_process->cpu_burst_time2;
                    current_info.cpu_start_time2 = current_time;
                }
                
                processing = true;
            }

            // Process current process for one time unit
            if (processing) {
                remaining_burst--;
                
                // If current burst is complete
                if (remaining_burst == 0) {
                    if (burst_phase == 0) {
                        current_info.cpu_end_time1 = current_time + 1;
                        current_process->cpu_burst_time1 = 0;
                        
                        // Move to I/O phase if needed
                        if (current_process->io_time > 0) {
                            burst_phase = 1;
                            remaining_burst = current_process->io_time;
                            current_info.io_start_time = current_time + 1;
                        }
                        // Or to second CPU burst if no I/O
                        else if (current_process->cpu_burst_time2 > 0) {
                            burst_phase = 2;
                            remaining_burst = current_process->cpu_burst_time2;
                            current_info.cpu_start_time2 = current_time + 1;
                        }
                        // Or process is complete
                        else {
                            grantt_chart.push_back(current_info);
                            processing = false;
                            delete current_process;
                            current_process = nullptr;
                        }
                    } else if (burst_phase == 1) {
                        current_info.io_end_time = current_time + 1;
                        current_process->io_time = 0;
                        
                        // Move to second CPU burst if needed
                        if (current_process->cpu_burst_time2 > 0) {
                            burst_phase = 2;
                            remaining_burst = current_process->cpu_burst_time2;
                            current_info.cpu_start_time2 = current_time + 1;
                        }
                        // Or process is complete
                        else {
                            grantt_chart.push_back(current_info);
                            processing = false;
                            delete current_process;
                            current_process = nullptr;
                        }
                    } else { // burst_phase == 2
                        current_info.cpu_end_time2 = current_time + 1;
                        current_process->cpu_burst_time2 = 0;
                        
                        // Process is complete
                        grantt_chart.push_back(current_info);
                        processing = false;
                        delete current_process;
                        current_process = nullptr;
                    }
                }
            }

            // Advance time
            current_time++;
            
            // If no more processes and nothing in ready queue, break
            if (processes_copy.empty() && ready_queue.empty() && !processing) {
                break;
            }
        }

        // Clean up if needed
        if (current_process != nullptr) {
            delete current_process;
        }

        return grantt_chart;
    }
};