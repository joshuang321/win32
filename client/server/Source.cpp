#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define BUFSZ	1024

int main(void)
{
	WCHAR chwBuf[BUFSZ];
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE),
		hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwRead;


	if (hInput &&
		hOutput)
	{
		while (true)
		{
			if (ReadFile(hInput, chwBuf, BUFSZ, &dwRead, NULL) &&
				0 != dwRead)
			{
				if (1 == *chwBuf)
					break;
				MessageBoxW(NULL, chwBuf, L"Child", MB_OK);
			}
		}
	}
	return 0;
}