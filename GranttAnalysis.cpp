#include "GranttAnalysis.h"

GranttAnalysis::GranttAnalysis(std::vector<ProcessGrantInfo> chart, std::vector<Process> processes) {
    grantt_chart = chart;

    // Map process info back to original processes
    for (auto &info : grantt_chart) {
        for (auto &p : processes) {
            if (info.process.pid == p.pid) {
                info.process = p;
            }
        }
    }
}

void GranttAnalysis::calculate_turn_around_time() {
    for (auto &info : grantt_chart) {
        turn_around_time.push_back({info.process, info.get_end_time() - info.process.arrival_time});
    }
}

void GranttAnalysis::calculate_waiting_time() {
    for (auto &info : grantt_chart) {
        int wt = (info.get_end_time() - info.process.arrival_time) -
                 (info.process.cpu_burst_time1 + info.process.cpu_burst_time2);
        waiting_time.push_back({info.process, wt});
    }
}

void GranttAnalysis::calculate_response_time() {
    for (auto &info : grantt_chart) {
        int rt = info.get_start_time() - info.process.arrival_time;
        response_time.push_back({info.process, rt});
    }
}

int GranttAnalysis::get_total_time() const {
    int max_end_time = 0;
    for (auto &info : grantt_chart) {
        max_end_time = std::max(max_end_time, info.get_end_time());
    }
    return max_end_time;
}

int GranttAnalysis::get_idle_time() const {
    return get_total_time() - get_burst_time();
}

int GranttAnalysis::get_burst_time() const {
    int sum = 0;
    for (auto &info : grantt_chart) {
        sum += info.process.cpu_burst_time1 + info.process.cpu_burst_time2;
    }
    return sum;
}

double GranttAnalysis::get_cpu_efficiency() const {
    return static_cast<double>(get_burst_time()) / get_total_time();
}

double GranttAnalysis::get_throughput() const {
    return (static_cast<double>(grantt_chart.size()) * 1000) / get_total_time();
}

void GranttAnalysis::pretty_print(const std::string& status) {
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
    std::cout << "Efficiency: " << get_cpu_efficiency() << "\n";
    std::cout << "Throughput: " << get_throughput() << " per second\n";

    std::cout << "=======================================================================================================\n";
}
