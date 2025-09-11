#ifndef FCFS_PREEMP_H
#define FCFS_PREEMP_H

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

    // Constructor
    FCFSPreemp(const std::vector<Process>& procs) : processes(procs) {}

    // Main scheduling function
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
                
                // Handle preemption
                if (processing) {
                    if (arrived.arrival_time < current_process->arrival_time) {
                        if (burst_phase == 0) {
                            current_info.cpu_end_time1 = current_time;
                            current_process->cpu_burst_time1 = remaining_burst;
                        } else if (burst_phase == 1) {
                            current_info.io_end_time = current_time;
                            current_process->io_time = remaining_burst;
                        } else {
                            current_info.cpu_end_time2 = current_time;
                            current_process->cpu_burst_time2 = remaining_burst;
                        }

                        if ((burst_phase == 0 && current_info.cpu_start_time1 < current_info.cpu_end_time1) ||
                            (burst_phase == 1 && current_info.io_start_time < current_info.io_end_time) ||
                            (burst_phase == 2 && current_info.cpu_start_time2 < current_info.cpu_end_time2)) {
                            grantt_chart.push_back(current_info);
                        }

                        ready_queue.push(*current_process);
                        processing = false;
                    }
                }

                ready_queue.push(arrived);
            }

            // If idle, fetch next process
            if (!processing && !ready_queue.empty()) {
                Process next_process = ready_queue.front();
                ready_queue.pop();

                current_process = new Process(next_process);
                current_info = ProcessGrantInfo(*current_process, current_time, 0, 0, 0, 0, 0);

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

            // Execute one unit of burst
            if (processing) {
                remaining_burst--;

                if (remaining_burst == 0) {
                    if (burst_phase == 0) {
                        current_info.cpu_end_time1 = current_time + 1;
                        current_process->cpu_burst_time1 = 0;

                        if (current_process->io_time > 0) {
                            burst_phase = 1;
                            remaining_burst = current_process->io_time;
                            current_info.io_start_time = current_time + 1;
                        } else if (current_process->cpu_burst_time2 > 0) {
                            burst_phase = 2;
                            remaining_burst = current_process->cpu_burst_time2;
                            current_info.cpu_start_time2 = current_time + 1;
                        } else {
                            grantt_chart.push_back(current_info);
                            processing = false;
                            delete current_process;
                            current_process = nullptr;
                        }
                    } else if (burst_phase == 1) {
                        current_info.io_end_time = current_time + 1;
                        current_process->io_time = 0;

                        if (current_process->cpu_burst_time2 > 0) {
                            burst_phase = 2;
                            remaining_burst = current_process->cpu_burst_time2;
                            current_info.cpu_start_time2 = current_time + 1;
                        } else {
                            grantt_chart.push_back(current_info);
                            processing = false;
                            delete current_process;
                            current_process = nullptr;
                        }
                    } else {
                        current_info.cpu_end_time2 = current_time + 1;
                        current_process->cpu_burst_time2 = 0;

                        grantt_chart.push_back(current_info);
                        processing = false;
                        delete current_process;
                        current_process = nullptr;
                    }
                }
            }

            current_time++;

            if (processes_copy.empty() && ready_queue.empty() && !processing) {
                break;
            }
        }

        if (current_process != nullptr) {
            delete current_process;
        }

        return grantt_chart;
    }
};

#endif // FCFS_PREEMP_H
