#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

// Include combined headers
#include "GranttAnalysisFull.h"
#include "FCFS.h"
#include "SJF.h"
#include "RoundRobin.h"
#include "PriorityScheduler.h"
#include "MLFQ.h"
#include "FCFSPreemp.h"

using namespace std;

// ================== Utility Functions ==================

// Clear screen (cross-platform)
void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Main Menu
void displayMenu()
{
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

// Input processes from user
vector<Process> getProcessData()
{
    vector<Process> processes;
    int n;
    cout << "Enter the number of processes: ";
    cin >> n;

    for (int i = 1; i <= n; i++)
    {
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
        cout << "Priority (lower = higher): ";
        cin >> priority;

        processes.push_back(Process("P" + to_string(i), arrival_time, cpu_burst1, io_time, cpu_burst2, priority));
    }
    return processes;
}

// Display processes in a table
void displayProcessData(const vector<Process> &processes)
{
    if (processes.empty())
    {
        cout << "No process data available.\n";
        return;
    }

    cout << "\nCurrent Process Data:\n";
    cout << "+-------+---------------+----------------+----------+----------------+----------+\n";
    cout << "| PID   | Arrival Time   | CPU Burst 1    | I/O Time | CPU Burst 2    | Priority |\n";
    cout << "+-------+---------------+----------------+----------+----------------+----------+\n";

    for (const auto &p : processes)
    {
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

// Save processes to file
void saveProcessDataToFile(const vector<Process> &processes)
{
    if (processes.empty())
    {
        cout << "No process data to save.\n";
        return;
    }
    string filename;
    cout << "Enter filename: ";
    cin >> filename;

    ofstream file(filename);
    for (const auto &p : processes)
    {
        file << p.arrival_time << " " << p.cpu_burst_time1 << " "
             << p.io_time << " " << p.cpu_burst_time2 << " " << p.priority << "\n";
    }
    cout << "Saved to " << filename << "\n";
}

// Load processes from file
vector<Process> loadProcessDataFromFile()
{
    vector<Process> processes;
    string filename;
    cout << "Enter filename: ";
    cin >> filename;

    ifstream file(filename);
    int id = 1, at, cpu1, io, cpu2, prio;
    while (file >> at >> cpu1 >> io >> cpu2 >> prio)
    {
        processes.push_back(Process("P" + to_string(id++), at, cpu1, io, cpu2, prio));
    }
    cout << processes.size() << " processes loaded.\n";
    return processes;
}

// Generate random processes
vector<Process> generateRandomProcesses()
{
    vector<Process> processes;
    int n;
    cout << "Enter number of processes: ";
    cin >> n;
    for (int i = 1; i <= n; i++)
    {
        processes.push_back(Process("P" + to_string(i),
                                    rand() % 20,
                                    1 + rand() % 10,
                                    rand() % 10,
                                    rand() % 10,
                                    1 + rand() % 10));
    }
    cout << n << " processes generated.\n";
    return processes;
}

// Display results using GranttAnalysis
void displayResults(const string &name, const vector<Process> &processes, const vector<ProcessGrantInfo> &chart)
{
    if (chart.empty())
    {
        cout << "No results.\n";
        return;
    }
    GranttAnalysis analysis(chart, processes);
    analysis.pretty_print(name);
}

// ================== Main ==================
int main()
{
    srand(time(nullptr));
    vector<Process> processes;
    int choice;
    bool exit_program = false;

    while (!exit_program)
    {
        clearScreen();
        displayMenu();
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
        { // FCFS
            if (processes.empty())
                break;
            FCFS fcfs(processes);
            displayResults("FCFS", processes, fcfs.cpu_process());
            cin.get();
            break;
        }
        case 2:
        { // SJF
            if (processes.empty())
                break;
            SJF sjf(processes);
            displayResults("SJF", processes, sjf.cpu_process());
            cin.get();
            break;
        }
        case 3:
        { // RR
            if (processes.empty())
                break;
            int quantum;
            cout << "Enter quantum: ";
            cin >> quantum;
            RoundRobin rr(processes);
            displayResults("Round Robin", processes, rr.cpu_process(quantum));
            cin.get();
            break;
        }
        case 4:
        { // MLFQ
            if (processes.empty())
                break;
            MLFQ mlfq(processes);
            displayResults("MLFQ", processes, mlfq.cpu_process());
            cin.get();
            break;
        }
        case 5:
        { // Priority Non-preemptive
            if (processes.empty())
                break;
            PriorityScheduler prio(processes, false); // use non-preemptive
            displayResults("Priority (Non-preemptive)", processes, prio.cpu_process());
            cin.get();
            break;
        }
        case 6:
        { // Priority Preemptive
            if (processes.empty())
                break;
            PriorityScheduler prio_p(processes, true); // use preemptive
            displayResults("Priority (Preemptive)", processes, prio_p.cpu_process());
            cin.get();
            break;
        }
        case 7:
        { // FCFS Preemptive
            if (processes.empty())
                break;
            FCFSPreemp fcfs_p(processes);
            displayResults("FCFS Preemptive", processes, fcfs_p.cpu_process());
            cin.get();
            break;
        }
        case 8:
            processes = getProcessData();
            cin.get();
            break;
        case 9:
            displayProcessData(processes);
            cin.get();
            break;
        case 10:
            saveProcessDataToFile(processes);
            cin.get();
            break;
        case 11:
            processes = loadProcessDataFromFile();
            cin.get();
            break;
        case 12:
            processes = generateRandomProcesses();
            cin.get();
            break;
        case 13:
            exit_program = true;
            break;
        default:
            cout << "Invalid choice.\n";
            cin.get();
            break;
        }
    }
    cout << "Thank you for using CPU Scheduler Simulator!\n";
    return 0;
}
