#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

extern void InitDebug();
	extern void StopDebug();

extern void LogDebug(std::wstring);
	extern void LogFatal(std::wstring);