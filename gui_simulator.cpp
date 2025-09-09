#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstdlib>

#include "Process.h"
#include "ProcessGrantInfo.h"

// Include all scheduler implementations
#include "fcfs.cpp"  // Assuming this exists
#include "sjf.cpp"
#include "rr.cpp"
#include "mlfq.cpp"
#include "fcfsPreemp.cpp"

// Simple GUI implementation using Windows API
#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>

// Link with Comctl32.lib
#pragma comment(lib, "comctl32.lib")

// Global variables
HWND g_hwndMain = NULL;
HWND g_hwndProcessList = NULL;
HWND g_hwndAlgorithmCombo = NULL;
HWND g_hwndQuantumEdit = NULL;
HWND g_hwndCSEdit = NULL;
HWND g_hwndRunButton = NULL;
HWND g_hwndLoadButton = NULL;
HWND g_hwndRandomButton = NULL;
HWND g_hwndAddLevelButton = NULL;
HWND g_hwndRemoveLevelButton = NULL;
HWND g_hwndLevelsList = NULL;

std::vector<Process> g_processes;
std::vector<ProcessGrantInfo> g_ganttChart;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GanttChartProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void LoadProcessesFromFile(const std::string& filename);
void GenerateRandomProcesses(int count);
void RunSimulation();
void DisplayGanttChart(const std::vector<ProcessGrantInfo>& ganttChart);

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    // Register window class
    const char CLASS_NAME[] = "CPUSchedulerSimulatorClass";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);

    // Create the main window
    g_hwndMain = CreateWindowEx(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        "CPU Scheduler Simulator", // Window text
        WS_OVERLAPPEDWINDOW,       // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // Size and position
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (g_hwndMain == NULL) {
        return 0;
    }

    // Create controls
    CreateControls(g_hwndMain);

    // Show the window
    ShowWindow(g_hwndMain, nCmdShow);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1: // Run button
                    RunSimulation();
                    return 0;
                case 2: // Load button
                    {
                        char filename[MAX_PATH] = {0};
                        OPENFILENAME ofn = {0};
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hwnd;
                        ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0";
                        ofn.lpstrFile = filename;
                        ofn.nMaxFile = MAX_PATH;
                        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                        ofn.lpstrDefExt = "txt";

                        if (GetOpenFileName(&ofn)) {
                            LoadProcessesFromFile(filename);
                        }
                    }
                    return 0;
                case 3: // Random button
                    GenerateRandomProcesses(5); // Generate 5 random processes
                    return 0;
                case 4: // Add Level button
                    {
                        int count = ListBox_GetCount(g_hwndLevelsList);
                        if (count < 5) { // Limit to 5 levels
                            char buffer[256];
                            sprintf(buffer, "Level %d : Round Robin", count + 1);
                            ListBox_AddString(g_hwndLevelsList, buffer);
                        }
                    }
                    return 0;
                case 5: // Remove Level button
                    {
                        int count = ListBox_GetCount(g_hwndLevelsList);
                        if (count > 1) { // Keep at least one level
                            ListBox_DeleteString(g_hwndLevelsList, count - 1);
                        }
                    }
                    return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create all controls for the main window
void CreateControls(HWND hwnd) {
    // Process list (left side)
    g_hwndProcessList = CreateWindowEx(
        0, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
        20, 50, 300, 200,
        hwnd, NULL, NULL, NULL
    );
    SetWindowText(g_hwndProcessList, "6.08 2.5 1.1\r\n11.47 3.76 1.3\r\n6.82 1.58 1.1\r\n9.06 1.76 7.2\r\n10.48 2.39 4.3");

    // Algorithm selection combo box
    g_hwndAlgorithmCombo = CreateWindow(
        "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        350, 50, 150, 200,
        hwnd, NULL, NULL, NULL
    );
    ComboBox_AddString(g_hwndAlgorithmCombo, "Round Robin");
    ComboBox_AddString(g_hwndAlgorithmCombo, "Multi Level");
    ComboBox_AddString(g_hwndAlgorithmCombo, "SJF");
    ComboBox_AddString(g_hwndAlgorithmCombo, "FCFS");
    ComboBox_AddString(g_hwndAlgorithmCombo, "FCFS Preemptive");
    ComboBox_SetCurSel(g_hwndAlgorithmCombo, 0);

    // Simulation speed label
    CreateWindow(
        "STATIC", "Simulation Speed:",
        WS_CHILD | WS_VISIBLE,
        350, 90, 100, 20,
        hwnd, NULL, NULL, NULL
    );

    // Simulation speed edit
    CreateWindow(
        "EDIT", "1",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        450, 90, 50, 20,
        hwnd, NULL, NULL, NULL
    );

    // Context Switch label
    CreateWindow(
        "STATIC", "CS:",
        WS_CHILD | WS_VISIBLE,
        350, 120, 30, 20,
        hwnd, NULL, NULL, NULL
    );

    // Context Switch edit
    g_hwndCSEdit = CreateWindow(
        "EDIT", "0.4",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        380, 120, 50, 20,
        hwnd, NULL, NULL, NULL
    );

    // Quantum label
    CreateWindow(
        "STATIC", "Quantum:",
        WS_CHILD | WS_VISIBLE,
        350, 150, 60, 20,
        hwnd, NULL, NULL, NULL
    );

    // Quantum edit
    g_hwndQuantumEdit = CreateWindow(
        "EDIT", "1",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        410, 150, 50, 20,
        hwnd, NULL, NULL, NULL
    );

    // Run button
    g_hwndRunButton = CreateWindow(
        "BUTTON", "Run",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        350, 180, 100, 30,
        hwnd, (HMENU)1, NULL, NULL
    );

    // Load File button
    g_hwndLoadButton = CreateWindow(
        "BUTTON", "Load File",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 260, 100, 30,
        hwnd, (HMENU)2, NULL, NULL
    );

    // Random Input button
    g_hwndRandomButton = CreateWindow(
        "BUTTON", "Random Input",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, 260, 100, 30,
        hwnd, (HMENU)3, NULL, NULL
    );

    // Levels list
    g_hwndLevelsList = CreateWindow(
        "LISTBOX", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
        20, 300, 300, 100,
        hwnd, NULL, NULL, NULL
    );
    ListBox_AddString(g_hwndLevelsList, "Level 1 : Round Robin");
    ListBox_AddString(g_hwndLevelsList, "Level 2 : FCFS");
    ListBox_AddString(g_hwndLevelsList, "Level 3 : Round Robin");

    // Add Level button
    g_hwndAddLevelButton = CreateWindow(
        "BUTTON", "Add Level",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        330, 300, 100, 30,
        hwnd, (HMENU)4, NULL, NULL
    );

    // Remove Level button
    g_hwndRemoveLevelButton = CreateWindow(
        "BUTTON", "Remove Level",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        330, 340, 100, 30,
        hwnd, (HMENU)5, NULL, NULL
    );

    // Instructions
    CreateWindow(
        "STATIC", "Load From File or Write 'Random Processes'",
        WS_CHILD | WS_VISIBLE,
        20, 230, 300, 20,
        hwnd, NULL, NULL, NULL
    );
}

// Load processes from a file
void LoadProcessesFromFile(const std::string& filename) {
    g_processes.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        MessageBox(g_hwndMain, "Failed to open file", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::string line;
    int id = 1;
    std::stringstream processText;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        float arrival_time, cpu_burst1, io_time, cpu_burst2;
        
        if (iss >> arrival_time >> cpu_burst1 >> io_time) {
            // Optional second CPU burst
            if (!(iss >> cpu_burst2)) {
                cpu_burst2 = 0;
            }
            
            g_processes.push_back(Process("P" + std::to_string(id), 
                                        static_cast<int>(arrival_time), 
                                        static_cast<int>(cpu_burst1), 
                                        static_cast<int>(io_time), 
                                        static_cast<int>(cpu_burst2)));
            
            processText << arrival_time << " " << cpu_burst1 << " " << io_time;
            if (cpu_burst2 > 0) {
                processText << " " << cpu_burst2;
            }
            processText << "\r\n";
            
            id++;
        }
    }

    SetWindowText(g_hwndProcessList, processText.str().c_str());
}

// Generate random processes
void GenerateRandomProcesses(int count) {
    g_processes.clear();
    std::stringstream processText;

    for (int i = 1; i <= count; i++) {
        int arrival_time = rand() % 10;
        int cpu_burst1 = 1 + rand() % 10;
        int io_time = 1 + rand() % 5;
        int cpu_burst2 = rand() % 5; // 0-4, might be 0 (no second burst)

        g_processes.push_back(Process("P" + std::to_string(i), 
                                    arrival_time, 
                                    cpu_burst1, 
                                    io_time, 
                                    cpu_burst2));

        processText << arrival_time << " " << cpu_burst1 << " " << io_time;
        if (cpu_burst2 > 0) {
            processText << " " << cpu_burst2;
        }
        processText << "\r\n";
    }

    SetWindowText(g_hwndProcessList, processText.str().c_str());
}

// Run the selected scheduling algorithm
void RunSimulation() {
    // Parse processes from the text box
    g_processes.clear();
    char buffer[4096];
    GetWindowText(g_hwndProcessList, buffer, sizeof(buffer));
    
    std::istringstream iss(buffer);
    std::string line;
    int id = 1;
    
    while (std::getline(iss, line)) {
        std::istringstream line_iss(line);
        float arrival_time, cpu_burst1, io_time, cpu_burst2 = 0;
        
        if (line_iss >> arrival_time >> cpu_burst1 >> io_time) {
            // Optional second CPU burst
            if (!(line_iss >> cpu_burst2)) {
                cpu_burst2 = 0;
            }
            
            g_processes.push_back(Process("P" + std::to_string(id), 
                                        static_cast<int>(arrival_time), 
                                        static_cast<int>(cpu_burst1), 
                                        static_cast<int>(io_time), 
                                        static_cast<int>(cpu_burst2)));
            id++;
        }
    }
    
    if (g_processes.empty()) {
        MessageBox(g_hwndMain, "No valid processes found", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Get selected algorithm
    int algorithm = ComboBox_GetCurSel(g_hwndAlgorithmCombo);
    
    // Get quantum value for RR and MLFQ
    char quantumStr[32];
    GetWindowText(g_hwndQuantumEdit, quantumStr, sizeof(quantumStr));
    int quantum = atoi(quantumStr);
    if (quantum <= 0) quantum = 1;

    // Run the selected algorithm
    switch (algorithm) {
        case 0: // Round Robin
            {
                RoundRobin rr(g_processes);
                g_ganttChart = rr.cpu_process(quantum);
            }
            break;
        case 1: // Multi Level
            {
                MLFQ mlfq(g_processes);
                g_ganttChart = mlfq.cpu_process();
            }
            break;
        case 2: // SJF
            {
                SJF sjf(g_processes);
                g_ganttChart = sjf.cpu_process();
            }
            break;
        case 3: // FCFS
            {
                // Assuming FCFS class exists
                // FCFS fcfs(g_processes);
                // g_ganttChart = fcfs.cpu_process();
                MessageBox(g_hwndMain, "FCFS not implemented yet", "Info", MB_OK | MB_ICONINFORMATION);
                return;
            }
            break;
        case 4: // FCFS Preemptive
            {
                FCFSPreemp fcfsPreemp(g_processes);
                g_ganttChart = fcfsPreemp.cpu_process();
            }
            break;
    }

    // Display the Gantt chart in a new window
    DisplayGanttChart(g_ganttChart);
}

// Display Gantt chart in a new window
void DisplayGanttChart(const std::vector<ProcessGrantInfo>& ganttChart) {
    // Register window class for Gantt chart
    const char CLASS_NAME[] = "GanttChartClass";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = GanttChartProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);

    // Create the Gantt chart window
    HWND hwndGantt = CreateWindowEx(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        "Gantt Chart",             // Window text
        WS_OVERLAPPEDWINDOW,       // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, // Size and position
        NULL,       // Parent window    
        NULL,       // Menu
        GetModuleHandle(NULL),  // Instance handle
        NULL        // Additional application data
    );

    if (hwndGantt == NULL) {
        return;
    }

    // Store Gantt chart data as window property
    SetProp(hwndGantt, "GanttChart", (HANDLE)&g_ganttChart);

    // Show the window
    ShowWindow(hwndGantt, SW_SHOW);
}

// Window procedure for Gantt chart window
LRESULT CALLBACK GanttChartProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                // Get Gantt chart data
                std::vector<ProcessGrantInfo>* ganttChart = 
                    (std::vector<ProcessGrantInfo>*)GetProp(hwnd, "GanttChart");

                if (ganttChart && !ganttChart->empty()) {
                    // Find the maximum end time to scale the chart
                    int maxEndTime = 0;
                    for (const auto& info : *ganttChart) {
                        int endTime = std::max(info.cpu_end_time2 > 0 ? info.cpu_end_time2 : 
                                            (info.io_end_time > 0 ? info.io_end_time : info.cpu_end_time1), 0);
                        maxEndTime = std::max(maxEndTime, endTime);
                    }

                    // Get client area dimensions
                    RECT clientRect;
                    GetClientRect(hwnd, &clientRect);
                    int width = clientRect.right - clientRect.left - 40; // Leave margin
                    int height = clientRect.bottom - clientRect.top - 100; // Leave margin

                    // Calculate scaling factor
                    float timeScale = (float)width / maxEndTime;

                    // Draw timeline
                    MoveToEx(hdc, 20, height + 50, NULL);
                    LineTo(hdc, width + 20, height + 50);

                    // Draw time markers
                    for (int t = 0; t <= maxEndTime; t += 5) {
                        int x = 20 + (int)(t * timeScale);
                        MoveToEx(hdc, x, height + 45, NULL);
                        LineTo(hdc, x, height + 55);
                        
                        char timeStr[16];
                        sprintf(timeStr, "%d", t);
                        TextOut(hdc, x - 5, height + 60, timeStr, strlen(timeStr));
                    }

                    // Draw process bars
                    int processHeight = 20;
                    int processSpacing = 5;
                    int y = 30;

                    // First, collect all unique processes
                    std::vector<Process> uniqueProcesses;
                    for (const auto& info : *ganttChart) {
                        bool found = false;
                        for (const auto& p : uniqueProcesses) {
                            if (p.process_id == info.process.process_id) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            uniqueProcesses.push_back(info.process);
                        }
                    }

                    // Sort processes by ID for consistent display
                    std::sort(uniqueProcesses.begin(), uniqueProcesses.end(), 
                        [](const Process& a, const Process& b) {
                            return a.process_id < b.process_id;
                        });

                    // Draw process labels
                    for (size_t i = 0; i < uniqueProcesses.size(); i++) {
                        char processLabel[32];
                        sprintf(processLabel, "%s", uniqueProcesses[i].pid.c_str());
                        TextOut(hdc, 5, y + i * (processHeight + processSpacing) + 5, 
                                processLabel, strlen(processLabel));
                    }

                    // Draw Gantt chart bars
                    HBRUSH cpuBrush = CreateSolidBrush(RGB(0, 120, 215)); // Blue
                    HBRUSH ioBrush = CreateSolidBrush(RGB(255, 140, 0));  // Orange

                    for (const auto& info : *ganttChart) {
                        // Find the process index
                        int processIndex = 0;
                        for (size_t i = 0; i < uniqueProcesses.size(); i++) {
                            if (uniqueProcesses[i].process_id == info.process.process_id) {
                                processIndex = i;
                                break;
                            }
                        }

                        int processY = y + processIndex * (processHeight + processSpacing);

                        // Draw first CPU burst
                        if (info.cpu_start_time1 >= 0 && info.cpu_end_time1 > info.cpu_start_time1) {
                            int x1 = 20 + (int)(info.cpu_start_time1 * timeScale);
                            int x2 = 20 + (int)(info.cpu_end_time1 * timeScale);
                            RECT rect = {x1, processY, x2, processY + processHeight};
                            FillRect(hdc, &rect, cpuBrush);
                            FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                            
                            // Draw CPU burst time
                            char burstStr[16];
                            sprintf(burstStr, "%d", info.cpu_end_time1 - info.cpu_start_time1);
                            TextOut(hdc, (x1 + x2) / 2 - 5, processY + 5, burstStr, strlen(burstStr));
                        }

                        // Draw I/O burst
                        if (info.io_start_time >= 0 && info.io_end_time > info.io_start_time) {
                            int x1 = 20 + (int)(info.io_start_time * timeScale);
                            int x2 = 20 + (int)(info.io_end_time * timeScale);
                            RECT rect = {x1, processY, x2, processY + processHeight};
                            FillRect(hdc, &rect, ioBrush);
                            FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                            
                            // Draw I/O burst time
                            char burstStr[16];
                            sprintf(burstStr, "%d", info.io_end_time - info.io_start_time);
                            TextOut(hdc, (x1 + x2) / 2 - 5, processY + 5, burstStr, strlen(burstStr));
                        }

                        // Draw second CPU burst
                        if (info.cpu_start_time2 >= 0 && info.cpu_end_time2 > info.cpu_start_time2) {
                            int x1 = 20 + (int)(info.cpu_start_time2 * timeScale);
                            int x2 = 20 + (int)(info.cpu_end_time2 * timeScale);
                            RECT rect = {x1, processY, x2, processY + processHeight};
                            FillRect(hdc, &rect, cpuBrush);
                            FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                            
                            // Draw CPU burst time
                            char burstStr[16];
                            sprintf(burstStr, "%d", info.cpu_end_time2 - info.cpu_start_time2);
                            TextOut(hdc, (x1 + x2) / 2 - 5, processY + 5, burstStr, strlen(burstStr));
                        }
                    }

                    // Draw legend
                    RECT cpuRect = {20, 10, 40, 25};
                    FillRect(hdc, &cpuRect, cpuBrush);
                    FrameRect(hdc, &cpuRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                    TextOut(hdc, 45, 10, "CPU Burst", 9);

                    RECT ioRect = {120, 10, 140, 25};
                    FillRect(hdc, &ioRect, ioBrush);
                    FrameRect(hdc, &ioRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                    TextOut(hdc, 145, 10, "I/O Burst", 9);

                    // Calculate and display metrics
                    float avgWaitTime = 0;
                    float avgTurnaroundTime = 0;
                    float cpuUtilization = 0;
                    int totalCpuTime = 0;

                    for (const auto& info : *ganttChart) {
                        int waitTime = info.cpu_start_time1 - info.process.arrival_time;
                        if (info.cpu_start_time2 > 0 && info.io_end_time > 0) {
                            waitTime += info.cpu_start_time2 - info.io_end_time;
                        }

                        int turnaroundTime = 0;
                        if (info.cpu_end_time2 > 0) {
                            turnaroundTime = info.cpu_end_time2 - info.process.arrival_time;
                        } else if (info.io_end_time > 0) {
                            turnaroundTime = info.io_end_time - info.process.arrival_time;
                        } else {
                            turnaroundTime = info.cpu_end_time1 - info.process.arrival_time;
                        }

                        avgWaitTime += waitTime;
                        avgTurnaroundTime += turnaroundTime;

                        totalCpuTime += (info.cpu_end_time1 - info.cpu_start_time1);
                        if (info.cpu_end_time2 > 0 && info.cpu_start_time2 >= 0) {
                            totalCpuTime += (info.cpu_end_time2 - info.cpu_start_time2);
                        }
                    }

                    avgWaitTime /= uniqueProcesses.size();
                    avgTurnaroundTime /= uniqueProcesses.size();
                    cpuUtilization = (float)totalCpuTime / maxEndTime * 100;

                    // Display metrics
                    char metricsStr[256];
                    sprintf(metricsStr, "Report:\nAWT: %.1f\nATT: %.1f\nCPU Util: %.1f%%", 
                            avgWaitTime, avgTurnaroundTime, cpuUtilization);
                    RECT metricsRect = {width - 150, height - 80, width, height};
                    DrawText(hdc, metricsStr, -1, &metricsRect, DT_LEFT);

                    // Clean up
                    DeleteObject(cpuBrush);
                    DeleteObject(ioBrush);
                }

                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) { // Back button
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        case WM_CREATE:
            // Add a Back button
            CreateWindow(
                "BUTTON", "Back",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 10, 80, 30,
                hwnd, (HMENU)1, NULL, NULL
            );
            return 0;

        case WM_DESTROY:
            RemoveProp(hwnd, "GanttChart");
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#else
// Non-Windows implementation (placeholder)
int main() {
    std::cout << "GUI is only implemented for Windows" << std::endl;
    return 0;
}
#endif