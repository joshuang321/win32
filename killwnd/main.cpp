#include <Windows.h>
#include <iostream>
#include <cassert>

static DWORD dwOldConsoleMode{};

#if defined(DEBUG)
#define HANDLE_WIN32(statement) assert(statement)
#elif defined(NDEBUG)
#define HANDLE_WIN32(statement) Handle_Win32(statement)
#else
#define HANDLE_WIN32(statement) statement
#endif

static void _cdecl win32_atexit(void);
static void Handle_Win32(BOOL bSuccess);
static BOOL CALLBACK EnumWindowProc(HWND hWnd, LPARAM lParam);
static DWORD WINAPI DestroyWndThreadProc(_In_ LPVOID lParam);

int main(int argc, char* argv[])
{
    HANDLE_WIN32(GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwOldConsoleMode));
    HANDLE_WIN32(SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwOldConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING));
    assert(atexit(win32_atexit) == 0);

    if (argc < 3)
    {
        std::cout << "Usage: killwnd [options] [modifiers] [arguements]" << std::endl << std::endl
            << "Options:" << std::endl
            << "-h" << "\t\t\t" << "Shows the list of options, modifiers and required arguements." << std::endl
            << "-c [windowclassName]" << "\t\t\t" << "Specifies the class name of the windows. " << std::endl
            << "-w [windowName]" << "\t\t\t" << "Specifies the window name of the window." << std::endl << std::endl
            << "Modifiers" << std::endl
            << "-m" "\t\t\t" << "Specifies the window is message only.";
    }
    else
    {
        bool isMsg = argc > 3 && std::string_view("-m") == std::string_view(argv[2]);
        bool isClass = std::string_view("-c") == std::string_view(argv[1]);

        if (!isClass)
        {
            if (std::string_view("-w") != std::string_view(argv[1]))
            {
                std::cout << "\x1B[91m" << "Invalid Arguements! Please type \"killwnd -h\" for more options."
                    << "\x1B[0m" << std::endl;
                return -1;
            }
        }
        char *arg = isMsg ? argv[3] : argv[2];
        DWORD dwProcessId{},
            dwThreadId{};
        HANDLE hProcess{};


        HWND hWnd = FindWindowExA(isMsg ? HWND_MESSAGE : NULL, NULL, isClass ? arg : NULL, isClass ? NULL : arg);
        while (hWnd)
        {
            GetWindowThreadProcessId(hWnd, &dwProcessId);
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
            HANDLE_WIN32(hProcess != NULL);
            HANDLE_WIN32(CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)DestroyWndThreadProc, (LPVOID)hWnd, 0, &dwThreadId) != NULL);
            
            if (hProcess)
                CloseHandle(hProcess);
            hWnd = FindWindowExA(isMsg ? HWND_MESSAGE : NULL, hWnd, isClass ? arg : NULL, isClass ? NULL : arg);
        }
    }

    return 0;
}

static void _cdecl win32_atexit(void)
{
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwOldConsoleMode);
}

static void Handle_Win32(BOOL bSuccess)
{
    if (!bSuccess)
    {
        DWORD dwError = GetLastError();
        if (dwError)
            std::cout << "\x1B[91m" << "Win32 Error Code: " << "\x1B[0m" << dwError << std::endl;
    }
}

static DWORD WINAPI DestroyWndThreadProc(_In_ LPVOID lParam)
{
    DestroyWindow((HWND)lParam);
    return 0;
}