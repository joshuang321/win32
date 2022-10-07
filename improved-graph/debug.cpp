#include "headers/debug.h"


static HANDLE hConsoleOutput = NULL;
static DWORD dwBytesWritten = 0;

const std::wstring lpszDebugTag = std::wstring(L"[DEBUG]: "),
	lpszErrorTag = std::wstring(L"[ERROR]: ");

void InitDebug() {

	if (AllocConsole()) {


			hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
			if (INVALID_HANDLE_VALUE != hConsoleOutput) {

				LogDebug(std::wstring(L"Debugging has Started!"));
				return;
			} 
	}
	MessageBoxW(NULL, L"Failed to Init Debug!", L"GraphDebug", MB_OK);
	ExitProcess(1);
}

void StopDebug() {

	FreeConsole();
}

static void LogConsoleMessage(const std::wstring lpszTag,
	std::wstring wstrMessage) {

	if (!wstrMessage.empty()) {
	
	wstrMessage += std::wstring(L"\r\n");
	WriteConsoleW(hConsoleOutput, lpszTag.c_str(), lpszTag.length(),
		&dwBytesWritten,
		NULL);
	WriteConsoleW(hConsoleOutput, (LPWSTR)wstrMessage.c_str(),
		wstrMessage.length(),
		&dwBytesWritten,
		NULL);
	}
}

void LogDebug(std::wstring wstrLogDebug) {
	
	LogConsoleMessage(lpszDebugTag, wstrLogDebug);
}

void LogFatal(std::wstring wstrLogFatal) {

	LogConsoleMessage(lpszErrorTag, wstrLogFatal);
	StopDebug();
	ExitProcess(2);
}