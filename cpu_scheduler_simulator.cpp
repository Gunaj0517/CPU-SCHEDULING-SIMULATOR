#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>

#include "Process.h"
#include "ProcessGrantInfo.h"
#include "GranttAnalysis.h"

// Include all scheduler implementations
#include "fcfs.cpp"
#include "sjf.cpp"
#include "rr.cpp"
#include "mlfq.cpp"
#include "fcfsPreemp.cpp"
#include "priority.cpp"

using namespace std;

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
    cout << "\n==================================================\n";
    cout << "           CPU SCHEDULING SIMULATOR           \n";
    cout << "==================================================\n";
    cout << "  1. First Come First Served (FCFS)\n";
    cout << "  2. Shortest Job First (SJF) - Non-preemptive\n";
    cout << "  3. Round Robin (RR)\n";
    cout << "  4. Multi-Level Feedback Queue (MLFQ)\n";
    cout << "  5. Priority Scheduling - Non-preemptive\n";
    cout << "  6. Priority Scheduling - Preemptive\n";
    cout << "  7. Enter Process Data\n";
    cout << "  8. Display Current Process Data\n";
    cout << "  9. Exit\n";
    cout << "==================================================\n";
    cout << "Enter your choice: ";
}

// Function to get process data from user
vector<Process> getProcessData() {
    vector<Process> processes;
    int n;
    
    cout << "\nEnter the number of processes: ";
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string pid;
        int arrival_time, cpu_burst1, io_time, cpu_burst2, priority;
        
        cout << "\nProcess " << (i + 1) << ":" << endl;
        cout << "Process ID: ";
        cin >> pid;
        
        cout << "Arrival Time: ";
        cin >> arrival_time;
        
        cout << "First CPU Burst Time: ";
        cin >> cpu_burst1;
        
        cout << "I/O Time: ";
        cin >> io_time;
        
        cout << "Second CPU Burst Time: ";
        cin >> cpu_burst2;
        
        cout << "Priority (lower number = higher priority): ";
        cin >> priority;
        
        processes.push_back(Process(pid, arrival_time, cpu_burst1, io_time, cpu_burst2, priority));
    }
    
    return processes;
}

// Function to display process data
void displayProcessData(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    cout << "\n==================================================\n";
    cout << "                PROCESS DATA                 \n";
    cout << "==================================================\n";
    cout << setw(5) << "PID" << setw(12) << "Arrival" << setw(12) << "CPU Burst 1" 
         << setw(12) << "I/O Time" << setw(12) << "CPU Burst 2" << setw(12) << "Priority" << endl;
    cout << "--------------------------------------------------\n";
    
    for (const auto& p : processes) {
        cout << setw(5) << p.pid << setw(12) << p.arrival_time << setw(12) << p.cpu_burst_time1 
             << setw(12) << p.io_time << setw(12) << p.cpu_burst_time2 << setw(12) << p.priority << endl;
    }
    cout << "==================================================\n";
}

// Function to convert Process objects to match the format expected by SJF, RR, and MLFQ
vector<Process> convertProcessFormat(const vector<Process>& processes) {
    // With the updated Process.h, we can just return a copy of the processes
    // since process_id is now automatically set in the constructor
    return processes;
}

// Function to convert ProcessGrantInfo objects from SJF, RR, and MLFQ format to the format expected by GranttAnalysis
vector<ProcessGrantInfo> convertGranttFormat(const vector<ProcessGrantInfo>& chart, const vector<Process>& originalProcesses) {
    vector<ProcessGrantInfo> converted;
    for (const auto& info : chart) {
        // Find the original Process object
        Process originalProcess;
        for (const auto& p : originalProcesses) {
            if (p.process_id == info.process.process_id) {
                originalProcess = p;
                break;
            }
        }
        
        // Create a new ProcessGrantInfo object with the original Process
        converted.push_back(ProcessGrantInfo(
            originalProcess,
            info.cpu_start_time1,
            info.io_start_time,
            info.cpu_start_time2,
            info.cpu_end_time1,
            info.io_end_time,
            info.cpu_end_time2
        ));
    }
    return converted;
}

// Function to run FCFS scheduling
void runFCFS(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    FCFS scheduler(processes);
    vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("First Come First Served (FCFS)");
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Function to run SJF scheduling
void runSJF(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    vector<Process> convertedProcesses = convertProcessFormat(processes);
    SJF scheduler(convertedProcesses);
    vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    vector<ProcessGrantInfo> convertedChart = convertGranttFormat(chart, processes);
    
    GranttAnalysis analysis(convertedChart, processes);
    analysis.pretty_print("Shortest Job First (SJF) - Non-preemptive");
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Function to run Round Robin scheduling
void runRoundRobin(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    int quantum;
    cout << "\nEnter time quantum for Round Robin: ";
    cin >> quantum;
    
    vector<Process> convertedProcesses = convertProcessFormat(processes);
    RoundRobin scheduler(convertedProcesses);
    vector<ProcessGrantInfo> chart = scheduler.cpu_process(quantum);
    vector<ProcessGrantInfo> convertedChart = convertGranttFormat(chart, processes);
    
    GranttAnalysis analysis(convertedChart, processes);
    analysis.pretty_print("Round Robin (RR) with Time Quantum = " + to_string(quantum));
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Function to run MLFQ scheduling
void runMLFQ(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    vector<Process> convertedProcesses = convertProcessFormat(processes);
    MLFQ scheduler(convertedProcesses);
    vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    vector<ProcessGrantInfo> convertedChart = convertGranttFormat(chart, processes);
    
    GranttAnalysis analysis(convertedChart, processes);
    analysis.pretty_print("Multi-Level Feedback Queue (MLFQ)");
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Function to run Priority scheduling (non-preemptive)
void runPriorityNonPreemptive(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    PriorityScheduler scheduler(processes, false);
    vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Priority Scheduling - Non-preemptive");
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Function to run Priority scheduling (preemptive)
void runPriorityPreemptive(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "\nNo process data available. Please enter process data first.\n";
        return;
    }
    
    PriorityScheduler scheduler(processes, true);
    vector<ProcessGrantInfo> chart = scheduler.cpu_process();
    
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print("Priority Scheduling - Preemptive");
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

int main() {
    vector<Process> processes;
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
        cin >> choice;
        
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
                cout << "\nProcess data updated successfully!\n";
                cout << "\nPress Enter to continue...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                break;
            case 8:
                displayProcessData(processes);
                cout << "\nPress Enter to continue...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                break;
            case 9:
                cout << "\nExiting CPU Scheduling Simulator. Goodbye!\n";
                return 0;
            default:
                cout << "\nInvalid choice. Please try again.\n";
                cout << "\nPress Enter to continue...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
        }
    }
    
    return 0;
}