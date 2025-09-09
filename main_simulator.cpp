#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <unordered_map>

// Define Process struct once
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
    
    // Constructor for numeric process_id (for compatibility with existing code)
    Process(int id, int at, int cpu1, int io, int cpu2)
        : pid("P" + std::to_string(id)), process_id(id), arrival_time(at),
          cpu_burst_time1(cpu1), io_time(io), cpu_burst_time2(cpu2), priority(0) {}
};

// Define ProcessGrantInfo struct once
struct ProcessGrantInfo {
    Process process;
    int cpu_start_time1;
    int cpu_start_time2;
    int io_start_time;
    int cpu_end_time1;
    int cpu_end_time2;
    int io_end_time;

    ProcessGrantInfo() : cpu_start_time1(-1), cpu_start_time2(-1), io_start_time(-1),
                         cpu_end_time1(-1), cpu_end_time2(-1), io_end_time(-1) {}

    ProcessGrantInfo(const Process &p,
                     int cpu_start_time1_,
                     int io_start_time_,
                     int cpu_start_time2_,
                     int cpu_end_time1_,
                     int io_end_time_,
                     int cpu_end_time2_)
        : process(p),
          cpu_start_time1(cpu_start_time1_),
          cpu_start_time2(cpu_start_time2_),
          io_start_time(io_start_time_),
          cpu_end_time1(cpu_end_time1_),
          cpu_end_time2(cpu_end_time2_),
          io_end_time(io_end_time_) {}

    int get_start_time() const { return cpu_start_time1; }
    int get_end_time() const { return cpu_end_time2; }
    
    void print() const {
        std::cout << "Process " << process.pid
                 << " | First CPU: " << cpu_start_time1 << "-" << cpu_end_time1
                 << " | IO: " << io_start_time << "-" << io_end_time
                 << " | Second CPU: " << cpu_start_time2 << "-" << cpu_end_time2
                 << std::endl;
    }
};

// FCFS Scheduler
class FCFS {
private:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;

public:
    FCFS(std::vector<Process> procs) : processes(procs) {}

    std::vector<ProcessGrantInfo> cpu_process() {
        for (size_t i = 0; i < processes.size(); i++) {
            Process p = processes[i];

            if (grantt_chart.empty()) {
                int fcs = p.arrival_time;
                int fce = fcs + p.cpu_burst_time1;
                int ios = fce;
                int ioe = ios + p.io_time;
                int scs = ioe;
                int sce = scs + p.cpu_burst_time2;

                grantt_chart.push_back(ProcessGrantInfo(p, fcs, ios, scs, fce, ioe, sce));
            } else {
                ProcessGrantInfo prev = grantt_chart.back();

                if (p.arrival_time < prev.get_end_time()) {
                    int fcs = prev.get_end_time();
                    int fce = fcs + p.cpu_burst_time1;
                    int ios = fce;
                    int ioe = ios + p.io_time;
                    int scs = ioe;
                    int sce = scs + p.cpu_burst_time2;

                    grantt_chart.push_back(ProcessGrantInfo(p, fcs, ios, scs, fce, ioe, sce));
                } else {
                    int fcs = p.arrival_time;
                    int fce = fcs + p.cpu_burst_time1;
                    int ios = fce;
                    int ioe = ios + p.io_time;
                    int scs = ioe;
                    int sce = scs + p.cpu_burst_time2;

                    grantt_chart.push_back(ProcessGrantInfo(p, fcs, ios, scs, fce, ioe, sce));
                }
            }
        }
        return grantt_chart;
    }

    void print_chart() {
        for (auto &g : grantt_chart) {
            g.print();
        }
    }
};

// SJF Scheduler (non-preemptive)
class SJF {
public:
    std::vector<Process> processes;           // remaining processes, sorted by arrival initially
    std::vector<ProcessGrantInfo> grantt_chart;
    std::string ClassName = "SJF";

    SJF(const std::vector<Process> &procs) : processes(procs) {}

    // helper: sort a queue of processes by total remaining burst (cpu1 + cpu2)
    static std::vector<Process> sorted_based_on_burst_time(const std::vector<Process> &queue) {
        std::vector<Process> copy = queue;
        std::sort(copy.begin(), copy.end(), [](const Process &a, const Process &b) {
            int ba = a.cpu_burst_time1 + a.cpu_burst_time2;
            int bb = b.cpu_burst_time1 + b.cpu_burst_time2;
            if (ba != bb) return ba < bb;
            return a.arrival_time < b.arrival_time;
        });
        return copy;
    }

