#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>

int main(int argc, char **argv)
{
    MONITORINFO mi = { sizeof(MONITORINFO) };
    if (!GetMonitorInfoW(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY), &mi))
    {
        std::cout << "Failed to get monitor info!" << std::endl;
        return -1;
    }
    std::cout << "Width: "  << mi.rcMonitor.right << std::endl << "Height: " << mi.rcMonitor.bottom << std::endl; 
    return 0;
}