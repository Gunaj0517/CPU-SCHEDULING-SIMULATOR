// schedulers.cpp
// Translated from your Python versions: Process, SJF (non-preemptive), RoundRobin, MLFQ (2-level).
// Compile: g++ schedulers.cpp -std=c++17 -O2 -o schedulers

#include <bits/stdc++.h>
using namespace std;

// -------------------- Process --------------------
struct Process {
    int process_id;
    int arrival_time;
    int cpu_burst_time1;
    int cpu_burst_time2;
    int io_time;

    Process() : process_id(0), arrival_time(0), cpu_burst_time1(0), cpu_burst_time2(0), io_time(0) {}
    Process(int pid, int at, int cpu1, int io, int cpu2)
        : process_id(pid), arrival_time(at), cpu_burst_time1(cpu1), cpu_burst_time2(cpu2), io_time(io) {}
};

// -------------------- ProcessGrantInfo --------------------
struct ProcessGrantInfo {
    Process process;
    int cpu_start_time1 = -1;
    int cpu_start_time2 = -1;
    int io_start_time = -1;
    int cpu_end_time1 = -1;
    int cpu_end_time2 = -1;
    int io_end_time = -1;

    ProcessGrantInfo() {}
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
};

// ------------- Utility: clone vector of processes (copy) -------------
static vector<Process> clone_procs(const vector<Process> &v) {
    return vector<Process>(v.begin(), v.end());
}

// -------------------- SJF (non-preemptive) --------------------
class SJF {
public:
    vector<Process> processes;           // remaining processes, sorted by arrival initially
    vector<ProcessGrantInfo> grantt_chart;
    string ClassName = "SJF";

    SJF(const vector<Process> &procs) : processes(clone_procs(procs)) {}

    // helper: sort a queue of processes by total remaining burst (cpu1 + cpu2)
    static vector<Process> sorted_based_on_burst_time(const vector<Process> &queue) {
        vector<Process> copy = queue;
        sort(copy.begin(), copy.end(), [](const Process &a, const Process &b) {
            int ba = a.cpu_burst_time1 + a.cpu_burst_time2;
            int bb = b.cpu_burst_time1 + b.cpu_burst_time2;
            if (ba != bb) return ba < bb;
            return a.arrival_time < b.arrival_time;
        });
        return copy;
    }