    std::vector<ProcessGrantInfo> cpu_process() {
        bool not_started = false;
        std::vector<Process> ready_processes_queue;

        while (true) {
            if (!not_started) {
                if (processes.empty()) break;
                Process first_process = processes.front();
                processes.erase(processes.begin());
                ready_processes_queue.push_back(first_process);

                // move processes with same arrival
                for (auto it = processes.begin(); it != processes.end();) {
                    if (it->arrival_time == first_process.arrival_time) {
                        ready_processes_queue.push_back(*it);
                        it = processes.erase(it);
                    } else ++it;
                }
                not_started = true;
            }

            // sort ready queue by burst time
            ready_processes_queue = sorted_based_on_burst_time(ready_processes_queue);
            Process current_process = ready_processes_queue.front();
            ready_processes_queue.erase(ready_processes_queue.begin());

            if (grantt_chart.empty()) {
                grantt_chart.emplace_back(current_process,
                                          current_process.arrival_time,                                 // cpu_start_time1
                                          current_process.arrival_time + current_process.cpu_burst_time1, // io_start_time
                                          current_process.arrival_time + current_process.cpu_burst_time1 + current_process.io_time, // cpu_start_time2
                                          current_process.arrival_time + current_process.cpu_burst_time1, // cpu_end_time1
                                          current_process.arrival_time + current_process.cpu_burst_time1 + current_process.io_time, // io_end_time
                                          current_process.arrival_time + current_process.cpu_burst_time1 + current_process.io_time + current_process.cpu_burst_time2 // cpu_end_time2
                                          );
            } else {
                ProcessGrantInfo prev = grantt_chart.back();
                if (current_process.arrival_time < prev.get_end_time()) {
                    int base = prev.get_end_time();
                    grantt_chart.emplace_back(current_process,
                                              base + 0,                                                // cpu_start_time1
                                              base + current_process.cpu_burst_time1,                  // io_start_time
                                              base + current_process.cpu_burst_time1 + current_process.io_time, // cpu_start_time2
                                              base + current_process.cpu_burst_time1,                  // cpu_end_time1
                                              base + current_process.cpu_burst_time1 + current_process.io_time, // io_end_time
                                              base + current_process.cpu_burst_time1 + current_process.io_time + current_process.cpu_burst_time2); // cpu_end_time2
                } else {
                    grantt_chart.emplace_back(current_process,
                                              current_process.arrival_time,                           // cpu_start_time1
                                              current_process.arrival_time + current_process.cpu_burst_time1, // io_start_time
                                              current_process.arrival_time + current_process.cpu_burst_time1 + current_process.io_time, // cpu_start_time2
                                              current_process.arrival_time + current_process.cpu_burst_time1, // cpu_end_time1
                                              current_process.arrival_time + current_process.cpu_burst_time1 + current_process.io_time, // io_end_time
                                              current_process.arrival_time + current_process.cpu_burst_time1 + current_process.io_time + current_process.cpu_burst_time2); // cpu_end_time2
                }
            }

            int cpu_current_time = grantt_chart.back().get_end_time();

            // move arrived processes into ready queue
            for (auto it = processes.begin(); it != processes.end();) {
                if (it->arrival_time <= cpu_current_time) {
                    ready_processes_queue.push_back(*it);
                    it = processes.erase(it);
                } else ++it;
            }

            if (processes.empty() && ready_processes_queue.empty()) break;
        }

        return grantt_chart;
    }
};

// Round Robin Scheduler
class RoundRobin {
public:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    std::string ClassName = "RR";

    RoundRobin(const std::vector<Process> &procs) : processes(procs) {}

    // returns arrival times map
    std::unordered_map<int,int> get_arrival_times() {
        std::unordered_map<int,int> temp;
        for (const auto &p : processes) temp[p.process_id] = p.arrival_time;
        return temp;
    }

