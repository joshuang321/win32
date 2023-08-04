#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int
main(int argc, char *argv[])
{
    if (argc==3)
    {
		HANDLE a = INVALID_HANDLE_VALUE;
		
        HANDLE hFile= CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_READONLY|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        HANDLE hOtherFile = CreateFileA(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE
            && hOtherFile != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER lgSzFile;
            GetFileSizeEx(hFile, &lgSzFile);
            if (lgSzFile.HighPart != 0)
            {
                printf("File too big to handle, too lazy to fix.\r\n");
                CloseHandle(hFile);
                CloseHandle(hOtherFile);
                DeleteFileA(argv[2]);
                return 0;
            }
            LPSTR lpBigMemBlock = VirtualAlloc(NULL, lgSzFile.LowPart, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            if (lpBigMemBlock)
            {
                DWORD dwOffset=0;
                DWORD dwnlOffset =0;
                DWORD dwBytesRead;
                DWORD dwWritten;
                DWORD dwToBeWritten;
                DWORD dwWrittenOffset=0;

                while (lgSzFile.LowPart > 0)
                {
                    if (ReadFile(hFile, lpBigMemBlock + dwOffset, lgSzFile.LowPart, &dwBytesRead, NULL))
                    {
                        lgSzFile.LowPart -= dwBytesRead;
                        dwOffset += dwBytesRead;
                    }
                }
                lgSzFile.LowPart = dwOffset;
                dwOffset =0;
                
                while (dwOffset < lgSzFile.LowPart
                    && lpBigMemBlock[dwOffset++] != '\n');
                dwToBeWritten = dwOffset-1;
                if (WriteFile(hOtherFile, lpBigMemBlock +dwWrittenOffset, dwToBeWritten, &dwWritten, NULL))
                {
                    dwToBeWritten -= dwWritten;
                    dwWrittenOffset += dwWritten;
                }
                dwnlOffset = dwOffset;

                #if 1
                while (dwOffset < lgSzFile.LowPart)
                {
                    while (dwOffset < lgSzFile.LowPart
                        && lpBigMemBlock[dwOffset++] != '\n');
                    if (dwOffset >= lgSzFile.LowPart)
                    {
                        break;
                    }

                    dwToBeWritten = dwOffset-dwnlOffset-1;
                    dwWrittenOffset=0;
                    while (dwToBeWritten > 0)
                    {
                        if (WriteFile(hOtherFile, lpBigMemBlock + dwnlOffset+dwWrittenOffset, dwToBeWritten, &dwWritten, NULL))
                        {
                            dwToBeWritten -= dwWritten;
                            dwWrittenOffset += dwWritten;
                        }
                    }
                    dwnlOffset = dwOffset;
                }
                #endif

                VirtualFree((LPVOID)lpBigMemBlock, 0, MEM_FREE);
                CloseHandle(hFile);
                CloseHandle(hOtherFile);
            }
        }
        else
        {
            printf("Failed to open file, file may not exist or file already exists: %d\r\nRead file handle value: %p\r\nWrite file handle value: %p\r\n",
                GetLastError(), (void*)hFile, (void*)hOtherFile);
        }
    }
}