#include <iostream>
#include <vector>
#include <string>
#include "Process.h"
#include "ProcessGrantInfo.h"

using namespace std;



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
            std::cout << "Process: " << g.process.pid
                       << " CPU1: " << g.cpu_start_time1 << "-" << g.cpu_end_time1
                       << " IO: " << g.io_start_time << "-" << g.io_end_time
                       << " CPU2: " << g.cpu_start_time2 << "-" << g.cpu_end_time2 << std::endl;
        }
    }
};

// Main function removed to avoid conflicts with cpu_scheduler_simulator.cpp