    std::vector<ProcessGrantInfo> cpu_process(int time_quantum) {
        int current_cpu_time = 0;
        int prev_cpu_time = -1;
        bool not_started = false;
        std::vector<std::pair<Process,int>> ready_processes_queue; // pair(process, sub_count)
        auto processes_next_ready_queue = get_arrival_times();

        while (true) {
            if (!not_started) {
                if (processes.empty()) break;
                Process first_process = processes.front();
                processes.erase(processes.begin());
                ready_processes_queue.emplace_back(first_process, 0);

                // add others with same arrival
                for (auto it = processes.begin(); it != processes.end();) {
                    if (it->arrival_time == first_process.arrival_time) {
                        ready_processes_queue.emplace_back(*it, 0);
                        it = processes.erase(it);
                    } else ++it;
                }
                not_started = true;
            }

            if (ready_processes_queue.empty()) {
                current_cpu_time += 1;
                continue;
            }

            auto &front_pair = ready_processes_queue.front();
            Process current_process = front_pair.first;
            int sub_count = front_pair.second;

            int pre_current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;

            if (current_process.cpu_burst_time1 > 0 && current_process.arrival_time <= current_cpu_time) {
                bool not_entered = true;
                for (auto &e : grantt_chart) if (e.process.process_id == current_process.process_id) { not_entered = false; break; }
                if (not_entered) {
                    grantt_chart.emplace_back(current_process,
                                              std::max(current_process.arrival_time, current_cpu_time),
                                              -1, -1, -1, -1, -1);
                }

                current_process.cpu_burst_time1 -= time_quantum;
                sub_count += 1;
                if (current_process.cpu_burst_time1 < 0) current_cpu_time += (current_process.cpu_burst_time1 + time_quantum);
                else current_cpu_time += time_quantum;
            }

            if (current_process.cpu_burst_time1 <= 0 && current_process.arrival_time <= current_cpu_time &&
                processes_next_ready_queue[current_process.process_id] <= current_cpu_time) {

                if (current_process.io_time > 0) {
                    ready_processes_queue.front().second = 0;
                    sub_count = 0;
                    processes_next_ready_queue[current_process.process_id] = current_cpu_time + current_process.io_time;

                    for (auto &info : grantt_chart) {
                        if (info.process.process_id == current_process.process_id) {
                            info.process = current_process;
                            info.io_start_time = current_cpu_time;
                            info.cpu_end_time1 = current_cpu_time;
                            info.io_end_time = current_cpu_time + current_process.io_time;
                            if (current_process.cpu_burst_time2 <= 0) {
                                info.cpu_start_time2 = -1;
                                info.cpu_end_time2 = -1;
                            }
                            break;
                        }
                    }
                    // consume IO separately - in this model we just mark times and set io_time=0 so later second burst can run
                    // In this translation we will set io_time to 0 to match python (it mutated the process)
                    for (auto &p : processes) if (p.process_id == current_process.process_id) { p.io_time = 0; }
                    // finished flag not needed externally
                } else if (current_process.cpu_burst_time2 > 0 && current_process.cpu_burst_time1 <= 0) {
                    if (sub_count == 0) {
                        for (auto &info : grantt_chart) {
                            if (info.process.process_id == current_process.process_id) {
                                info.cpu_start_time2 = current_cpu_time;
                                break;
                            }
                        }
                    }
                    if (processes_next_ready_queue[current_process.process_id] <= current_cpu_time) {
                        current_process.cpu_burst_time2 -= time_quantum;
                        sub_count += 1;
                        if (current_process.cpu_burst_time2 < 0) current_cpu_time += (current_process.cpu_burst_time2 + time_quantum);
                        else current_cpu_time += time_quantum;
                    }
                    if (current_process.cpu_burst_time2 <= 0) {
                        for (auto &info : grantt_chart) {
                            if (info.process.process_id == current_process.process_id) {
                                info.process = current_process;
                                info.cpu_end_time2 = current_cpu_time;
                                break;
                            }
                        }
                    }
                }
            }

            // update the front entry's process and sub_count
            ready_processes_queue.front().first = current_process;
            ready_processes_queue.front().second = sub_count;

            int current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;
            if (pre_current_process_value != current_process_value) {
                // rotate: pop and reappend if still has remaining
                auto finished_pair = ready_processes_queue.front();
                ready_processes_queue.erase(ready_processes_queue.begin());
                if (finished_pair.first.cpu_burst_time2 > 0 || finished_pair.first.cpu_burst_time1 > 0 || finished_pair.first.io_time > 0) {
                    ready_processes_queue.push_back(finished_pair);
                }
            }

            if (processes.empty() && ready_processes_queue.empty()) break;

            if (prev_cpu_time == current_cpu_time) current_cpu_time += 1;
            prev_cpu_time = current_cpu_time;

            // bring new arrivals
            std::vector<std::pair<Process,int>> temp;
            for (auto it = processes.begin(); it != processes.end();) {
                if (it->arrival_time <= current_cpu_time) {
                    temp.insert(temp.begin(), {*it, 0});
                    it = processes.erase(it);
                } else ++it;
            }

            if (!temp.empty()) {
                // insert at front preserving order inserted (matches python insert(0,...))
                for (auto &pi : temp) ready_processes_queue.insert(ready_processes_queue.begin(), pi);
            }
        }

        return grantt_chart;
    }
};

