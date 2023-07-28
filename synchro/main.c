#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
HANDLE g_hEvent;

UINT64 k=0;
#if defined (CRITSEC)
CRITICAL_SECTION criticalSection;
#endif
DWORD
WINAPI ThreadAddProc(LPVOID lParameter)
{
#if defined (CRITSEC)
    EnterCriticalSection(&criticalSection);
#endif
    UINT64 j=0;
    for (;
        j<10000000;
        j++)k++;
#if defined (CRITSEC)
    LeaveCriticalSection(&criticalSection);
#endif
    SetEvent(*((HANDLE*)lParameter));

    return 0;
}



int
main (int argc, char *argv[])
{
    HANDLE hEvents[16];
#if defined (CRITSEC)
    InitializeCriticalSection(&criticalSection);
#endif
    for (int i=0;
        i<16;
        i++)
    {
        hEvents[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
    }
    for (int i=0;
        i< 16;
        i++)
    {
        CreateThread(NULL, 0, ThreadAddProc, &hEvents[i], 0, NULL);
    }
    WaitForMultipleObjects(16, hEvents, TRUE, INFINITE);
    printf("%lld\r\n", k);
#if defined (CRITSEC)
    DeleteCriticalSection(&criticalSection);
#endif
    return 0;
}