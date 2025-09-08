#include <iostream>
#include <vector>
#include <string>
using namespace std;
struct Process {
    string pid;                 // Process ID
    int arrival_time;           // Arrival time
    int cpu_burst_time1;        // First CPU burst
    int io_time;                // I/O time
    int cpu_burst_time2;        // Second CPU burst

    Process(string id, int at, int cpu1, int io, int cpu2)
        : pid(id), arrival_time(at),
          cpu_burst_time1(cpu1), io_time(io), cpu_burst_time2(cpu2) {}
};

struct ProcessGrantInfo {
    Process process;
    int first_cpu_start;
    int first_cpu_end;
    int io_start;
    int io_end;
    int second_cpu_start;
    int second_cpu_end;

    ProcessGrantInfo(Process p, int fcs, int fce, int ios, int ioe, int scs, int sce)
        : process(p),
          first_cpu_start(fcs), first_cpu_end(fce),
          io_start(ios), io_end(ioe),
          second_cpu_start(scs), second_cpu_end(sce) {}

    int get_end_time() const {
        return second_cpu_end;
    }

    void print() const {
        cout << "Process " << process.pid
             << " | First CPU: " << first_cpu_start << "-" << first_cpu_end
             << " | IO: " << io_start << "-" << io_end
             << " | Second CPU: " << second_cpu_start << "-" << second_cpu_end
             << endl;
    }
};

class FCFS {
private:
    vector<Process> processes;
    vector<ProcessGrantInfo> grantt_chart;

public:
    FCFS(vector<Process> procs) : processes(procs) {}

    vector<ProcessGrantInfo> cpu_process() {
        for (size_t i = 0; i < processes.size(); i++) {
            Process p = processes[i];

            if (grantt_chart.empty()) {
                int fcs = p.arrival_time;
                int fce = fcs + p.cpu_burst_time1;
                int ios = fce;
                int ioe = ios + p.io_time;
                int scs = ioe;
                int sce = scs + p.cpu_burst_time2;

                grantt_chart.push_back(ProcessGrantInfo(p, fcs, fce, ios, ioe, scs, sce));
            } else {
                ProcessGrantInfo prev = grantt_chart.back();

                if (p.arrival_time < prev.get_end_time()) {
                    int fcs = prev.get_end_time();
                    int fce = fcs + p.cpu_burst_time1;
                    int ios = fce;
                    int ioe = ios + p.io_time;
                    int scs = ioe;
                    int sce = scs + p.cpu_burst_time2;

                    grantt_chart.push_back(ProcessGrantInfo(p, fcs, fce, ios, ioe, scs, sce));
                } else {
                    int fcs = p.arrival_time;
                    int fce = fcs + p.cpu_burst_time1;
                    int ios = fce;
                    int ioe = ios + p.io_time;
                    int scs = ioe;
                    int sce = scs + p.cpu_burst_time2;

                    grantt_chart.push_back(ProcessGrantInfo(p, fcs, fce, ios, ioe, scs, sce));
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

int main() {
    vector<Process> processes = {
        Process("P1", 0, 4, 3, 5),
        Process("P2", 2, 3, 2, 4),
        Process("P3", 5, 2, 4, 3)
    };

    FCFS scheduler(processes);
    scheduler.cpu_process();
    scheduler.print_chart();

    return 0;
}