    vector<ProcessGrantInfo> cpu_process() {
        bool not_started = false;
        vector<Process> ready_processes_queue;

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
                    int base = prev.get_end_time();
                    grantt_chart.emplace_back(current_process,
                                              current_process.arrival_time,                           // cpu_start_time1
                                              base + current_process.cpu_burst_time1,                 // io_start_time
                                              base + current_process.cpu_burst_time1 + current_process.io_time, // cpu_start_time2
                                              base + current_process.cpu_burst_time1,                 // cpu_end_time1
                                              base + current_process.cpu_burst_time1 + current_process.io_time, // io_end_time
                                              base + current_process.cpu_burst_time1 + current_process.io_time + current_process.cpu_burst_time2); // cpu_end_time2
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

// -------------------- Round Robin --------------------
class RoundRobin {
public:
    vector<Process> processes;
    vector<ProcessGrantInfo> grantt_chart;
    string ClassName = "RR";

    RoundRobin(const vector<Process> &procs) : processes(clone_procs(procs)) {}

    // returns arrival times map
    unordered_map<int,int> get_arrival_times() {
        unordered_map<int,int> temp;
        for (const auto &p : processes) temp[p.process_id] = p.arrival_time;
        return temp;
    }

    vector<ProcessGrantInfo> cpu_process(int time_quantum) {
        int current_cpu_time = 0;
        int prev_cpu_time = -1;
        bool not_started = false;
        vector<pair<Process,int>> ready_processes_queue; // pair(process, sub_count)
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
                                              max(current_process.arrival_time, current_cpu_time),
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
            vector<pair<Process,int>> temp;
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

// -------------------- MLFQ (two-queue with different quantum) --------------------
class MLFQ {
public:
    vector<Process> processes;
    vector<ProcessGrantInfo> grantt_chart;
    int current_cpu_time = 0;
    string ClassName = "MLFQ";

    MLFQ(const vector<Process> &procs) : processes(clone_procs(procs)) {}

    unordered_map<int,int> get_arrival_times() {
        unordered_map<int,int> ret;
        for (const auto &p : processes) ret[p.process_id] = p.arrival_time;
        return ret;
    }

    vector<ProcessGrantInfo> cpu_process() {
        int first_time_quantum = 8;
        int sec_time_quantum = 16;
        two_queue_with_diff_quantum(first_time_quantum, sec_time_quantum);
        fcfs_queue();
        return grantt_chart;
    }

private:
    void two_queue_with_diff_quantum(int first_time_quantum, int second_time_quantum) {
        vector<int> done_first_time_quantum;
        bool is_sec_burst_allowed = false;
        int processes_count = (int)processes.size();
        int prev_cpu_time = -1;
        bool started = false;
        vector<pair<Process,int>> ready_processes_queue;
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
                                              max(current_process.arrival_time, current_cpu_time),
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
                    // mark io consumed in the original processes vector if exists
                    for (auto &p : processes) if (p.process_id == current_process.process_id) p.io_time = 0;
                } else if (current_process.cpu_burst_time2 > 0 && current_process.cpu_burst_time1 <= 0 && is_sec_burst_allowed) {
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

            // update front pair
            ready_processes_queue.front().first = current_process;
            ready_processes_queue.front().second = sub_count;

            int current_process_value = current_process.cpu_burst_time1 + current_process.cpu_burst_time2 + current_process.io_time;
            if (pre_current_process_value != current_process_value) {
                // rotate
                auto old = ready_processes_queue.front();
                ready_processes_queue.erase(ready_processes_queue.begin());
                total_counter += 1;
                if (old.first.cpu_burst_time2 > 0 || old.first.cpu_burst_time1 > 0 || old.first.io_time > 0) {
                    ready_processes_queue.push_back(old);
                }
            }

            if (processes.empty() && ready_processes_queue.empty()) break;

            if ((int)ready_processes_queue.size() == processes_count && total_counter == processes_count) {
                if (!is_sec_burst_allowed) {
                    is_sec_burst_allowed = true;
                    // move ready processes back to processes list and sort by arrival
                    for (auto &each : ready_processes_queue) processes.push_back(each.first);
                    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b){ return a.arrival_time < b.arrival_time; });
                    ready_processes_queue.clear();
                    done_first_time_quantum.clear();
                    cycle = 2;
                    continue;
                } else {
                    break;
                }
            }

            if (prev_cpu_time == current_cpu_time) current_cpu_time += 1;
            prev_cpu_time = current_cpu_time;

            // bring new arrivals
            vector<pair<Process,int>> temp;
            for (auto it = processes.begin(); it != processes.end();) {
                if (it->arrival_time <= current_cpu_time) {
                    temp.insert(temp.begin(), {*it, 0});
                    it = processes.erase(it);
                } else ++it;
            }
            if (!temp.empty()) {
                for (auto &pi : temp) ready_processes_queue.insert(ready_processes_queue.begin(), pi);
            }
        } // while
    }

    void fcfs_queue() {
        // Matches Python's fcfs_queue logic: fill in missing start/end times and advance current_cpu_time.
        for (auto &pinfo : grantt_chart) {
            if (pinfo.cpu_start_time1 < 0) pinfo.cpu_start_time1 = current_cpu_time;

            if (pinfo.cpu_end_time1 < 0) {
                pinfo.cpu_end_time1 = pinfo.cpu_start_time1 + pinfo.process.cpu_burst_time1;
                current_cpu_time += pinfo.process.cpu_burst_time1;
            }

            if (pinfo.io_start_time < 0) pinfo.io_start_time = pinfo.cpu_end_time1;

            if (pinfo.io_end_time > 0) {
                // NOTE: Python had a bug: `if process_info.io_end_time:` then did `process_info.io_start_time = process_info.io_start_time + process_info.process.io_time`
                // We'll follow intention: set io_end_time = io_start_time + io_time
                pinfo.io_end_time = pinfo.io_start_time + pinfo.process.io_time;
                current_cpu_time += pinfo.process.io_time;
            }

            if (pinfo.cpu_start_time2 < 0) {
                if (pinfo.io_end_time <= current_cpu_time) pinfo.cpu_start_time2 = current_cpu_time;
                else {
                    pinfo.cpu_start_time2 = pinfo.io_end_time + current_cpu_time;
                    current_cpu_time += pinfo.io_end_time;
                }
            } else if (pinfo.cpu_end_time2 < 0) {
                pinfo.cpu_end_time2 = current_cpu_time + pinfo.process.cpu_burst_time2;
                current_cpu_time += pinfo.process.cpu_burst_time2;
            }
        }
    }
};

// -------------------- Small test harness --------------------
void print_chart(const vector<ProcessGrantInfo> &chart) {
    for (const auto &g : chart) {
        cout << "P" << g.process.process_id
             << " | CPU1: " << g.cpu_start_time1 << "-" << g.cpu_end_time1
             << " | IO: " << g.io_start_time << "-" << g.io_end_time
             << " | CPU2: " << g.cpu_start_time2 << "-" << g.cpu_end_time2
             << "\n";
    }
}

int main() {
    // Example set of processes (pid, arrival, cpu1, io, cpu2)
    vector<Process> processes = {
        Process(1, 0, 4, 3, 5),
        Process(2, 2, 3, 2, 4),
        Process(3, 5, 2, 4, 3)
    };

    cout << "=== SJF (non-preemptive) ===\n";
    SJF sjf_proc(processes);
    auto sjf_chart = sjf_proc.cpu_process();
    print_chart(sjf_chart);

    cout << "\n=== Round Robin (quantum 3) ===\n";
    RoundRobin rr_proc(processes);
    auto rr_chart = rr_proc.cpu_process(3);
    print_chart(rr_chart);

    cout << "\n=== MLFQ (8,16) ===\n";
    MLFQ mlfq_proc(processes);
    auto mlfq_chart = mlfq_proc.cpu_process();
    print_chart(mlfq_chart);

    return 0;
}