// MLFQ Scheduler
class MLFQ {
public:
    std::vector<Process> processes;
    std::vector<ProcessGrantInfo> grantt_chart;
    int current_cpu_time = 0;
    std::string ClassName = "MLFQ";

    MLFQ(const std::vector<Process> &procs) : processes(procs) {}

    std::unordered_map<int,int> get_arrival_times() {
        std::unordered_map<int,int> ret;
        for (const auto &p : processes) ret[p.process_id] = p.arrival_time;
        return ret;
    }

    std::vector<ProcessGrantInfo> cpu_process() {
        int first_time_quantum = 8;
        int sec_time_quantum = 16;
        two_queue_with_diff_quantum(first_time_quantum, sec_time_quantum);
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
        std::vector<std::pair<Process,int>> ready_processes_queue;
        auto processes_next_ready_queue = get_arrival_times();
        int cycle = 1;
        int total_counter = 0;

        while (true) {
            int time_quantum = (cycle == 1) ? first_time_quantum : second_time_quantum;

            if (!started) {
                if (processes.empty()) break;
                Process first_process = processes.front();
                processes.erase(processes.begin());
                ready_processes_queue.emplace_back(first_process, 0);
                for (auto it = processes.begin(); it != processes.end();) {
                    if (it->arrival_time == first_process.arrival_time) {
                        ready_processes_queue.emplace_back(*it, 0);
                        it = processes.erase(it);
                    } else ++it;
                }
                started = true;
            }

            if (ready_processes_queue.empty() && !is_sec_burst_allowed) {
                current_cpu_time += 1;
                continue;
            } else if (ready_processes_queue.empty() && is_sec_burst_allowed) {
                if (processes.empty()) break;
                ready_processes_queue.emplace_back(processes.front(), 0);
                processes.erase(processes.begin());
            }

            auto &front_pair = ready_processes_queue.front();
            Process current_process = front_pair.first;
            int sub_count = front_pair.second;
            int pre_current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;

            if (current_process.cpu_burst_time1 > 0 && current_process.arrival_time <= current_cpu_time) {
                bool not_entered = true;
                for (auto &element : grantt_chart) if (element.process.process_id == current_process.process_id) { not_entered = false; break; }
                if (not_entered) {
                    grantt_chart.emplace_back(current_process,
                                              std::max(current_process.arrival_time, current_cpu_time),
                                              -1, -1, -1, -1, -1);
                }

                current_process.cpu_burst_time1 -= time_quantum;
                sub_count += 1;
                if (current_process.cpu_burst_time1 < 0) current_cpu_time += (current_process.cpu_burst_time1 + time_quantum);
                else current_cpu_time += time_quantum;
            }

            if (current_process.cpu_burst_time1 <= 0 && current_process.arrival_time <= current_cpu_time &&
                processes_next_ready_queue[current_process.process_id] <= current_cpu_time) {

                if (current_process.io_time > 0) {
                    ready_processes_queue.front().second = 0;
                    sub_count = 0;
                    processes_next_ready_queue[current_process.process_id] = current_cpu_time + current_process.io_time;

                    for (auto &info : grantt_chart) {
                        if (info.process.process_id == current_process.process_id) {
                            info.process = current_process;
                            info.io_start_time = current_cpu_time;
                            info.cpu_end_time1 = current_cpu_time;
                            info.io_end_time = current_cpu_time + current_process.io_time;
                            if (current_process.cpu_burst_time2 <= 0) {
                                info.cpu_start_time2 = -1;
                                info.cpu_end_time2 = -1;
                            }
                            break;
                        }
                    }
                } else if (current_process.cpu_burst_time2 > 0 && current_process.cpu_burst_time1 <= 0) {
                    if (sub_count == 0) {
                        for (auto &info : grantt_chart) {
                            if (info.process.process_id == current_process.process_id) {
                                info.cpu_start_time2 = current_cpu_time;
                                break;
                            }
                        }
                    }
                    if (processes_next_ready_queue[current_process.process_id] <= current_cpu_time) {
                        current_process.cpu_burst_time2 -= time_quantum;
                        sub_count += 1;
                        if (current_process.cpu_burst_time2 < 0) current_cpu_time += (current_process.cpu_burst_time2 + time_quantum);
                        else current_cpu_time += time_quantum;
                    }
                    if (current_process.cpu_burst_time2 <= 0) {
                        for (auto &info : grantt_chart) {
                            if (info.process.process_id == current_process.process_id) {
                                info.process = current_process;
                                info.cpu_end_time2 = current_cpu_time;
                                break;
                            }
                        }
                    }
                }
            }

            // update the front entry's process and sub_count
            ready_processes_queue.front().first = current_process;
            ready_processes_queue.front().second = sub_count;

            int current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;
            if (pre_current_process_value != current_process_value) {
                // rotate: pop and reappend if still has remaining
                auto finished_pair = ready_processes_queue.front();
                ready_processes_queue.erase(ready_processes_queue.begin());
                if (finished_pair.first.cpu_burst_time2 > 0 || finished_pair.first.cpu_burst_time1 > 0 || finished_pair.first.io_time > 0) {
                    ready_processes_queue.push_back(finished_pair);
                }
            }

            if (processes.empty() && ready_processes_queue.empty()) break;

            if (prev_cpu_time == current_cpu_time) current_cpu_time += 1;
            prev_cpu_time = current_cpu_time;

            // bring new arrivals
            std::vector<std::pair<Process,int>> temp;
            for (auto it = processes.begin(); it != processes.end();) {
                if (it->arrival_time <= current_cpu_time) {
                    temp.insert(temp.begin(), {*it, 0});
                    it = processes.erase(it);
                } else ++it;
            }

            if (!temp.empty()) {
                // insert at front preserving order inserted (matches python insert(0,...))
                for (auto &pi : temp) ready_processes_queue.insert(ready_processes_queue.begin(), pi);
            }
        }
    }

