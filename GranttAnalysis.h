#ifndef GRANTT_ANALYSIS_H
#define GRANTT_ANALYSIS_H

#include <vector>
#include <iostream>
#include <iomanip>
#include "ProcessGrantInfo.h"

class GranttAnalysis {
private:
    std::vector<ProcessGrantInfo> grantt_chart;
    std::vector<std::pair<Process, int>> turn_around_time;
    std::vector<std::pair<Process, int>> response_time;
    std::vector<std::pair<Process, int>> waiting_time;

public:
    GranttAnalysis(std::vector<ProcessGrantInfo> chart, std::vector<Process> processes);

    void calculate_turn_around_time();
    void calculate_waiting_time();
    void calculate_response_time();

    int get_total_time() const;
    int get_idle_time() const;
    int get_burst_time() const;
    double get_cpu_efficiency() const;
    double get_throughput() const;

    void pretty_print(const std::string& status);
};

#endif
