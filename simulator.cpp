#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>

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
    cout << "  7. FCFS Preemptive\n";
    cout << "  8. Enter Process Data\n";
    cout << "  9. Display Current Process Data\n";
    cout << "  10. Save Process Data to File\n";
    cout << "  11. Load Process Data from File\n";
    cout << "  12. Generate Random Processes\n";
    cout << "  13. Exit\n";
    cout << "==================================================\n";
    cout << "Enter your choice: ";
}

// Function to get process data from user
vector<Process> getProcessData() {
    vector<Process> processes;
    int n;
    
    cout << "Enter the number of processes: ";
    cin >> n;
    
    for (int i = 1; i <= n; i++) {
        int arrival_time, cpu_burst1, io_time, cpu_burst2, priority;
        
        cout << "\nProcess P" << i << ":\n";
        cout << "Arrival Time: ";
        cin >> arrival_time;
        cout << "First CPU Burst: ";
        cin >> cpu_burst1;
        cout << "I/O Time: ";
        cin >> io_time;
        cout << "Second CPU Burst (0 if none): ";
        cin >> cpu_burst2;
        cout << "Priority (lower number = higher priority): ";
        cin >> priority;
        
        processes.push_back(Process("P" + to_string(i), arrival_time, cpu_burst1, io_time, cpu_burst2, priority));
    }
    
    return processes;
}

// Function to display process data
void displayProcessData(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "No process data available. Please enter process data first.\n";
        return;
    }
    
    cout << "\nCurrent Process Data:\n";
    cout << "+-------+---------------+----------------+----------+----------------+----------+\n";
    cout << "| PID   | Arrival Time   | CPU Burst 1    | I/O Time | CPU Burst 2    | Priority |\n";
    cout << "+-------+---------------+----------------+----------+----------------+----------+\n";
    
    for (const auto& p : processes) {
        cout << "| " << setw(5) << left << p.pid
             << " | " << setw(13) << right << p.arrival_time
             << " | " << setw(14) << right << p.cpu_burst_time1
             << " | " << setw(8) << right << p.io_time
             << " | " << setw(14) << right << p.cpu_burst_time2
             << " | " << setw(8) << right << p.priority
             << " |\n";
    }
    
    cout << "+-------+---------------+----------------+----------+----------------+----------+\n";
}

// Function to save process data to file
void saveProcessDataToFile(const vector<Process>& processes) {
    if (processes.empty()) {
        cout << "No process data available. Please enter process data first.\n";
        return;
    }
    
    string filename;
    cout << "Enter filename to save process data: ";
    cin >> filename;
    
    ofstream file(filename);
    if (!file) {
        cout << "Error opening file for writing.\n";
        return;
    }
    
    for (const auto& p : processes) {
        file << p.arrival_time << " " << p.cpu_burst_time1 << " " 
             << p.io_time << " " << p.cpu_burst_time2 << " " << p.priority << "\n";
    }
    
    cout << "Process data saved to " << filename << "\n";
}

// Function to load process data from file
vector<Process> loadProcessDataFromFile() {
    vector<Process> processes;
    string filename;
    
    cout << "Enter filename to load process data from: ";
    cin >> filename;
    
    ifstream file(filename);
    if (!file) {
        cout << "Error opening file for reading.\n";
        return processes;
    }
    
    int id = 1;
    int arrival_time, cpu_burst1, io_time, cpu_burst2, priority;
    
    while (file >> arrival_time >> cpu_burst1 >> io_time >> cpu_burst2 >> priority) {
        processes.push_back(Process("P" + to_string(id), arrival_time, cpu_burst1, io_time, cpu_burst2, priority));
        id++;
    }
    
    cout << id - 1 << " processes loaded from " << filename << "\n";
    return processes;
}

