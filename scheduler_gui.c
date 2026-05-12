#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <ctype.h>

#define MAX 15

// GUI Controls
HWND hNumProc, hArrival, hBurst, hAlgo;
HWND hTableOut, hGanttOut, hStatsOut; 
HFONT hFont, hFontMono;

// Process Structure
typedef struct {
    int pid;
    int at;
    int bt;
    int wt;
    int tat;
    int rt;
    int completed;
} Process;

Process p[MAX];

// Function Prototypes
void RunScheduling(HWND hwnd);
void FCFS(int n);
void SJF(int n);
void SRTF(int n);

BOOL CALLBACK SetFontCallback(HWND child, LPARAM font) {
    SendMessage(child, WM_SETFONT, (WPARAM)font, TRUE);
    return TRUE;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
    case WM_CREATE:
        hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                           DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        
        hFontMono = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                               DEFAULT_PITCH | FF_MODERN, "Consolas");

        // UI Labels & Inputs
        CreateWindow("STATIC", "No. of Processes:", WS_VISIBLE | WS_CHILD, 20, 20, 150, 25, hwnd, NULL, NULL, NULL);
        hNumProc = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 180, 20, 100, 25, hwnd, NULL, NULL, NULL);

        CreateWindow("STATIC", "Arrival Times (e.g. 1,2):", WS_VISIBLE | WS_CHILD, 20, 60, 155, 25, hwnd, NULL, NULL, NULL);
        hArrival = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 180, 60, 250, 25, hwnd, NULL, NULL, NULL);

        CreateWindow("STATIC", "Burst Times (e.g. 1,2):", WS_VISIBLE | WS_CHILD, 20, 100, 155, 25, hwnd, NULL, NULL, NULL);
        hBurst = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 180, 100, 250, 25, hwnd, NULL, NULL, NULL);

        CreateWindow("STATIC", "Algorithm:", WS_VISIBLE | WS_CHILD, 20, 140, 100, 25, hwnd, NULL, NULL, NULL);
        hAlgo = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 120, 140, 180, 150, hwnd, NULL, NULL, NULL);
        SendMessage(hAlgo, CB_ADDSTRING, 0, (LPARAM)"FCFS");
        SendMessage(hAlgo, CB_ADDSTRING, 0, (LPARAM)"SJF (Non-Preemptive)");
        SendMessage(hAlgo, CB_ADDSTRING, 0, (LPARAM)"SRTF (Preemptive)");
        SendMessage(hAlgo, CB_SETCURSEL, 0, 0);

        // Buttons
        CreateWindow("BUTTON", "Run Analysis", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 20, 190, 110, 35, hwnd, (HMENU)1, NULL, NULL);
        CreateWindow("BUTTON", "Reset Fields", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 140, 190, 110, 35, hwnd, (HMENU)2, NULL, NULL);
        CreateWindow("BUTTON", "Clear Output", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 260, 190, 110, 35, hwnd, (HMENU)3, NULL, NULL);
        CreateWindow("BUTTON", "Exit", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 380, 190, 110, 35, hwnd, (HMENU)4, NULL, NULL);

        // Result Boxes
        hTableOut = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 20, 240, 560, 140, hwnd, NULL, NULL, NULL);
        hGanttOut = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY, 20, 390, 560, 60, hwnd, NULL, NULL, NULL);
        hStatsOut = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, 20, 460, 560, 35, hwnd, NULL, NULL, NULL);
        
        EnumChildWindows(hwnd, (WNDENUMPROC)SetFontCallback, (LPARAM)hFont);
        SendMessage(hTableOut, WM_SETFONT, (WPARAM)hFontMono, TRUE);
        SendMessage(hGanttOut, WM_SETFONT, (WPARAM)hFontMono, TRUE);
        SendMessage(hStatsOut, WM_SETFONT, (WPARAM)hFontMono, TRUE);
        break;

    case WM_COMMAND:
        switch(LOWORD(wp)) {
        case 1: RunScheduling(hwnd); break;
        case 2:
            SetWindowText(hNumProc, ""); SetWindowText(hArrival, ""); SetWindowText(hBurst, "");
            break;
        case 3: 
            SetWindowText(hTableOut, ""); SetWindowText(hGanttOut, ""); SetWindowText(hStatsOut, "");
            break;
        case 4: PostQuitMessage(0); break;
        }
        break;

    case WM_DESTROY:
        DeleteObject(hFont); DeleteObject(hFontMono);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

