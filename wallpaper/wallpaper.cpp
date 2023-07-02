#include "worker.cpp"

#include <stdio.h>
#include <CommCtrl.h>
#include <objidl.h>
#include <Gdiplus.h>

static BOOL CALLBACK
EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    static HWND hWndChild;
    hWndChild = FindWindowExA(hWnd, NULL, "SHELLDLL_DefView", NULL);

    if (hWndChild) {
        *((HWND*)lParam) = FindWindowExA(NULL, hWnd, "WorkerW", NULL);
    }
    return TRUE;
}

static HWND
GetWorkerWindow(void)
{
    HWND hWnd = FindWindowA("Progman", NULL);
    HWND wallpaper_hwnd;
    if (hWnd)
    {
        SendMessageTimeoutA(hWnd, 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);
        EnumWindows(EnumWindowsProc, (LPARAM) &hWnd);
        return hWnd;
    }
    return 0L;
}

const char LPSZCLASSNAME[] = "BEHIND_WND",
    LPSZENDCLASSNAME[] = "END_WND";

HINSTANCE g_hInst;
RECT g_Rect;
wchar_t g_lpszWallpaperPath[MAX_PATH+1];
HANDLE g_hEvent;

static DWORD __stdcall
WallpaperThread(LPVOID lpTParam)
{
    MSG msg;
    MONITORINFO mi;
    const POINT pt = {0};
    Gdiplus::GdiplusStartupInput gdiplusInput;
    Gdiplus::GdiplusStartupOutput gdiplusOutput;
    ULONG_PTR uToken;
    GUID guID;
    int nFrameCount=0, iCurFrame=0, nWidthOffset, nHeightOffset;
    UINT uPropSz;
    Gdiplus::PropertyItem* pPropItem;
    bool isRunning = true;
    Gdiplus::Graphics* pGraphics;
    LARGE_INTEGER lgCurTick, lgPrevTick, lg;
    double dPrevTick, dFreq;

    HWND hWndWallpaper = CreateWindowExA(0, LPSZCLASSNAME, "Behind?", WS_VISIBLE|WS_CHILD, 0, 0, g_Rect.right, g_Rect.bottom, (HWND)lpTParam, NULL, g_hInst, NULL);

    if (hWndWallpaper)
    {
        Gdiplus::GdiplusStartup(&uToken, &gdiplusInput, &gdiplusOutput);
        Gdiplus::Image *pImage = new Gdiplus::Image(L"karen-kujou-karen.gif");
        
        if (pImage)
        {
            pImage->GetFrameDimensionsList(&guID, 1);
            nFrameCount = pImage->GetFrameCount(&guID);
            uPropSz = pImage->GetPropertyItemSize(PropertyTagFrameDelay);
            pPropItem = (Gdiplus::PropertyItem*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)uPropSz);
            pImage->GetPropertyItem(PropertyTagFrameDelay, uPropSz, pPropItem);
            pGraphics = new Gdiplus::Graphics(hWndWallpaper);

            nWidthOffset = (g_Rect.right - pImage->GetWidth()) >> 1;
            nHeightOffset = (g_Rect.bottom - pImage->GetHeight()) >> 1; 
            
            for (int i=0; i<nFrameCount; i++)
            {
                ((UINT*)pPropItem->value)[i] *= 10;
            }
            QueryPerformanceCounter(&lgPrevTick);
            dPrevTick = (double)lgPrevTick.QuadPart;
            QueryPerformanceFrequency(&lg);
            dFreq = (double)lg.QuadPart / 1000.0;
        }
        else
        {
            MessageBoxA(NULL, "Failed to load gif!", "wallpaper", MB_OK);
            SetEvent(g_hEvent);
            return 0;
        }



        while (isRunning)
        {
            if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == (WM_USER+1))
                {
                    delete pImage;
                    pImage = new Gdiplus::Image(g_lpszWallpaperPath);
                    pGraphics->DrawImage(pImage, 0, 0, g_Rect.right, g_Rect.bottom);
                    
                    isRunning = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            else
            {
                QueryPerformanceCounter(&lgCurTick);
                if (((double)lgCurTick.QuadPart - dPrevTick)/dFreq > (double)((UINT*)pPropItem->value)[iCurFrame])
                {
                    lgPrevTick.QuadPart = lgCurTick.QuadPart;
                    dPrevTick = (double)lgPrevTick.QuadPart;

                    pImage->SelectActiveFrame(&guID, iCurFrame);
                    pGraphics->DrawImage(pImage, nWidthOffset, nHeightOffset, pImage->GetWidth(), pImage->GetHeight());
                    iCurFrame = ++iCurFrame % nFrameCount;
                }
            }

        }
        
        delete pImage;
        delete pGraphics;
        DestroyWindow(hWndWallpaper);
        Gdiplus::GdiplusShutdown(uToken);
        SetEvent(g_hEvent);
    }
    return 0;
}

static LRESULT WINAPI MainWindowProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSA wc = {0};
    MSG msg;
    DWORD dwThreadId;

    SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH+1, g_lpszWallpaperPath, 0);
    SetProcessDPIAware();

    g_hInst = hInstance;

    wc.lpszClassName = LPSZCLASSNAME;
    wc.hCursor = LoadCursorA(hInstance, MAKEINTRESOURCEA(IDC_ARROW));
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_APPLICATION));
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    HWND hWndWorker = GetWorkerWindow();
    GetClientRect(hWndWorker, &g_Rect);
    g_hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);

    RegisterClassA(&wc);
    wc.lpszClassName = LPSZENDCLASSNAME;

    RegisterClassA(&wc);

    if (hWndWorker)
    {
        CreateThread(NULL, 0, WallpaperThread, (LPVOID)hWndWorker, 0, &dwThreadId);
        HWND hWnd = CreateWindowExA(0, LPSZENDCLASSNAME, "Close this window to close the wallpaper", WS_VISIBLE|WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 20, NULL,
            NULL, hInstance, NULL);
        
        while (GetMessageA(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        PostThreadMessageA(dwThreadId, WM_USER+1, 0L, 0L);

        if (WaitForSingleObject(g_hEvent, INFINITE) != WAIT_OBJECT_0)
        {
            MessageBoxA(NULL, "Wallpaper failed to clean up!", "wallpaper", MB_OK);
        }

        return msg.wParam;
    }
    else
    {
        MessageBoxA(NULL, "Failed to init!", "wallpaper", MB_OK);
    }
    return 1;
}

static LRESULT WINAPI
MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
    }

    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}   