// Function to generate random processes
vector<Process> generateRandomProcesses() {
    vector<Process> processes;
    int n;
    
    cout << "Enter the number of random processes to generate: ";
    cin >> n;
    
    for (int i = 1; i <= n; i++) {
        int arrival_time = rand() % 20;  // 0-19
        int cpu_burst1 = 1 + rand() % 10;  // 1-10
        int io_time = rand() % 10;  // 0-9
        int cpu_burst2 = rand() % 10;  // 0-9
        int priority = 1 + rand() % 10;  // 1-10
        
        processes.push_back(Process("P" + to_string(i), arrival_time, cpu_burst1, io_time, cpu_burst2, priority));
    }
    
    cout << n << " random processes generated.\n";
    return processes;
}

// Function to display Gantt chart and metrics
void displayResults(const string& algorithm_name, const vector<ProcessGrantInfo>& gantt_chart) {
    if (gantt_chart.empty()) {
        cout << "No results to display.\n";
        return;
    }
    
    cout << "\n==================================================\n";
    cout << "           " << algorithm_name << " Results           \n";
    cout << "==================================================\n";
    
    // Display Gantt chart
    cout << "\nGantt Chart:\n";
    
    // Find the maximum end time
    int max_end_time = 0;
    for (const auto& info : gantt_chart) {
        int end_time = max(info.cpu_end_time2 > 0 ? info.cpu_end_time2 : 
                          (info.io_end_time > 0 ? info.io_end_time : info.cpu_end_time1), 0);
        max_end_time = max(max_end_time, end_time);
    }
    
    // Create a map of process IDs to their positions in the chart
    vector<string> process_ids;
    for (const auto& info : gantt_chart) {
        bool found = false;
        for (const auto& id : process_ids) {
            if (id == info.process.pid) {
                found = true;
                break;
            }
        }
        if (!found) {
            process_ids.push_back(info.process.pid);
        }
    }
    
    // Sort process IDs
    sort(process_ids.begin(), process_ids.end());
    
    // Display timeline
    cout << "     ";
    for (int t = 0; t <= max_end_time; t += 5) {
        cout << setw(5) << left << t;
    }
    cout << "\n";
    
    cout << "     ";
    for (int t = 0; t <= max_end_time; t += 5) {
        cout << "|    ";
    }
    cout << "|\n";
    
    // Display process timelines
    for (const auto& pid : process_ids) {
        cout << pid << " : ";
        
        // Create a timeline for this process
        string timeline(max_end_time + 1, ' ');
        
        for (const auto& info : gantt_chart) {
            if (info.process.pid == pid) {
                // Mark CPU bursts with 'C'
                if (info.cpu_start_time1 >= 0 && info.cpu_end_time1 > 0) {
                    for (int t = info.cpu_start_time1; t < info.cpu_end_time1; t++) {
                        if (t < timeline.size()) timeline[t] = 'C';
                    }
                }
                
                // Mark I/O bursts with 'I'
                if (info.io_start_time >= 0 && info.io_end_time > 0) {
                    for (int t = info.io_start_time; t < info.io_end_time; t++) {
                        if (t < timeline.size()) timeline[t] = 'I';
                    }
                }
                
                // Mark second CPU bursts with 'C'
                if (info.cpu_start_time2 >= 0 && info.cpu_end_time2 > 0) {
                    for (int t = info.cpu_start_time2; t < info.cpu_end_time2; t++) {
                        if (t < timeline.size()) timeline[t] = 'C';
                    }
                }
            }
        }
        
        // Print the timeline
        for (int t = 0; t < timeline.size(); t++) {
            cout << timeline[t];
        }
        cout << "\n";
    }
    
    // Calculate and display metrics
    double total_wait_time = 0;
    double total_turnaround_time = 0;
    double total_response_time = 0;
    double total_cpu_time = 0;
    
    for (const auto& info : gantt_chart) {
        // Wait time = (start time - arrival time) + any waiting between bursts
        int wait_time = info.cpu_start_time1 - info.process.arrival_time;
        if (info.cpu_start_time2 > 0 && info.io_end_time > 0) {
            wait_time += info.cpu_start_time2 - info.io_end_time;
        }
        
        // Turnaround time = completion time - arrival time
        int completion_time = 0;
        if (info.cpu_end_time2 > 0) {
            completion_time = info.cpu_end_time2;
        } else if (info.io_end_time > 0) {
            completion_time = info.io_end_time;
        } else {
            completion_time = info.cpu_end_time1;
        }
        int turnaround_time = completion_time - info.process.arrival_time;
        
        // Response time = first CPU start - arrival time
        int response_time = info.cpu_start_time1 - info.process.arrival_time;
        
        // CPU time
        int cpu_time = (info.cpu_end_time1 - info.cpu_start_time1);
        if (info.cpu_end_time2 > 0 && info.cpu_start_time2 >= 0) {
            cpu_time += (info.cpu_end_time2 - info.cpu_start_time2);
        }
        
        total_wait_time += wait_time;
        total_turnaround_time += turnaround_time;
        total_response_time += response_time;
        total_cpu_time += cpu_time;
    }
    
    int num_processes = process_ids.size();
    double avg_wait_time = total_wait_time / num_processes;
    double avg_turnaround_time = total_turnaround_time / num_processes;
    double avg_response_time = total_response_time / num_processes;
    double cpu_utilization = (total_cpu_time / max_end_time) * 100;
    
    cout << "\nPerformance Metrics:\n";
    cout << "Average Waiting Time: " << fixed << setprecision(2) << avg_wait_time << "\n";
    cout << "Average Turnaround Time: " << avg_turnaround_time << "\n";
    cout << "Average Response Time: " << avg_response_time << "\n";
    cout << "CPU Utilization: " << cpu_utilization << "%\n";
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Main function
int main() {
    vector<Process> processes;
    int choice;
    bool exit_program = false;
    
    // Seed random number generator
    srand(time(nullptr));
    
    while (!exit_program) {
        clearScreen();
        displayMenu();
        cin >> choice;
        
        // Clear input buffer
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: // FCFS
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                // Assuming FCFS class exists in fcfs.cpp
                // FCFS fcfs(processes);
                // displayResults("FCFS", fcfs.cpu_process());
                cout << "FCFS not implemented yet.\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 2: // SJF
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                {
                    SJF sjf(processes);
                    displayResults("Shortest Job First", sjf.cpu_process());
                }
                break;
                
            case 3: // Round Robin
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                {
                    int quantum;
                    cout << "Enter time quantum for Round Robin: ";
                    cin >> quantum;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    
                    RoundRobin rr(processes);
                    displayResults("Round Robin (Quantum = " + to_string(quantum) + ")", rr.cpu_process(quantum));
                }
                break;
                
            case 4: // MLFQ
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                {
                    MLFQ mlfq(processes);
                    displayResults("Multi-Level Feedback Queue", mlfq.cpu_process());
                }
                break;
                
            case 5: // Priority (Non-preemptive)
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                // Assuming Priority class exists in priority.cpp
                // Priority priority(processes);
                // displayResults("Priority (Non-preemptive)", priority.cpu_process());
                cout << "Priority Scheduling (Non-preemptive) not implemented yet.\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 6: // Priority (Preemptive)
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                // Assuming PriorityPreemptive class exists in priority.cpp
                // PriorityPreemptive priority_preemptive(processes);
                // displayResults("Priority (Preemptive)", priority_preemptive.cpu_process());
                cout << "Priority Scheduling (Preemptive) not implemented yet.\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 7: // FCFS Preemptive
                if (processes.empty()) {
                    cout << "No process data available. Please enter process data first.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }
                {
                    FCFSPreemp fcfs_preemp(processes);
                    displayResults("FCFS Preemptive", fcfs_preemp.cpu_process());
                }
                break;
                
            case 8: // Enter Process Data
                processes = getProcessData();
                cout << "Process data entered successfully.\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 9: // Display Current Process Data
                displayProcessData(processes);
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 10: // Save Process Data to File
                saveProcessDataToFile(processes);
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 11: // Load Process Data from File
                processes = loadProcessDataFromFile();
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 12: // Generate Random Processes
                processes = generateRandomProcesses();
                cout << "Press Enter to continue...";
                cin.get();
                break;
                
            case 13: // Exit
                exit_program = true;
                break;
                
            default:
                cout << "Invalid choice. Please try again.\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
        }
    }
    
    cout << "Thank you for using CPU Scheduler Simulator!\n";
    return 0;
}