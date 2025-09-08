#ifndef PROCESS_GRANT_INFO_H
#define PROCESS_GRANT_INFO_H

#include <string>
#include "Process.h"

class ProcessGrantInfo {
public:
    Process process;
    int cpu_start_time1;
    int cpu_start_time2;
    int io_start_time;
    int cpu_end_time1;
    int cpu_end_time2;
    int io_end_time;

    ProcessGrantInfo(Process p,
                     int cpu_start_time1,
                     int io_start_time,
                     int cpu_start_time2,
                     int cpu_end_time1,
                     int io_end_time,
                     int cpu_end_time2)
        : process(p),
          cpu_start_time1(cpu_start_time1),
          io_start_time(io_start_time),
          cpu_start_time2(cpu_start_time2),
          cpu_end_time1(cpu_end_time1),
          io_end_time(io_end_time),
          cpu_end_time2(cpu_end_time2) {}

    int get_start_time() const {
        return cpu_start_time1;
    }

    int get_end_time() const {
        return cpu_end_time2;
    }
};

#endif