void RunScheduling(HWND hwnd) {
    char numStr[10], atStr[200], btStr[200];
    int n;

    GetWindowText(hNumProc, numStr, 10);
    GetWindowText(hArrival, atStr, 200);
    GetWindowText(hBurst, btStr, 200);

    // Empty field check
    if(strlen(numStr) == 0 || strlen(atStr) == 0 || strlen(btStr) == 0) {
        MessageBox(hwnd, "Please fill all fields first!", "Input Error", MB_ICONERROR);
        return;
    }

    // --- ENHANCED VALIDATION FOR ARRIVAL TIMES ---
    for (int i = 0; i < (int)strlen(atStr); i++) {
        if (!isdigit(atStr[i]) && atStr[i] != ',' && atStr[i] != ' ' && atStr[i] != '\r' && atStr[i] != '\n') {
            MessageBox(hwnd, "Invalid Character in Arrival Times! Only numbers and commas allowed.", "Data Type Error", MB_ICONERROR);
            return;
        }
    }

    // --- ENHANCED VALIDATION FOR BURST TIMES ---
    for (int i = 0; i < (int)strlen(btStr); i++) {
        if (!isdigit(btStr[i]) && btStr[i] != ',' && btStr[i] != ' ' && btStr[i] != '\r' && btStr[i] != '\n') {
            MessageBox(hwnd, "Invalid Character in Burst Times! Only numbers and commas allowed.", "Data Type Error", MB_ICONERROR);
            return;
        }
    }

    n = atoi(numStr);
    if (n <= 0 || n > MAX) {
        MessageBox(hwnd, "Please enter a valid number of processes (1-15).", "Range Error", MB_ICONERROR);
        return;
    }

    // Checking if count of values matches 'n'
    char tempAt[200], tempBt[200];
    strcpy(tempAt, atStr); strcpy(tempBt, btStr);
    int atCount = 0, btCount = 0;
    char *t = strtok(tempAt, ",");
    while(t) { atCount++; t = strtok(NULL, ","); }
    t = strtok(tempBt, ",");
    while(t) { btCount++; t = strtok(NULL, ","); }

    if (atCount != n || btCount != n) {
        MessageBox(hwnd, "Count of Arrival/Burst times does not match No. of Processes!", "Mismatch Error", MB_ICONERROR);
        return;
    }

    // Convert strings to process data
    char atCopy[200], btCopy[200];
    strcpy(atCopy, atStr); strcpy(btCopy, btStr);
    char *tok = strtok(atCopy, ",");
    for(int i=0; i<n; i++) { p[i].pid = i + 1; p[i].at = atoi(tok); tok = strtok(NULL, ","); }
    tok = strtok(btCopy, ",");
    for(int i=0; i<n; i++) { 
        p[i].bt = atoi(tok); 
        if(p[i].bt <= 0) {
            MessageBox(hwnd, "Burst Time must be greater than 0!", "Logic Error", MB_ICONERROR);
            return;
        }
        p[i].rt = p[i].bt; 
        p[i].completed = 0; 
        tok = strtok(NULL, ","); 
    }

    int sel = SendMessage(hAlgo, CB_GETCURSEL, 0, 0);
    if(sel == 0) FCFS(n);
    else if(sel == 1) SJF(n);
    else if(sel == 2) SRTF(n);
}

// ... FCFS, SJF, SRTF, and WinMain functions remain the same with their headings ...

