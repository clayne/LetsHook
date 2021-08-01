#include <pch.h>
#include <hook.h>
#include <breakpoint.h>
#include <string>
#include <pipe_client.h>
//tmp
#include <ram_assembly_finder.h>
#ifdef DBG
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#endif

const string version = "1.1", author = "h311d1n3r";

PipeClient* pipe = NULL;

#ifdef DBG
void RedirectIOToConsole() {
    AllocConsole();
    HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
    FILE* COutputHandle = _fdopen(SystemOutput, "w");
    HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
    int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
    FILE* CErrorHandle = _fdopen(SystemError, "w");
    HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
    FILE* CInputHandle = _fdopen(SystemInput, "r");
    std::ios::sync_with_stdio(true);
    freopen_s(&CInputHandle, "CONIN$", "r", stdin);
    freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
    freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}
#endif

void init() {
    #ifdef DBG
    RedirectIOToConsole();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 11);
    cout << "--- LetsHook : DEBUG ---" << endl;
    cout << " Version : " << version << endl;
    cout << " Author : " << author << endl;
    cout << "------------------------" << endl;
    SetConsoleTextAttribute(hConsole, 8);
    cout << endl;
    #endif
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
}

bool initPipe() {
    bool pipeSuccess;
    pipe = new PipeClient(pipeSuccess);
    if (pipeSuccess) {
        char msg[BUFF_LEN];
        int timeout = 0;
        while (pipe->readData(msg) <= 0) {
            Sleep(100);
            timeout++;
            if (timeout >= 20) {
                #ifdef DBG
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(hConsole, 12);
                cout << "Pipe synchronization failed due to timeout..." << endl;
                SetConsoleTextAttribute(hConsole, 8);
                #endif
                return false;
            }
        }
        if (msg) {
            if (msg[0] == SYNC) {
                const char sync[] = { SYNC };
                pipe->sendData((char*)sync, 1);
                return true;
            }
        }
    }
    else {
#ifdef DBG
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 12);
        cout << "An error occured while enabling the pipe..." << endl;
        SetConsoleTextAttribute(hConsole, 8);
#endif
    }
    return false;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        init();
        if (initPipe()) BreakpointInjector::pipe = pipe;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