    void fcfs_queue() {
        std::vector<Process> ready_queue;
        
        // Sort processes by arrival time
        std::sort(processes.begin(), processes.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        while (!processes.empty() || !ready_queue.empty()) {
            // Add newly arrived processes to ready queue
            while (!processes.empty() && processes.front().arrival_time <= current_cpu_time) {
                ready_queue.push_back(processes.front());
                processes.erase(processes.begin());
            }
            
            if (ready_queue.empty()) {
                // Jump to next process arrival time if no process in ready queue
                if (!processes.empty()) {
                    current_cpu_time = processes.front().arrival_time;
                    continue;
                } else {
                    break;
                }
            }
            
            // Get the first process in the ready queue (FCFS)
            Process current_process = ready_queue.front();
            ready_queue.erase(ready_queue.begin());
            
            // Execute first CPU burst if not already done
            if (current_process.cpu_burst_time1 > 0) {
                bool not_entered = true;
                for (auto &info : grantt_chart) {
                    if (info.process.process_id == current_process.process_id) {
                        not_entered = false;
                        break;
                    }
                }
                
                if (not_entered) {
                    grantt_chart.emplace_back(current_process,
                                              current_cpu_time,
                                              -1, -1, -1, -1, -1);
                }
                
                int burst_time = current_process.cpu_burst_time1;
                current_process.cpu_burst_time1 = 0;
                current_cpu_time += burst_time;
                
                // Update Gantt chart
                for (auto &info : grantt_chart) {
                    if (info.process.process_id == current_process.process_id) {
                        info.cpu_end_time1 = current_cpu_time;
                        break;
                    }
                }
            }
            
            // Execute I/O if not already done
            if (current_process.io_time > 0) {
                for (auto &info : grantt_chart) {
                    if (info.process.process_id == current_process.process_id) {
                        info.io_start_time = current_cpu_time;
                        info.io_end_time = current_cpu_time + current_process.io_time;
                        break;
                    }
                }
                
                int io_time = current_process.io_time;
                current_process.io_time = 0;
                current_cpu_time += io_time;
            }
            
            // Execute second CPU burst if not already done
            if (current_process.cpu_burst_time2 > 0) {
                for (auto &info : grantt_chart) {
                    if (info.process.process_id == current_process.process_id) {
                        info.cpu_start_time2 = current_cpu_time;
                        break;
                    }
                }
                
                int burst_time = current_process.cpu_burst_time2;
                current_process.cpu_burst_time2 = 0;
                current_cpu_time += burst_time;
                
                // Update Gantt chart
                for (auto &info : grantt_chart) {
                    if (info.process.process_id == current_process.process_id) {
                        info.cpu_end_time2 = current_cpu_time;
                        break;
                    }
                }
            }
        }
    }
};

// Priority Scheduler
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
        std::vector<Process> processes_copy = processes;
        int current_time = 0;
        Process* current_process = nullptr;
        int remaining_burst1 = 0;
        int remaining_burst2 = 0;
        bool in_io = false;
        int io_end_time = 0;
        std::unordered_map<int, ProcessGrantInfo> process_info;
        