void FCFS(int n) {
    char table[3000] = "FCFS TABLE:\r\nPID   AT   BT   WT   TAT\r\n--------------------------\r\n";
    char gantt[1000] = "GANTT CHART: | ";
    char stats[200] = "CALCULATIONS: ";
    int time = 0; float awt = 0, atat = 0;

    for(int i=0; i<n; i++) {
        if(time < p[i].at) time = p[i].at;
        p[i].wt = time - p[i].at;
        time += p[i].bt;
        p[i].tat = p[i].wt + p[i].bt;
        awt += p[i].wt; atat += p[i].tat;
        char row[100], g[20];
        sprintf(row, "P%d    %2d   %2d   %2d   %2d\r\n", p[i].pid, p[i].at, p[i].bt, p[i].wt, p[i].tat);
        strcat(table, row);
        sprintf(g, "P%d | ", p[i].pid); strcat(gantt, g);
    }
    char res[100]; sprintf(res, "Avg WT: %.2f | Avg TAT: %.2f", awt/n, atat/n);
    strcat(stats, res);
    SetWindowText(hTableOut, table); SetWindowText(hGanttOut, gantt); SetWindowText(hStatsOut, stats);
}

void SJF(int n) {
    char table[3000] = "SJF TABLE:\r\nPID   AT   BT   WT   TAT\r\n--------------------------\r\n";
    char gantt[1000] = "GANTT CHART: | ";
    char stats[200] = "CALCULATIONS: ";
    int comp = 0, time = 0; float awt = 0, atat = 0;

    while(comp < n) {
        int idx = -1, min = 9999;
        for(int i=0; i<n; i++) {
            if(p[i].at <= time && !p[i].completed && p[i].bt < min) { min = p[i].bt; idx = i; }
        }
        if(idx != -1) {
            p[idx].wt = time - p[idx].at;
            time += p[idx].bt;
            p[idx].tat = p[idx].wt + p[idx].bt;
            p[idx].completed = 1;
            awt += p[idx].wt; atat += p[idx].tat; comp++;
            char g[20]; sprintf(g, "P%d | ", p[idx].pid); strcat(gantt, g);
        } else time++;
    }
    for(int i=0; i<n; i++) {
        char row[100]; sprintf(row, "P%d    %2d   %2d   %2d   %2d\r\n", p[i].pid, p[i].at, p[i].bt, p[i].wt, p[i].tat);
        strcat(table, row);
    }
    char res[100]; sprintf(res, "Avg WT: %.2f | Avg TAT: %.2f", awt/n, atat/n);
    strcat(stats, res);
    SetWindowText(hTableOut, table); SetWindowText(hGanttOut, gantt); SetWindowText(hStatsOut, stats);
}

void SRTF(int n) {
    char table[3000] = "SRTF TABLE:\r\nPID   AT   BT   WT   TAT\r\n--------------------------\r\n";
    char gantt[2000] = "GANTT CHART: |";
    char stats[200] = "CALCULATIONS: ";
    int comp = 0, time = 0; float awt = 0, atat = 0;

    while(comp < n) {
        int idx = -1, min = 9999;
        for(int i=0; i<n; i++) {
            if(p[i].at <= time && !p[i].completed && p[i].rt < min && p[i].rt > 0) { min = p[i].rt; idx = i; }
        }
        if(idx != -1) {
            p[idx].rt--;
            char g[10]; sprintf(g, "P%d|", p[idx].pid); strcat(gantt, g);
            time++;
            if(p[idx].rt == 0) {
                p[idx].completed = 1; comp++;
                p[idx].tat = time - p[idx].at;
                p[idx].wt = p[idx].tat - p[idx].bt;
                awt += p[idx].wt; atat += p[idx].tat;
            }
        } else time++;
    }
    for(int i=0; i<n; i++) {
        char row[100]; sprintf(row, "P%d    %2d   %2d   %2d   %2d\r\n", p[i].pid, p[i].at, p[i].bt, p[i].wt, p[i].tat);
        strcat(table, row);
    }
    char res[100]; sprintf(res, "Avg WT: %.2f | Avg TAT: %.2f", awt/n, atat/n);
    strcat(stats, res);
    SetWindowText(hTableOut, table); SetWindowText(hGanttOut, gantt); SetWindowText(hStatsOut, stats);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR args, int ncmdshow) {
    (void)hPrev; (void)args; (void)ncmdshow;
    WNDCLASS wc = {0};
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = "SchedulerWindow";
    wc.lpfnWndProc = WindowProcedure;

    RegisterClass(&wc);
    CreateWindow("SchedulerWindow", "CPU Scheduling Simulator Pro", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 620, 580, NULL, NULL, hInst, NULL);

    MSG msg = {0};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}