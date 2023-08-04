#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>

#define KB(szMem) szMem<<10
#define MB(szMem) szMem<<20

int
main(int argc, char *argv[])
{
    if (argc ==3
        || argc==4)
    {
        DWORD dwBytesToRead= MB(5);
        DWORD dwBytesRead;
        DWORD dwOffset =0;
        int i=0;

        if (argc == 4
            && argv[3][0] >= '1'
            && argv[3][0] <= '9')
        {
            size_t argvlen = strlen(argv[3]);
   
            if (argvlen >=3)
            {
                if (argv[3][argvlen-1] == 'b'
                    || argv[3][argvlen-1] == 'B')
                {
                    if (argv[3][argvlen-2] == 'M'
                        || argv[3][argvlen-2] == 'm')
                    {
                        argv[3][argvlen-2] = '\0';
                        i =atoi(argv[3]);
                        i = MB(i);
                    }
                    else if (argv[3][argvlen-2] == 'K'
                        || argv[3][argvlen-2] == 'k')
                    {
                        argv[3][argvlen-2] = '\0';
                        i = KB(atoi(argv[3]));
                    }
                }
                else
                {
                    i =atoi(argv[3]);
                }
            }
            else
            {
                i = atoi(argv[3]);
            }
        }
        if (i>0)
        {
            dwBytesToRead = (DWORD)i;
        }
        HANDLE hFile = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_READONLY|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        HANDLE hOtherFile = CreateFileA(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE
            && hOtherFile != INVALID_HANDLE_VALUE)
        {
            LPSTR lpBigMemBlock = (LPSTR)VirtualAlloc(NULL, dwBytesToRead, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            if (lpBigMemBlock)
            {
                while (dwBytesToRead > 0)
                {
                    if (ReadFile(hFile, lpBigMemBlock + dwOffset, dwBytesToRead, &dwBytesRead, NULL))
                    {
                        dwBytesToRead -= dwBytesRead;
                        dwOffset += dwBytesRead;
                    }
                }
                dwBytesToRead = dwOffset;
                dwOffset = 0;
                while (dwBytesToRead > 0)
                {
                    if (WriteFile(hOtherFile, lpBigMemBlock + dwOffset, dwBytesRead, &dwBytesRead, NULL))
                    {
                        dwBytesToRead -=dwBytesRead;
                        dwOffset += dwBytesRead;
                    }
                }
                VirtualFree((LPVOID)lpBigMemBlock, 0, MEM_FREE);
                CloseHandle(hFile);
                CloseHandle(hOtherFile);
            }
            else
            {
                printf("Failed to allocate mem for file.: %d\r\n", GetLastError());
                CloseHandle(hFile);
                CloseHandle(hOtherFile);
            }
        }
        else
        {
            printf("Failed to open file, file most likely does not exist or created file already exists: %d\r\nRead file handle value: %p\r\nWrite file handle value"
                " %d\r\n", GetLastError(), (void*)hFile, (void*)hOtherFile);
        }
    }
    return 0;
}