        // Sort processes by arrival time initially
        std::sort(processes_copy.begin(), processes_copy.end(), 
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });
        
        while (!processes_copy.empty() || !ready_queue.empty() || current_process != nullptr) {
            // Check for newly arrived processes
            while (!processes_copy.empty() && processes_copy.front().arrival_time <= current_time) {
                ready_queue.push_back(processes_copy.front());
                processes_copy.erase(processes_copy.begin());
            }
            
            // Check if current process needs to be preempted
            if (current_process != nullptr) {
                // Find if there's a higher priority process in ready queue
                auto higher_priority = std::find_if(ready_queue.begin(), ready_queue.end(),
                    [&current_process](const Process& p) {
                        return p.priority < current_process->priority;
                    });
                
                if (higher_priority != ready_queue.end()) {
                    // Preemption occurs
                    // Save remaining burst time of current process
                    if (!in_io) {
                        if (remaining_burst1 > 0) {
                            // Update end time of first CPU burst
                            process_info[current_process->process_id].cpu_end_time1 = current_time;
                        } else if (remaining_burst2 > 0) {
                            // Update end time of second CPU burst
                            process_info[current_process->process_id].cpu_end_time2 = current_time;
                        }
                        
                        // Put current process back in ready queue with remaining bursts
                        Process preempted = *current_process;
                        preempted.cpu_burst_time1 = remaining_burst1;
                        preempted.cpu_burst_time2 = remaining_burst2;
                        ready_queue.push_back(preempted);
                    }
                    
                    // Switch to higher priority process
                    current_process = nullptr;
                }
            }
            
            // If no current process or current process finished its burst
            if (current_process == nullptr && !ready_queue.empty()) {
                // Find highest priority process
                auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                    [](const Process& a, const Process& b) {
                        return a.priority < b.priority;
                    });
                
                // Set as current process
                Process selected = *highest_priority;
                ready_queue.erase(highest_priority);
                current_process = new Process(selected);
                
                // Initialize remaining bursts
                remaining_burst1 = current_process->cpu_burst_time1;
                remaining_burst2 = current_process->cpu_burst_time2;
                in_io = false;
                
                // Create or update process info in Gantt chart
                if (process_info.find(current_process->process_id) == process_info.end()) {
                    // First time this process runs
                    ProcessGrantInfo info;
                    info.process = *current_process;
                    info.cpu_start_time1 = current_time;
                    info.cpu_start_time2 = -1;
                    info.io_start_time = -1;
                    info.cpu_end_time1 = -1;
                    info.io_end_time = -1;
                    info.cpu_end_time2 = -1;
                    process_info[current_process->process_id] = info;
                } else if (remaining_burst1 > 0) {
                    // Process was preempted during first burst
                    // Start time remains the same, just update end time later
                } else if (remaining_burst2 > 0 && process_info[current_process->process_id].cpu_start_time2 == -1) {
                    // Starting second burst for the first time
                    process_info[current_process->process_id].cpu_start_time2 = current_time;
                }
            }
            
            // Process current time unit
            if (current_process != nullptr) {
                if (!in_io) {
                    if (remaining_burst1 > 0) {
                        remaining_burst1--;
                        
                        if (remaining_burst1 == 0) {
                            // First CPU burst completed
                            process_info[current_process->process_id].cpu_end_time1 = current_time + 1;
                            
                            if (current_process->io_time > 0) {
                                // Start I/O
                                in_io = true;
                                io_end_time = current_time + 1 + current_process->io_time;
                                process_info[current_process->process_id].io_start_time = current_time + 1;
                                process_info[current_process->process_id].io_end_time = io_end_time;
                            } else if (remaining_burst2 > 0) {
                                // No I/O, start second CPU burst
                                process_info[current_process->process_id].cpu_start_time2 = current_time + 1;
                            } else {
                                // Process completed
                                delete current_process;
                                current_process = nullptr;
                            }
                        }
                    } else if (remaining_burst2 > 0) {
                        remaining_burst2--;
                        
                        if (remaining_burst2 == 0) {
                            // Second CPU burst completed
                            process_info[current_process->process_id].cpu_end_time2 = current_time + 1;
                            delete current_process;
                            current_process = nullptr;
                        }
                    }
                } else {
                    // In I/O
                    if (current_time + 1 >= io_end_time) {
                        // I/O completed
                        in_io = false;
                        process_info[current_process->process_id].cpu_start_time2 = current_time + 1;
                    }
                }
            }
            
            // Advance time
            current_time++;
            
            // If nothing more to do and no processes will arrive, break
            if (current_process == nullptr && ready_queue.empty() && processes_copy.empty()) {
                break;
            }
        }
        
        // Clean up if needed
        if (current_process != nullptr) {
            delete current_process;
        }
        
        // Convert process_info map to grantt_chart vector
        for (const auto& pair : process_info) {
            grantt_chart.push_back(pair.second);
        }
        
        return grantt_chart;
    }
};

