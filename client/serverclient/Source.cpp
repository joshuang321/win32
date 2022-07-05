#include "Header.h"

BOOL InstantiateServer(SERVERINSTANCE* pServInstance,
	PROCESS_INFORMATION* pprocInfo)
{
	if (nullptr == pServInstance ||
		nullptr == pprocInfo)
		return FALSE;

	SECURITY_ATTRIBUTES secAttr;
	HANDLE hChildIn, hChildOut;
	BOOL bSuccess = FALSE;
	STARTUPINFOW supInfo = { 0 };
	
	secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttr.lpSecurityDescriptor = NULL;
	secAttr.bInheritHandle = TRUE;

	if (CreatePipe(&pServInstance->hInputPipe, &hChildOut,
		&secAttr, 0))
	{
		if (CreatePipe(&hChildIn, &pServInstance->hOutputPipe,
			&secAttr, 0))
		{
			supInfo.cb = sizeof(STARTUPINFOW);
			supInfo.hStdError = hChildOut;
			supInfo.hStdOutput = hChildOut;
			supInfo.hStdInput = hChildIn;
			supInfo.dwFlags |= STARTF_USESTDHANDLES;

			bSuccess = CreateProcessW(L"server.exe",
				NULL, NULL, NULL, TRUE,
				0, NULL, NULL, &supInfo,
				pprocInfo);
			if (bSuccess)
				return TRUE;

			CloseHandle(hChildIn);
			CloseHandle(pServInstance->hOutputPipe);
		}
		CloseHandle(pServInstance->hInputPipe);
		CloseHandle(hChildOut);
	}

	return FALSE;
}

void WriteMessage(SERVERINSTANCE* pServInstance,
	LPWSTR lpwStr, int strLength)
{
	DWORD dwWritten;
	WriteFile(pServInstance->hOutputPipe, lpwStr, ((strLength+1) * sizeof(WCHAR)),
		&dwWritten, NULL);
}

void TerminateServer(SERVERINSTANCE* pServInstance)
{
	WriteMessage(pServInstance, (LPWSTR)L"\x1", 1);
}