// Gantt Chart Analysis
class GranttAnalysis {
private:
    std::vector<ProcessGrantInfo> grantt_chart;
    std::vector<std::pair<Process, int>> turn_around_time;
    std::vector<std::pair<Process, int>> response_time;
    std::vector<std::pair<Process, int>> waiting_time;

public:
    GranttAnalysis(std::vector<ProcessGrantInfo> chart, std::vector<Process> processes) 
        : grantt_chart(chart) {
        // Map process info back to original processes if needed
    }

    void calculate_turn_around_time() {
        for (auto &info : grantt_chart) {
            turn_around_time.push_back({info.process, info.get_end_time() - info.process.arrival_time});
        }
    }

    void calculate_waiting_time() {
        for (auto &info : grantt_chart) {
            int wt = (info.get_end_time() - info.process.arrival_time) -
                     (info.process.cpu_burst_time1 + info.process.cpu_burst_time2);
            waiting_time.push_back({info.process, wt});
        }
    }

    void calculate_response_time() {
        for (auto &info : grantt_chart) {
            int rt = info.get_start_time() - info.process.arrival_time;
            response_time.push_back({info.process, rt});
        }
    }

    int get_total_time() const {
        int max_end_time = 0;
        for (auto &info : grantt_chart) {
            max_end_time = std::max(max_end_time, info.get_end_time());
        }
        return max_end_time;
    }

    int get_idle_time() const {
        return get_total_time() - get_burst_time();
    }

    int get_burst_time() const {
        int sum = 0;
        for (auto &info : grantt_chart) {
            sum += info.process.cpu_burst_time1 + info.process.cpu_burst_time2;
        }
        return sum;
    }

    double get_cpu_efficiency() const {
        return static_cast<double>(get_burst_time()) / get_total_time();
    }

    double get_throughput() const {
        return (static_cast<double>(grantt_chart.size()) * 1000) / get_total_time();
    }

    void pretty_print(const std::string& status) {
        calculate_response_time();
        calculate_turn_around_time();
        calculate_waiting_time();

        double resp_avg = 0, tat_avg = 0, wt_avg = 0;

        std::cout << "=======================================================================================================\n";
        std::cout << "                                               " << status << "\n";
        std::cout << "=======================================================================================================\n";

        std::cout << "\tPID\tResponse\tTurnaround\tWaiting\tStart\tEnd\n";

        for (size_t i = 0; i < grantt_chart.size(); i++) {
            std::cout << "\t" << grantt_chart[i].process.pid
                      << "\t" << response_time[i].second
                      << "\t\t" << turn_around_time[i].second
                      << "\t\t" << waiting_time[i].second
                      << "\t\t" << grantt_chart[i].get_start_time()
                      << "\t" << grantt_chart[i].get_end_time() << "\n";

            resp_avg += response_time[i].second;
            tat_avg += turn_around_time[i].second;
            wt_avg += waiting_time[i].second;
        }

        resp_avg /= response_time.size();
        tat_avg /= turn_around_time.size();
        wt_avg /= waiting_time.size();

        std::cout << "-------------------------------------------------------------------------------------------------------\n";
        std::cout << "Avg\t\t" << resp_avg << "\t\t" << tat_avg << "\t\t" << wt_avg << "\n";

        std::cout << "Total Time: " << get_total_time() << "\n";
        std::cout << "Idle Time: " << get_idle_time() << "\n";
        std::cout << "Burst Time: " << get_burst_time() << "\n";
        std::cout << "CPU Efficiency: " << get_cpu_efficiency() * 100 << "%\n";
        std::cout << "Throughput: " << get_throughput() << " processes per 1000 time units\n";
    }
};

// Function to clear the console screen (cross-platform)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Function to display the main menu
void displayMenu() {
    std::cout << "\n==================================================\n";
    std::cout << "           CPU SCHEDULING SIMULATOR           \n";
    std::cout << "==================================================\n";
    std::cout << "  1. First Come First Served (FCFS)\n";
    std::cout << "  2. Shortest Job First (SJF) - Non-preemptive\n";
    std::cout << "  3. Round Robin (RR)\n";
    std::cout << "  4. Multi-Level Feedback Queue (MLFQ)\n";
    std::cout << "  5. Priority Scheduling - Non-preemptive\n";
    std::cout << "  6. Priority Scheduling - Preemptive\n";
    std::cout << "  7. Enter Process Data\n";
    std::cout << "  8. Display Current Process Data\n";
    std::cout << "  9. Exit\n";
    std::cout << "==================================================\n";
    std::cout << "Enter your choice: ";
}

// Function to get process data from user
std::vector<Process> getProcessData() {
    std::vector<Process> processes;
    int n;
    
    std::cout << "\nEnter the number of processes: ";
    std::cin >> n;
    
    for (int i = 0; i < n; i++) {
        std::string pid;
        int arrival_time, cpu_burst1, io_time, cpu_burst2, priority;
        
        std::cout << "\nProcess " << (i + 1) << ":" << std::endl;
        std::cout << "Process ID: ";
        std::cin >> pid;
        
        std::cout << "Arrival Time: ";
        std::cin >> arrival_time;
        
        std::cout << "First CPU Burst Time: ";
        std::cin >> cpu_burst1;
        
        std::cout << "I/O Time: ";
        std::cin >> io_time;
        
        std::cout << "Second CPU Burst Time: ";
        std::cin >> cpu_burst2;
        
        std::cout << "Priority (lower number = higher priority): ";
        std::cin >> priority;
        
        processes.push_back(Process(pid, arrival_time, cpu_burst1, io_time, cpu_burst2, priority));
    }
    
    return processes;
}

// Function to display process data
void displayProcessData(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    std::cout << "\n==================================================\n";
    std::cout << "                PROCESS DATA                 \n";
    std::cout << "==================================================\n";
    std::cout << std::setw(5) << "PID" << std::setw(12) << "Arrival" << std::setw(12) << "CPU Burst 1" 
         << std::setw(12) << "I/O Time" << std::setw(12) << "CPU Burst 2" << std::setw(12) << "Priority" << std::endl;
    std::cout << "--------------------------------------------------\n";
    
    for (const auto& p : processes) {
        std::cout << std::setw(5) << p.pid << std::setw(12) << p.arrival_time << std::setw(12) << p.cpu_burst_time1 
             << std::setw(12) << p.io_time << std::setw(12) << p.cpu_burst_time2 << std::setw(12) << p.priority << std::endl;
    }
    std::cout << "==================================================\n";
}

// Function to run FCFS scheduling
void runFCFS(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    FCFS scheduler(processes);
    std::vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("First Come First Served (FCFS)");
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Function to run SJF scheduling
void runSJF(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    SJF scheduler(processes);
    std::vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Shortest Job First (SJF) - Non-preemptive");
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Function to run Round Robin scheduling
void runRoundRobin(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    int quantum;
    std::cout << "\nEnter time quantum for Round Robin: ";
    std::cin >> quantum;
    
    RoundRobin scheduler(processes);
    std::vector<ProcessGrantInfo> chart = scheduler.cpu_process(quantum);
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Round Robin (RR) with Time Quantum = " + std::to_string(quantum));
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Function to run MLFQ scheduling
void runMLFQ(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    MLFQ scheduler(processes);
    std::vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Multi-Level Feedback Queue (MLFQ)");
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Function to run Priority scheduling (non-preemptive)
void runPriorityNonPreemptive(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    PriorityScheduler scheduler(processes, false);
    std::vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Priority Scheduling - Non-preemptive");
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Function to run Priority scheduling (preemptive)
void runPriorityPreemptive(const std::vector<Process>& processes) {
    if (processes.empty()) {
        std::cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    PriorityScheduler scheduler(processes, true);
    std::vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Priority Scheduling - Preemptive");
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    std::vector<Process> processes;
    int choice;
    
    // Default sample processes
    processes = {
        Process("P1", 0, 4, 3, 5, 3),
        Process("P2", 2, 3, 2, 4, 1),
        Process("P3", 5, 2, 4, 3, 2)
    };
    
    while (true) {
        clearScreen();
        displayMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                runFCFS(processes);
                break;
            case 2:
                runSJF(processes);
                break;
            case 3:
                runRoundRobin(processes);
                break;
            case 4:
                runMLFQ(processes);
                break;
            case 5:
                runPriorityNonPreemptive(processes);
                break;
            case 6:
                runPriorityPreemptive(processes);
                break;
            case 7:
                processes = getProcessData();
                std::cout << "\nProcess data updated successfully!\n";
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;
            case 8:
                displayProcessData(processes);
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;
            case 9:
                std::cout << "\nExiting CPU Scheduling Simulator. Goodbye!\n";
                return 0;
            default:
                std::cout << "\nInvalid choice. Please try again.\n";
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
        }
    }
    
    return 0;
}