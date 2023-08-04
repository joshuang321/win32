#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdio.h>

#define KB(szMem)       (szMem<<10)
#define MB(szMem)       (szMem<<20)
#define GB(szMem)       (((LONGLONG)szMem)<<30)

#define HISTORY_BUF_SZ  ((MB(64)>>3)/3)
#define PAD_SZ          MB(64)

#define LINE_BUF_SZ     MB(24)
#define CHAR_BUF_SZ     (LINE_BUF_SZ>>2)
#define LOAD_BUF_SZ     MB(4)

#define GP_PAD_SZ       (PAD_SZ-LINE_BUF_SZ-CHAR_BUF_SZ-LOAD_BUF_SZ)


#define LINE_BUF_CN     ((LINE_BUF_SZ/sizeof(UINT32))+1)

#define CHAR_RANGE      (0xFF-0x1F)

#define WM_CANTHANDLEFILE   WM_USER+0x1
#define WM_LOADFILECOMPLETE WM_USER+0x2
#define WM_INTERNAL_QUIT    WM_USER

#define SCROLL_WIDTH            10
#define SCROLLPELLET_BREADTH    12

#define MAX_WIDTH   400
#define MAX_HEIGHT  300

#if defined (__CHECKVOLATILE)
#define SHARED  volatile
#else
#define SHARED
#endif

typedef
struct HistoryBuffer
{
    LONGLONG szFileOffset;
    UINT64 nStartPosition;
    UINT64 nEndPosition;
} HistoryBuffer;

typedef
struct TextScroll
{
    union
    {
        struct
        {
            union
            {
                struct
                {

                    UINT32 nWindowLineStartOffset;
                    UINT32 nWindowLineEndOffset;
                    UINT32 nWidth;
                    UINT32 nHeight;
                    
                    UINT64 nMaxWidth;
                    UINT64 nMaxHeight;
                    UINT64 nScrollWidth;
                    UINT64 nScrollHeight;
                    UINT64 nScrollXPos;
                    UINT64 nScrollYPos;

                    int nScrollPixelXPos;
                    int nScrollPixelYPos;

                    RECT rcTextScroll;
                    RECT rcVertScroll;
                    RECT rcHortzScroll;
                    RECT rcVertScrollPellet;
                    RECT rcHortzScrollPellet;
                    RECT rcOldClip;

                    UINT64 nHistories;
                    UINT32 nHistoryStartIndex;
                    UINT32 nHistoryEndIndex;

                    UINT64 nStartLineOffset;
                    UINT32 nLineStartIndex;
                    UINT32 nLineEndIndex;
                    UINT32 nCharacterStartIndex;
                    UINT32 nCharacterEndIndex;

                    HWND hWnd;
                    HDC hDCMem;
                    
                    HGDIOBJ hGdiOldBmpMem;
                    HGDIOBJ hGdiOldFont;

                    LOGFONTA lgFont;
                    
                    HBRUSH hScrollBkgndBrush;
                    HBRUSH hScrollHotBkgndBrush;
                    HBRUSH hScrollFkgndBrush;
                    HBRUSH hScrollHotFkgndBrush;
                    HBRUSH hScrollActiveFkgndBrush;

                    BOOL isVertScrollHot;
                    BOOL isVertScrollPelletHot;
                    BOOL isVertScrollPelletActive;

                    BOOL isHortzScrollHot;
                    BOOL isHortzScrollPelletHot;
                    BOOL isHortzScrollPelletActive;

                    BOOL isOverHeight;
                    BOOL isOverWidth;
                    BOOL isMouseDown;

                    int nRenderVertTextScrollOffset;
                    int nRenderHortzTextScrollOffset;
                    POINT ptPrevMouse;

                    DWORD dwMouseScrollDelta;

                    HANDLE hFile;
                    LARGE_INTEGER lgFileSize;

                    DWORD dwComputeThread;
                    DWORD dwRenderThread;
                    HANDLE hWaitQuitComputeThreadEvent;
                    HANDLE hWaitQuitRenderThreadEvent;

                    HANDLE hWaitOnLoadRenderEvent;
                    
                    unsigned short nTextHeight;
                    short abcWidths[CHAR_RANGE];


                    char filePath[MAX_PATH+1];
                };
                char padding[GP_PAD_SZ];
            };
            UINT32 lineBuffer[LINE_BUF_CN];
            unsigned char loadFileBuffer[LOAD_BUF_SZ];
            unsigned char characterBuffer[CHAR_BUF_SZ];
        };
        char spacing[PAD_SZ];
    };
    HistoryBuffer historyBuffer[HISTORY_BUF_SZ];
#if defined (__DEBUG)
#define DEBUG_STRING_SZ     MB(1)
    char lpDebugStrBuf[DEBUG_STRING_SZ];
#endif
} TextScroll;

#if defined (__DEBUG)
char *lpDebugStrBuf = NULL;
void
DebugString(const char *sFormat, ...)
{
    if (lpDebugStrBuf)
    {
        va_list args;
        va_start(args, sFormat);
        vsnprintf(lpDebugStrBuf, DEBUG_STRING_SZ, sFormat, args);
        printf("\x1B[35mDEBUG:\x1B[0m%s\r\n", lpDebugStrBuf);
    }
}

#define DEBUG_STRING(sFormat, ...) DebugString(sFormat, __VA_ARGS__)
#else
#define DEBUG_STRING(sFormat, ...)  1
#endif

#define CHECKIF_DELETEGDIOBJ(hObj)  \
if (hObj)                           \
{                                   \
    DeleteObject(hObj);             \
    hObj = 0L;                      \
}

BOOL
InitializeBackBuffer(TextScroll *pTextScroll)
{
    BOOL isInit = FALSE;
    TEXTMETRICA tm;
    HDC hDCWnd = GetDC(pTextScroll->hWnd);
    
    LOGFONTA lgDefaultFont =
    {
        .lfHeight =-MulDiv(8, GetDeviceCaps(hDCWnd, LOGPIXELSY),72),
        .lfWeight = FW_BOLD,
        .lfCharSet = ANSI_CHARSET,
        .lfOutPrecision =OUT_DEFAULT_PRECIS,
        .lfClipPrecision = CLIP_DEFAULT_PRECIS,
        .lfQuality = DEFAULT_QUALITY,
        .lfPitchAndFamily = FIXED_PITCH,
        .lfFaceName = "Courier New"
    };
    pTextScroll->lgFont = lgDefaultFont;
    if (hDCWnd)
    {
        pTextScroll->hDCMem = CreateCompatibleDC(hDCWnd);
        if (pTextScroll->hDCMem)
        {
            pTextScroll->hGdiOldBmpMem = CreateCompatibleBitmap(hDCWnd, pTextScroll->nWidth, pTextScroll->nHeight);
            if (pTextScroll->hGdiOldBmpMem)
            {
                pTextScroll->hGdiOldBmpMem = SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldBmpMem);
                pTextScroll->hGdiOldFont = CreateFontIndirectA(&lgDefaultFont);
                if (pTextScroll->hGdiOldFont)
                {
                    pTextScroll->hGdiOldFont = SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldFont);
                    GetTextMetricsA(pTextScroll->hDCMem, &tm);
                    pTextScroll->nTextHeight = tm.tmHeight;

                    pTextScroll->hScrollBkgndBrush =CreateSolidBrush(RGB(220,220,220));
                    pTextScroll->hScrollHotBkgndBrush = CreateSolidBrush(RGB(211,211,211));
                    pTextScroll->hScrollFkgndBrush =CreateSolidBrush(RGB(128,128,128));
                    pTextScroll->hScrollHotFkgndBrush = CreateSolidBrush(RGB(105, 105, 105));
                    pTextScroll->hScrollActiveFkgndBrush= CreateSolidBrush(RGB(82,82,82));

                    isInit = TRUE;
                }
            }
        }
        ReleaseDC(pTextScroll->hWnd, hDCWnd);
        if (isInit)
        {
            goto IsSuccess;
        }
    }
    if (pTextScroll->hGdiOldBmpMem)
    {
        DeleteObject(SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldBmpMem));
        pTextScroll->hGdiOldBmpMem =0L;
    }
    if (pTextScroll->hGdiOldFont)
    {
        DeleteObject(SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldFont));
        pTextScroll->hGdiOldFont =0L;
    }
    if (pTextScroll->hDCMem)
    {
        DeleteDC(pTextScroll->hDCMem);
        pTextScroll->hDCMem = 0L;
    }
    CHECKIF_DELETEGDIOBJ(pTextScroll->hScrollBkgndBrush);
    CHECKIF_DELETEGDIOBJ(pTextScroll->hScrollHotBkgndBrush);
    CHECKIF_DELETEGDIOBJ(pTextScroll->hScrollFkgndBrush);
    CHECKIF_DELETEGDIOBJ(pTextScroll->hScrollActiveFkgndBrush);

IsSuccess:

    return isInit;
}

void
ResizeBackBuffer(TextScroll *pTextScroll)
{
    HDC hDCWnd = GetDC(pTextScroll->hWnd);
    if (hDCWnd)
    {
        HDC hDCNewMem = CreateCompatibleDC(hDCWnd);
        HBITMAP hBmpNewMem = CreateCompatibleBitmap(hDCWnd, pTextScroll->nWidth, pTextScroll->nHeight);

        if (hBmpNewMem
            && hDCNewMem)
        {
            DeleteObject(SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldBmpMem));
            pTextScroll->hGdiOldBmpMem = SelectObject(hDCNewMem, hBmpNewMem);
            HGDIOBJ hTemp = SelectObject(hDCNewMem, SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldFont));
            DeleteDC(pTextScroll->hDCMem);
            pTextScroll->hGdiOldFont = hTemp;
            pTextScroll->hDCMem = hDCNewMem;
        }
        ReleaseDC(pTextScroll->hWnd, hDCWnd);
    }
}

void
ReleaseBackBuffer(TextScroll *pTextScroll)
{
    DeleteObject(SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldBmpMem));
    DeleteObject(SelectObject(pTextScroll->hDCMem, pTextScroll->hGdiOldFont));
    DeleteDC(pTextScroll->hDCMem);
    
    DeleteObject(pTextScroll->hScrollBkgndBrush);
    DeleteObject(pTextScroll->hScrollHotBkgndBrush);
    DeleteObject(pTextScroll->hScrollFkgndBrush);
    DeleteObject(pTextScroll->hScrollActiveFkgndBrush);
}

#define IDM_LOADFILE    0x101
#define IDM_CHOOSEFONT  0x102

#define CALC_RECT_W(rcClient) (rcClient.right-rcClient.left)
#define CALC_RECT_H(rcClient) (rcClient.bottom-rcClient.top)

void
RenderThread_CopyToWindow(SHARED TextScroll *pTextScroll)
{
    HDC hDCWnd= GetDC(pTextScroll->hWnd);
    if (hDCWnd)
    {
        DEBUG_STRING("Mem copied to window.");
        BitBlt(hDCWnd, 0, 0, pTextScroll->nWidth, pTextScroll->nHeight, pTextScroll->hDCMem, 0, 0, SRCCOPY);
        ReleaseDC(pTextScroll->hWnd, hDCWnd);
    }
}

void
RenderThread_CopyScrollToWindow(SHARED TextScroll* pTextScroll)
{
    HDC hDCWnd = GetDC(pTextScroll->hWnd);
    if (hDCWnd)
    {
        BitBlt(hDCWnd, pTextScroll->rcHortzScroll.left, pTextScroll->rcHortzScroll.top, pTextScroll->nWidth, SCROLL_WIDTH, pTextScroll->hDCMem,
            pTextScroll->rcHortzScroll.left, pTextScroll->rcHortzScroll.top, SRCCOPY);
        BitBlt(hDCWnd, pTextScroll->rcVertScroll.left, pTextScroll->rcVertScroll.top, SCROLL_WIDTH, pTextScroll->nHeight -SCROLL_WIDTH, pTextScroll->hDCMem,
            pTextScroll->rcVertScroll.left, pTextScroll->rcVertScroll.top, SRCCOPY);

        ReleaseDC(pTextScroll->hWnd, hDCWnd);
    }
}

void inline
RenderThread_DrawLine(SHARED TextScroll *pTextScroll, WPARAM wParam)
{

    TextOutA(pTextScroll->hDCMem, -pTextScroll->nScrollXPos, wParam * pTextScroll->nTextHeight -pTextScroll->nRenderVertTextScrollOffset, (LPCSTR)(pTextScroll->characterBuffer + pTextScroll->lineBuffer[wParam]),
        pTextScroll->lineBuffer[wParam+1] - pTextScroll->lineBuffer[wParam]);
}

void inline
RenderThread_DrawHorzVertScroll(SHARED TextScroll *pTextScroll)
{
    if (pTextScroll->nScrollWidth)
    {
        FillRect(pTextScroll->hDCMem, &pTextScroll->rcHortzScroll, pTextScroll->isHortzScrollHot ? pTextScroll->hScrollHotBkgndBrush :
            pTextScroll->hScrollBkgndBrush);
        FillRect(pTextScroll->hDCMem, &pTextScroll->rcHortzScrollPellet, pTextScroll->isHortzScrollPelletActive ? pTextScroll->hScrollActiveFkgndBrush :
            (pTextScroll->isHortzScrollPelletHot ? pTextScroll->hScrollHotFkgndBrush : pTextScroll->hScrollFkgndBrush));
    }
    if (pTextScroll->nScrollHeight)
    {
        FillRect(pTextScroll->hDCMem, &pTextScroll->rcVertScroll, pTextScroll->isVertScrollHot ? pTextScroll->hScrollHotBkgndBrush :
            pTextScroll->hScrollBkgndBrush);
        FillRect(pTextScroll->hDCMem, &pTextScroll->rcVertScrollPellet, pTextScroll->isVertScrollPelletActive ? pTextScroll->hScrollActiveFkgndBrush :
            (pTextScroll->isVertScrollPelletHot ? pTextScroll->hScrollHotFkgndBrush : pTextScroll->hScrollFkgndBrush));
    } 
}

void
RenderThread_DrawTextScroll(SHARED TextScroll *pTextScroll)
{
    PatBlt(pTextScroll->hDCMem, 0, 0, pTextScroll->nWidth, pTextScroll->nHeight, WHITENESS);

    DEBUG_STRING("nWindowLineStartOffset: %d",pTextScroll->nWindowLineStartOffset);
    DEBUG_STRING("nWindowLineEndOffset: %d", pTextScroll->nWindowLineEndOffset);

    for (UINT32 i=pTextScroll->nWindowLineStartOffset, n=0;
        i<pTextScroll->nWindowLineEndOffset;
        i++, n++)
    {
        if (!TextOutA(pTextScroll->hDCMem, -pTextScroll->nScrollXPos, n * pTextScroll->nTextHeight - pTextScroll->nRenderVertTextScrollOffset, (LPCSTR)(pTextScroll->characterBuffer
            + pTextScroll->lineBuffer[i]), pTextScroll->lineBuffer[i+1] - pTextScroll->lineBuffer[i]))
        {
            DEBUG_STRING("TexOutA return false!");
        }
    }
    RenderThread_DrawHorzVertScroll(pTextScroll);
    RenderThread_CopyToWindow(pTextScroll);
}

#define WM_INTERNAL_SHARED   WM_USER+0x200

#define WM_INTERNAL_RENDER_ONLOADRENDER         WM_USER+0x1
#define WM_INTERNAL_RENDER_RENDERREADY          WM_USER+0x2
#define WM_INTERNAL_RENDER_RENDERSTOP           WM_USER+0x3
#define WM_INTERNAL_RENDER_REDRAW               WM_USER+0x4
#define WM_INTERNAL_RENDER_SCROLLRENDER         WM_USER+0x5
#define WM_INTERNAL_SHARED_LOADRENDERFINISHED   WM_INTERNAL_SHARED+0x1

DWORD WINAPI
WindowRenderThreadProc(LPVOID lpParameter)
{
    MSG msg;
    SHARED TextScroll *pTextScroll = (TextScroll *)lpParameter;
    LARGE_INTEGER lgCurTick;
    LARGE_INTEGER lgPrevTick;
    LARGE_INTEGER lg;
    double dPrevTick;
    double dFreq;
    BOOL isRunning =TRUE;

    QueryPerformanceCounter(&lgPrevTick);
    dPrevTick = (double)lgPrevTick.QuadPart;
    QueryPerformanceFrequency(&lg);
    dFreq = (double)lg.QuadPart / 1000.0;

    PeekMessageA(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    while (GetMessageA(&msg, NULL, 0, 0))
    {
        if (WM_INTERNAL_RENDER_ONLOADRENDER == msg.message)
        {
            DEBUG_STRING("WM_INTERNAL_RENDER_ONLOADRENDER");
            SetEvent(pTextScroll->hWaitOnLoadRenderEvent);
            PatBlt(pTextScroll->hDCMem, 0, 0, pTextScroll->nWidth, pTextScroll->nHeight, WHITENESS);
            
            while (GetMessageA(&msg, NULL, 0, 0))
            {
                // wait for messages from compute thread
                if (WM_INTERNAL_SHARED_LOADRENDERFINISHED == msg.message)
                {
                    break;
                }
                DEBUG_STRING(".wParam = %lld\t.lParam = %lld", msg.wParam, msg.lParam);
                RenderThread_DrawLine(pTextScroll, msg.wParam);
            }
            RenderThread_DrawHorzVertScroll(pTextScroll);
            RenderThread_CopyToWindow(pTextScroll);
                
        }
        else if (WM_INTERNAL_RENDER_RENDERREADY == msg.message)
        {
            DEBUG_STRING("WM_INTERNAL_RENDER_RENDERSTOP");
            while (isRunning)
            {
                if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_INTERNAL_RENDER_RENDERSTOP)
                    {
                        isRunning=FALSE;
                        break;
                    }
                }
                else
                {
                    #define TICK_RATE   16.0

                    QueryPerformanceCounter(&lgCurTick);
                    if ((((double)lgCurTick.QuadPart - dPrevTick)/dFreq) > TICK_RATE)
                    {
                        lgPrevTick.QuadPart = lgCurTick.QuadPart;
                        dPrevTick = (double)lgPrevTick.QuadPart;
                        RenderThread_DrawTextScroll(pTextScroll);
                    }

                }
            }
            isRunning = TRUE;
        }
        else if (WM_INTERNAL_RENDER_SCROLLRENDER == msg.message)
        {
            RenderThread_DrawHorzVertScroll(pTextScroll);
            RenderThread_CopyScrollToWindow(pTextScroll);
        }
        else if (WM_INTERNAL_RENDER_REDRAW == msg.message)
        {
            RenderThread_DrawTextScroll(pTextScroll);
        }
        else if (WM_INTERNAL_QUIT == msg.message)
        {
            break;
        }
    }
    SetEvent(pTextScroll->hWaitQuitRenderThreadEvent);
    return 0;
}

void inline
ComputeThread_LoadFile(SHARED TextScroll *pTextScroll)
{
    HANDLE hFile = CreateFileA(pTextScroll->filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesToBeRead = CHAR_BUF_SZ;
        DWORD dwOffset =0;
        DWORD dwBufferOffset =0;
        DWORD dwBytesRead;
        DWORD i;

        short nCurrentWidth=0;
        unsigned short nStartWidth=0;
        unsigned int nLinesCompute = 0;
        unsigned int nLinesToCompute = ((int)(pTextScroll->nHeight/pTextScroll->nTextHeight)) +1;

        unsigned int nTextLargestWidth =0;
        unsigned int nTempWidth=0;

        BOOL isWindowStillLoading =TRUE;
        BOOL isSmallerThanBuffer;

        LONGLONG llBytesToBeRead,
            llOffset;


        GetFileSizeEx(hFile,&(pTextScroll->lgFileSize));
        // 2|8 << 30
        // 1 <<31 | 1 << 33

        isSmallerThanBuffer = pTextScroll->lgFileSize.HighPart == 0 &&
            dwBytesToBeRead > pTextScroll->lgFileSize.LowPart;

        DEBUG_STRING("isSmallerThanBuffer: %d", isSmallerThanBuffer);
        if (isSmallerThanBuffer)
        {
            dwBytesToBeRead = pTextScroll->lgFileSize.LowPart;
        }

        pTextScroll->nRenderVertTextScrollOffset = 0;
        pTextScroll->nRenderHortzTextScrollOffset = 0;
        pTextScroll->nWindowLineStartOffset = 0;
        pTextScroll->nWindowLineEndOffset = 0;

        pTextScroll->nCharacterStartIndex = 0;
        pTextScroll->nCharacterEndIndex = dwBytesToBeRead-1;

        pTextScroll->nStartLineOffset =0;
        pTextScroll->nLineStartIndex = 0;
        pTextScroll->nLineEndIndex = 1;
        
        pTextScroll->lineBuffer[0] = 0;
        pTextScroll->lineBuffer[1] = 0;

        pTextScroll->nHistories = 0;
        pTextScroll->nHistoryStartIndex = 0;
        pTextScroll->nHistoryEndIndex = 0;
        pTextScroll->historyBuffer[0].nStartPosition=1;
        pTextScroll->historyBuffer[0].nEndPosition =1;
        pTextScroll->historyBuffer[0].szFileOffset=0;
        DEBUG_STRING("dwBytesToBeRead: %d", dwBytesToBeRead);

        if (pTextScroll->lgFileSize.QuadPart >= MB(6))
        {
            DEBUG_STRING("lgFileSize >= GB(10)");
            PostThreadMessageA(pTextScroll->dwComputeThread, WM_INTERNAL_SHARED_LOADRENDERFINISHED, 0L, 0L);
            PostMessageA(pTextScroll->hWnd, WM_CANTHANDLEFILE, 0L, 0L);
            return;
        }
        #if 0
        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED_LOADRENDERFINISHED, (WPARAM)0, (LPARAM)0);
        #else
        while (dwBytesToBeRead > 0)
        {
            DEBUG_STRING("%d", dwBytesToBeRead);
            if (ReadFile(hFile, pTextScroll->characterBuffer + dwOffset, dwBytesToBeRead, &dwBytesRead, NULL))
            {
                dwBytesToBeRead -= dwBytesRead;
                for (i=0;
                    i<dwBytesRead;
                    i++)
                {
                    if (pTextScroll->characterBuffer[i+dwOffset]  == '\r'
                        || pTextScroll->characterBuffer[i+dwOffset] =='\n')
                    {
                        if (pTextScroll->characterBuffer[i+dwOffset] == '\r')
                        {
                            i++;
                        }
                        pTextScroll->nCharacterEndIndex = i+dwOffset +1;
                        if (nTempWidth > nTextLargestWidth)
                        {
                            nTextLargestWidth = nTempWidth;
                        }
                        pTextScroll->lineBuffer[pTextScroll->nLineEndIndex] = i+dwOffset;
                        if (((dwOffset+i) - pTextScroll->historyBuffer[pTextScroll->nHistories].szFileOffset)
                            >= KB(4))
                        {
                            pTextScroll->nHistoryEndIndex = pTextScroll->nHistories;
                            pTextScroll->nHistories++;
                            pTextScroll->historyBuffer[pTextScroll->nHistories].nStartPosition=  pTextScroll->historyBuffer[pTextScroll->nHistories-1].nEndPosition+1;
                            pTextScroll->historyBuffer[pTextScroll->nHistories].nEndPosition= pTextScroll->historyBuffer[pTextScroll->nHistories].nStartPosition;
                            pTextScroll->historyBuffer[pTextScroll->nHistories].szFileOffset = dwOffset+i+1;
                        }
                        else
                        {
                            pTextScroll->historyBuffer[pTextScroll->nHistories].nEndPosition++;
                        }

                        if (isWindowStillLoading)
                        {
                            pTextScroll->nWindowLineEndOffset++;
                            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED, (WPARAM)pTextScroll->nLineEndIndex-1, 0L);
                        }
                        pTextScroll->lineBuffer[++(pTextScroll->nLineEndIndex)] = i+dwOffset+1;
                        nLinesCompute++;
                        if (nLinesCompute == nLinesToCompute)
                        {
                            isWindowStillLoading = FALSE;
                            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED_LOADRENDERFINISHED, 0L, 0L);
                        }
                        if (nTextLargestWidth < nTempWidth)
                        {
                            nTextLargestWidth = nTempWidth;
                        }
                        nTempWidth =0;
                    }
                    else
                    {
                        nTempWidth += pTextScroll->abcWidths[pTextScroll->characterBuffer[i+dwOffset]-0x20];
                    }
                }
                dwOffset += dwBytesRead;
            }
        }
        DEBUG_STRING("Loading the rest of the file ..");
        if (nLinesCompute==0
            || nLinesCompute < nLinesToCompute)
        {
            if (nLinesCompute == 0)
            {
                nTextLargestWidth = nTempWidth;
            }
            pTextScroll->nWindowLineEndOffset++;
            pTextScroll->lineBuffer[pTextScroll->nLineEndIndex] = dwOffset;
            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED, (WPARAM)pTextScroll->nLineEndIndex-1, 0L);
            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED_LOADRENDERFINISHED, 0L, 0L);
        }
        if (!isSmallerThanBuffer)
        {
            if (pTextScroll->lgFileSize.HighPart ==0)
            {
                dwBytesToBeRead = pTextScroll->lgFileSize.LowPart - dwOffset;
                DEBUG_STRING("dwBytesToBeRead before: %d", dwBytesToBeRead);
            }
            else
            {
                llOffset = (LONGLONG)dwOffset;
                llBytesToBeRead = pTextScroll->lgFileSize.QuadPart - llOffset;
            }
            dwBufferOffset =0;
            
            while ((pTextScroll->lgFileSize.HighPart ==0 && dwBytesToBeRead > 0)
                || llBytesToBeRead > 0)
            {
                if (ReadFile(hFile, pTextScroll->loadFileBuffer, LOAD_BUF_SZ, &dwBytesRead, NULL))
                {
                    if (pTextScroll->lgFileSize.HighPart ==0)
                    {
                        dwBytesToBeRead -= dwBytesRead;
                    }
                    else
                    {
                        llBytesToBeRead -= (LONGLONG)dwBytesRead;
                    }
                    for (i=0;
                        i<dwBytesRead;
                        i++)
                    {
                        if (pTextScroll->loadFileBuffer[i]  == '\r'
                            || pTextScroll->loadFileBuffer[i] =='\n')
                        {
                            if (pTextScroll->loadFileBuffer[i] == '\r')
                            {
                                i++;
                            }
                     
                            if (((pTextScroll->lgFileSize.HighPart==0)
                                && (dwOffset - (DWORD)pTextScroll->historyBuffer[pTextScroll->nHistories].szFileOffset) >= KB(4))
                                
                                || ((llOffset - pTextScroll->historyBuffer[pTextScroll->nHistories].szFileOffset) >= KB(4)))
                            {
                                pTextScroll->nHistories++;
                                pTextScroll->historyBuffer[pTextScroll->nHistories].nStartPosition=  pTextScroll->historyBuffer[pTextScroll->nHistories-1].nEndPosition+1;
                                pTextScroll->historyBuffer[pTextScroll->nHistories].nEndPosition= pTextScroll->historyBuffer[pTextScroll->nHistories].nStartPosition;
                                if (pTextScroll->lgFileSize.HighPart ==0)
                                {
                                    pTextScroll->historyBuffer[pTextScroll->nHistories].szFileOffset = dwOffset+i+1;
                                }
                                else
                                {
                                    pTextScroll->historyBuffer[pTextScroll->nHistories].szFileOffset = llOffset+i+1;
                                }
                            }
                            else
                            {
                                pTextScroll->historyBuffer[pTextScroll->nHistories].nEndPosition++;
                            }
                            if (nTextLargestWidth < nTempWidth)
                            {
                                nTextLargestWidth =nTempWidth;
                            }
                        }
                        else
                        {
                            nTempWidth += pTextScroll->abcWidths[pTextScroll->loadFileBuffer[i]-0x20];
                        }
                    }
                    if (pTextScroll->lgFileSize.HighPart == 0)
                    {
                        dwOffset += dwBytesRead;
                    }
                    else
                    {
                        llOffset += (LONGLONG)dwBytesRead;
                    }
                }
            }
        }
        pTextScroll->nHistories++;
        pTextScroll->nMaxWidth = nTempWidth > nTextLargestWidth ? nTempWidth : nTextLargestWidth;
        pTextScroll->nMaxHeight = pTextScroll->historyBuffer[pTextScroll->nHistories-1].nEndPosition * pTextScroll->nTextHeight;

        DEBUG_STRING("Load File complete!");
        
        DEBUG_STRING("nMaxHeight: %lld", pTextScroll->nMaxHeight);
        DEBUG_STRING("nHistories: %d", pTextScroll->nHistories);
        DEBUG_STRING("nHistoryEndIndex: %d", pTextScroll->nHistoryEndIndex);
        DEBUG_STRING("nEndPosFileOffset: %lld", pTextScroll->historyBuffer[pTextScroll->nHistories-1].szFileOffset);
        DEBUG_STRING("nEndPos: %lld",pTextScroll->historyBuffer[pTextScroll->nHistories-1].nEndPosition);
        DEBUG_STRING("nCharacterStartIndex: %d", pTextScroll->nCharacterStartIndex);
        DEBUG_STRING("nCharacterEndIndex: %d", pTextScroll->nCharacterEndIndex);

        PostMessageA(pTextScroll->hWnd, WM_LOADFILECOMPLETE, 0L, 0L);
        #endif
#if defined (__DEBUG) && 0
        for (int u=pTextScroll->nLineStartIndex;
            u != pTextScroll->nLineEndIndex;
            u++)
        {
            DEBUG_STRING("nLineIndex: %d\tch Offset: %d", u, pTextScroll->lineBuffer[u]);
        }
        for (int p=0;
            p < pTextScroll->nHistories;
            p++)
        {
            DEBUG_STRING("nHistoryIndex: %d\tszFileOffset: %lld\tnStartPos: %lld\tnEndPos: %lld\n", p, pTextScroll->historyBuffer[p].szFileOffset,
                pTextScroll->historyBuffer[p].nStartPosition, pTextScroll->historyBuffer[p].nEndPosition);
        }
#endif
    }
    CloseHandle(hFile);
    pTextScroll->hFile = CreateFileA(pTextScroll->filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
}
/* 
void
ComputeThread_LoadLine(SHARED TextScroll* pTextScroll, WPARAM wParam, LPARAM lParam)
{
    #if 1
    UINT64 bufferStartLine = pTextScroll->nStartLineOffset;
    UINT64 bufferEndLine = pTextScroll->nStartLineOffset + GetLineBufferCount(pTextScroll);

    if (bufferEndLine < lParam)
    {
        if (bufferEndLine < wParam)
        {
        }
        else
        {
            LARGE_INTEGER lgFileMove;
            while (pTextScroll->nHistoryEndIndex < pTextScroll->nHistories
                && pTextScroll->historyBuffer[pTextScroll->nHistoryEndIndex].nEndPosition < bufferStartLine)
            {
                pTextScroll->nHistoryStartIndex++;
                pTextScroll->nHistoryEndIndex++;
            }
            UINT64 startLineIndex = pTextScroll->historyBuffer[pTextScroll->nHistoryEndIndex].nStartPosition - pTextScroll->nStartLineOffset;
            UINT64 endLineIndex = pTextScroll->nLineEndIndex;
            lgFileMove.QuadPart = pTextScroll->historyBuffer[pTextScroll->nHistoryEndIndex].nStartPosition;
            SetFilePointerEx(pTextScroll->hFile, lgFileMove, NULL, FILE_BEGIN);

            DWORD dwToBeRead=0;
            DWORD dwRead=0;
            DWORD dwToRead=0;
            DWORD dwOffset=0;

            if (pTextScroll->nHistoryEndIndex == (pTextScroll->nHistories-1))
            {
                dwToBeRead = (DWORD)(pTextScroll->lgFileSize.QuadPart - lgFileMove.QuadPart);
            }
            else
            {
                dwToBeRead = (DWORD)(pTextScroll->historyBuffer[pTextScroll->nHistoryEndIndex+1].szFileOffset - lgFileMove.QuadPart);
            }
            while (dwToBeRead > 0)
            {
                if ((pTextScroll->nCharacterEndIndex-1) == CHAR_BUF_SZ)
                {
                    pTextScroll->nCharacterEndIndex = -1;
                }
                else
                {
                    dwToRead = (dwToBeRead > (CHAR_BUF_SZ-pTextScroll->nCharacterEndIndex-1)) ? (CHAR_BUF_SZ-pTextScroll->nCharacterEndIndex-1) : dwToBeRead; 
                    if (ReadFile(pTextScroll->hFile, pTextScroll->characterBuffer+pTextScroll->nCharacterEndIndex+1, dwToRead, &dwRead, NULL))
                    {
                        for (DWORD i=0;
                            i<dwRead;
                            i++)
                        {
                            if (pTextScroll->characterBuffer[pTextScroll->nCharacterEndIndex+1+i] == '\r'
                                || pTextScroll->characterBuffer[pTextScroll->nCharacterEndIndex+1+i] == '\n')
                            {
                                if (pTextScroll->characterBuffer[pTextScroll->nCharacterEndIndex+1+i] == '\r')
                                {
                                    i++;
                                }
                                pTextScroll->lineBuffer[pTextScroll->nLineEndIndex] = pTextScroll->nCharacterEndIndex+2+i;
                                PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED, (WPARAM)pTextScroll->nLineEndIndex-1, 0L);
                                if (pTextScroll->nLineEndIndex == (LINE_BUF_CN-1))
                                {
                                    pTextScroll->nLineEndIndex=-1;
                                    pTextScroll->nLineStartIndex =0;
                                }
                                pTextScroll->lineBuffer[++pTextScroll->nLineEndIndex] = pTextScroll->nCharacterEndIndex+2+i;
                            }
                        }
                        dwToBeRead -= dwRead;
                        pTextScroll->nCharacterEndIndex+= dwRead;
                    }
                }
            }
            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_SHARED_LOADRENDERFINISHED, 0L, 0L);
        }
    }
    else if (bufferStartLine > wParam)
    {
        if (bufferStartLine > lParam)
        {

        }
        else
        {

        }
    }
    #endif
} */

#define WM_INTERNAL_COMPUTE_LOADFILE    WM_USER+0x100
#define WM_INTERNAL_COMPUTE_LOADLINE    WM_USER+0x101

DWORD WINAPI
ComputeLinesThreadProc(LPVOID lpParameter)
{
    MSG msg;
    SHARED TextScroll *pTextScroll = (TextScroll *)lpParameter;

    PeekMessageA(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    while (GetMessageA(&msg, NULL, 0, 0))
    {
        if (WM_INTERNAL_COMPUTE_LOADFILE == msg.message)
        {
            ComputeThread_LoadFile(pTextScroll);
        }
        else if (WM_INTERNAL_COMPUTE_LOADLINE == msg.message)
        {
           // ComputeThread_LoadLine(pTextScroll, msg.wParam, msg.lParam);
        }
        else if (WM_INTERNAL_QUIT == msg.message)
        {
            break;
        }
    }
    SetEvent(pTextScroll->hWaitQuitComputeThreadEvent);
    return 0;
}

BOOL
LoadFile(SHARED TextScroll *pTextScroll)
{
    pTextScroll->nRenderHortzTextScrollOffset=0;
    pTextScroll->nRenderVertTextScrollOffset=0;
    pTextScroll->nScrollWidth = 0;
    pTextScroll->nScrollHeight =0;


    PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
    PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_ONLOADRENDER, 0L, 0L);
    WaitForSingleObject(pTextScroll->hWaitOnLoadRenderEvent, INFINITE);
    ResetEvent(pTextScroll->hWaitOnLoadRenderEvent);

    PostThreadMessageA(pTextScroll->dwComputeThread, WM_INTERNAL_COMPUTE_LOADFILE, 0L, 0L);

    return TRUE;
}

void
ChooseFile(SHARED TextScroll *pTextScroll)
{
    OPENFILENAMEA opfn =
    {
        .lStructSize = sizeof(OPENFILENAMEA),
        .hwndOwner = pTextScroll->hWnd,
        .hInstance = (HINSTANCE)GetWindowLongPtrA(pTextScroll->hWnd, GWLP_HINSTANCE),
        .lpstrFilter = "Text Files\0*.TXT;\0",
        .lpstrFile = (LPSTR)pTextScroll->filePath,
        .nMaxFile = sizeof(pTextScroll->filePath),
        .Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST
    };
    if (GetOpenFileNameA(&opfn))
    {
        // xD
        DEBUG_STRING("Filename: %s", opfn.lpstrFile);
        if (pTextScroll->hFile)
        {
            CloseHandle(pTextScroll->hFile);
            pTextScroll->hFile = 0L;
        }
        LoadFile(pTextScroll);
    }
}

BOOL
ChooseRenderFont(SHARED TextScroll *pTextScroll)
{
    pTextScroll->lgFont.lfCharSet = ANSI_CHARSET;

    CHOOSEFONTA cf =
    {
        .lStructSize = sizeof(CHOOSEFONTA),
        .hwndOwner = pTextScroll->hWnd,
        .hDC = pTextScroll->hDCMem,
        .lpLogFont =&pTextScroll->lgFont,
        .Flags = CF_ANSIONLY|CF_EFFECTS|CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT|CF_LIMITSIZE|CF_SHOWHELP,
        .rgbColors = GetTextColor(pTextScroll->hDCMem),
        .nSizeMin= 8,
        .nSizeMax = 26
    };
    if (ChooseFontA(&cf))
    {
        TEXTMETRICA tm;
        DeleteObject(SelectObject(pTextScroll->hDCMem, CreateFontIndirectA(&pTextScroll->lgFont)));
        SetTextColor(pTextScroll->hDCMem, cf.rgbColors);
        GetTextMetricsA(pTextScroll->hDCMem, &tm);

        pTextScroll->nTextHeight = tm.tmHeight;

        DEBUG_STRING(".lfHeight: %d\r\n.tmHeight: %d", pTextScroll->lgFont.lfHeight, tm.tmHeight);
    
        return TRUE;
    }
    return FALSE;
}

unsigned int inline
GetLineBufferCount(SHARED TextScroll *pTextScroll)
{
    if (pTextScroll->nLineEndIndex < pTextScroll->nLineStartIndex)
    {
        return LINE_BUF_CN;
    }
    return pTextScroll->nLineEndIndex -pTextScroll->nLineStartIndex;
}

void
RecalculateAndDrawVertOffsets(SHARED TextScroll *pTextScroll)
{
    UINT64 startLine = pTextScroll->nScrollYPos/pTextScroll->nTextHeight;
    UINT64 endLine = ((pTextScroll->nScrollYPos+CALC_RECT_H(pTextScroll->rcTextScroll))/pTextScroll->nTextHeight)+1;

    if (endLine > pTextScroll->historyBuffer[pTextScroll->nHistories-1].nEndPosition)
    {
        endLine = pTextScroll->historyBuffer[pTextScroll->nHistories-1].nEndPosition;
    }
    DEBUG_STRING("nHeight: %lld", (pTextScroll->nScrollYPos+CALC_RECT_H(pTextScroll->rcTextScroll)));

    if (endLine > (pTextScroll->nStartLineOffset + GetLineBufferCount(pTextScroll))
        || startLine < pTextScroll->nStartLineOffset)
    {
    /*     //TODO: Compute (pTextScroll->nStartLineOffset+GetLineBufferCount(pTextScroll)+1) to endLine
        DEBUG_STRING("Computing %d to %d", startLine, endLine);
        PostThreadMessageA(pTextScroll->dwComputeThread, WM_INTERNAL_COMPUTE_LOADLINE, (WPARAM)startLine, (LPARAM)endLine); */
    }
    else
    {
        pTextScroll->nWindowLineStartOffset = startLine +pTextScroll->nStartLineOffset;
        pTextScroll->nWindowLineEndOffset = endLine +pTextScroll->nStartLineOffset;
        DEBUG_STRING("endLine: %lld\r\n", endLine);
        pTextScroll->nRenderVertTextScrollOffset  = pTextScroll->nScrollYPos - (startLine * pTextScroll->nTextHeight);
    }
}

void
RecaculateVertScrollPosition(SHARED TextScroll *pTextScroll)
{
    if (pTextScroll->nScrollPixelYPos < 0)
    {
        pTextScroll->nScrollPixelYPos =0;
    }
    else if (pTextScroll->nScrollPixelYPos > (CALC_RECT_H(pTextScroll->rcVertScroll) -CALC_RECT_H(pTextScroll->rcVertScrollPellet)))
    {
        pTextScroll->nScrollPixelYPos = CALC_RECT_H(pTextScroll->rcVertScroll) - CALC_RECT_H(pTextScroll->rcVertScrollPellet);
    }

    SetRect(&pTextScroll->rcVertScrollPellet, pTextScroll->rcVertScrollPellet.left, pTextScroll->nScrollPixelYPos, pTextScroll->rcVertScrollPellet.right,
        pTextScroll->nScrollPixelYPos +CALC_RECT_H(pTextScroll->rcVertScrollPellet));
    
    if (pTextScroll->isOverHeight)
    {
        if (pTextScroll->nScrollPixelYPos == (CALC_RECT_H(pTextScroll->rcVertScroll)- CALC_RECT_H(pTextScroll->rcVertScrollPellet)))
        {
            pTextScroll->nScrollYPos =pTextScroll->nScrollHeight;
        }
        else if (pTextScroll->nScrollPixelYPos == 0)
        {
            pTextScroll->nScrollYPos = 0;
        }
        else
        {
            pTextScroll->nScrollYPos = (((double)pTextScroll->nScrollHeight)/((double)(CALC_RECT_H(pTextScroll->rcVertScroll)- CALC_RECT_H(pTextScroll->rcVertScrollPellet))))
                * ((double)pTextScroll->nScrollPixelYPos);
        }
    }
    else
    {
        pTextScroll->nScrollYPos = pTextScroll->nScrollPixelYPos;
    }
    RecalculateAndDrawVertOffsets(pTextScroll);
    PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_REDRAW, 0L, 0L);
}

void inline
RecalculateVertPixelScrollPosition(SHARED TextScroll *pTextScroll)
{
    if (pTextScroll->isOverHeight)
    {
        if (pTextScroll->nScrollYPos == pTextScroll->nScrollHeight)
        {
            pTextScroll->nScrollPixelYPos = pTextScroll->rcVertScroll.bottom- CALC_RECT_H(pTextScroll->rcVertScrollPellet);
        }
        else if (pTextScroll->nScrollYPos ==0)
        {
            pTextScroll->nScrollPixelYPos = 0;
        }
        else
        {
            pTextScroll->nScrollPixelYPos = ((double)(pTextScroll->rcVertScroll.bottom- CALC_RECT_H(pTextScroll->rcVertScrollPellet)))/((double)pTextScroll->nScrollHeight) *
                pTextScroll->nScrollYPos;
        }
    }
    else
    {
        pTextScroll->nScrollPixelYPos = pTextScroll->nScrollYPos;
    }
    SetRect(&pTextScroll->rcVertScrollPellet, pTextScroll->rcVertScrollPellet.left, pTextScroll->nScrollPixelYPos, pTextScroll->rcVertScrollPellet.right,
        pTextScroll->nScrollPixelYPos +CALC_RECT_H(pTextScroll->rcVertScrollPellet));

    RecalculateAndDrawVertOffsets(pTextScroll);
    PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_REDRAW, 0L, 0L);
}

void
RecalculateScrollParamaeters(SHARED TextScroll *pTextScroll)
{
    UINT64 uTextScrollWidth =CALC_RECT_W(pTextScroll->rcTextScroll);
    UINT64 uTextScrollHeight = CALC_RECT_H(pTextScroll->rcTextScroll);

    DEBUG_STRING("Width: %lld", uTextScrollWidth);
    DEBUG_STRING("Height: %lld", uTextScrollHeight);
    
    if (pTextScroll->nMaxWidth <= uTextScrollWidth)
    {
        pTextScroll->nScrollWidth=0;
    }
    else
    {
        pTextScroll->nScrollWidth =pTextScroll->nMaxWidth-uTextScrollWidth;
        pTextScroll->rcHortzScrollPellet.left = 0;
        pTextScroll->rcHortzScrollPellet.top = pTextScroll->nHeight -SCROLL_WIDTH;
        pTextScroll->rcHortzScrollPellet.bottom = pTextScroll->nHeight;
        pTextScroll->isOverWidth = pTextScroll->nScrollWidth > (uTextScrollWidth -SCROLLPELLET_BREADTH);
    
        pTextScroll->rcHortzScrollPellet.right = pTextScroll->isOverWidth ? SCROLLPELLET_BREADTH :
            (uTextScrollWidth-pTextScroll->nScrollWidth);
        pTextScroll->nScrollPixelXPos =0;
    }
    if (pTextScroll->nMaxHeight <= uTextScrollHeight)
    {
        pTextScroll->nScrollHeight=0;
    }
    else
    {
        pTextScroll->nScrollHeight =pTextScroll->nMaxHeight-uTextScrollHeight;
        pTextScroll->rcVertScrollPellet.left = pTextScroll->nWidth - SCROLL_WIDTH;
        pTextScroll->rcVertScrollPellet.right=pTextScroll->nWidth;
        pTextScroll->rcVertScrollPellet.top=0;
        pTextScroll->isOverHeight = pTextScroll->nScrollHeight > (uTextScrollHeight-SCROLLPELLET_BREADTH);
        pTextScroll->rcVertScrollPellet.bottom = pTextScroll->isOverHeight ? SCROLLPELLET_BREADTH :
            (uTextScrollHeight- pTextScroll->nScrollHeight);
        pTextScroll->nScrollPixelYPos =0;
    }

    pTextScroll->nScrollXPos =0;
    pTextScroll->nScrollYPos = 0;
}

LRESULT WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static SHARED TextScroll* pTextScroll;

    switch (uMsg)
    {
        case WM_CREATE:
            pTextScroll = (TextScroll *)VirtualAlloc(NULL, sizeof(TextScroll), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            if (pTextScroll)
            {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);
#if defined (__DEBUG)
                lpDebugStrBuf = pTextScroll->lpDebugStrBuf;
#endif
                
                pTextScroll->hWnd = hWnd;
                pTextScroll->nWidth = rcClient.right;
                pTextScroll->nHeight = rcClient.bottom;
                DEBUG_STRING("->nWidth: %d\r\n->nHeight: %d", pTextScroll->nWidth, pTextScroll->nHeight);
                
                pTextScroll->nMaxWidth = 0;
                pTextScroll->nMaxHeight =0;
                pTextScroll->nScrollWidth =0;
                pTextScroll->nScrollHeight =0;
                pTextScroll->nScrollXPos =0;
                pTextScroll->nScrollYPos=0;

                pTextScroll->nRenderVertTextScrollOffset = 0;

                pTextScroll->filePath[0] = '\0';
                pTextScroll->rcTextScroll.left= 0;
                pTextScroll->rcTextScroll.top =0;
                pTextScroll->rcTextScroll.right =pTextScroll->nWidth -SCROLL_WIDTH;
                pTextScroll->rcTextScroll.bottom=pTextScroll->nHeight-SCROLL_WIDTH;

                pTextScroll->rcHortzScroll.left=0;
                pTextScroll->rcHortzScroll.right=pTextScroll->nWidth;
                pTextScroll->rcHortzScroll.top=pTextScroll->nHeight -SCROLL_WIDTH;
                pTextScroll->rcHortzScroll.bottom=pTextScroll->nHeight;

                pTextScroll->rcVertScroll.left =pTextScroll->nWidth -SCROLL_WIDTH;
                pTextScroll->rcVertScroll.right = pTextScroll->nWidth;
                pTextScroll->rcVertScroll.top=0;
                pTextScroll->rcVertScroll.bottom=pTextScroll->nHeight -SCROLL_WIDTH;

                pTextScroll->isHortzScrollHot =FALSE;
                pTextScroll->isHortzScrollPelletActive = FALSE;
                pTextScroll->isHortzScrollPelletHot = FALSE;
                pTextScroll->isHortzScrollPelletActive = FALSE;
                
                pTextScroll->isVertScrollHot =FALSE;
                pTextScroll->isVertScrollPelletHot = FALSE;
                pTextScroll->isVertScrollPelletActive = FALSE;

                pTextScroll->isMouseDown = FALSE;
                pTextScroll->isOverHeight = FALSE;
                pTextScroll->dwMouseScrollDelta = 0;

                pTextScroll->hDCMem = 0L;
                pTextScroll->hGdiOldBmpMem = 0L;
                pTextScroll->hGdiOldFont= 0L;
                pTextScroll->hFile =0L;

                pTextScroll->nHistories =0;
                pTextScroll->nHistoryStartIndex = 0;
                pTextScroll->nHistoryEndIndex= 0;
                pTextScroll->nStartLineOffset =0;
                pTextScroll->nLineStartIndex = 0;
                pTextScroll->nLineEndIndex = 0;
                pTextScroll->nCharacterStartIndex = 0;
                pTextScroll->nCharacterEndIndex = 0;

                BOOL isBufferInit = InitializeBackBuffer(pTextScroll);
                
                pTextScroll->hWaitQuitRenderThreadEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
                CreateThread(NULL, 0, WindowRenderThreadProc, (LPVOID)pTextScroll, 0, &pTextScroll->dwRenderThread);

                pTextScroll->hWaitQuitComputeThreadEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
                CreateThread(NULL, 0, ComputeLinesThreadProc, (LPVOID)pTextScroll, 0, &pTextScroll->dwComputeThread);

                pTextScroll->hWaitOnLoadRenderEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
                
                if (isBufferInit)
                {
                    ABC abcWidths[CHAR_RANGE];
                    GetCharABCWidthsA(pTextScroll->hDCMem, 0x20, 0xFF, abcWidths);
                    for (int i=0;
                        i<CHAR_RANGE;
                        i++)
                    {
                        pTextScroll->abcWidths[i] = abcWidths[i].abcA +abcWidths[i].abcB + abcWidths[i].abcC;
                    }
                    LPCREATESTRUCTA lpCSA = (LPCREATESTRUCTA)lParam;
                    if (lpCSA->lpCreateParams)
                    {
                        WIN32_FIND_DATAA *pWin32FindData = (WIN32_FIND_DATAA*)lpCSA->lpCreateParams;
                        strcpy(pTextScroll->filePath, pWin32FindData->cFileName);
                        LoadFile(pTextScroll);
                    }
                    return 0L;
                }
            }
            return -1L;

        case WM_COMMAND:
            if (HIWORD(wParam) ==0)
            {
                if (LOWORD(wParam) == IDM_LOADFILE)
                {
                    ChooseFile(pTextScroll);
                    return 0L;
                }
                else if (LOWORD(wParam) == IDM_CHOOSEFONT)
                {
                    if (ChooseRenderFont(pTextScroll))
                    {
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_REDRAW, 0L, 0L);
                    }
                    return 0L;
                }
            }
            break;

        case WM_MOUSEWHEEL:
            #define APP_SCROLL_DELTA 16

            short nScrollAmt = GET_WHEEL_DELTA_WPARAM(wParam)/ APP_SCROLL_DELTA;
            if (nScrollAmt > 0)
            {
                if (pTextScroll->nScrollYPos > nScrollAmt)
                {
                    pTextScroll->nScrollYPos-= nScrollAmt;
                }
                else
                {
                    pTextScroll->nScrollYPos = 0;
                }
            }
            else if (nScrollAmt < 0)
            {
                if ((pTextScroll->nScrollYPos-nScrollAmt) < pTextScroll->nScrollHeight)
                {
                    pTextScroll->nScrollYPos -= nScrollAmt; 
                }
                else
                {
                    pTextScroll->nScrollYPos = pTextScroll->nScrollHeight;
                }
            }
            RecalculateVertPixelScrollPosition(pTextScroll);
            return 0L;

        case WM_LBUTTONDOWN:
            if (pTextScroll->isHortzScrollPelletHot
                || pTextScroll->isVertScrollPelletHot)
            {
                RECT rcClient;
                SetCapture(hWnd);
                pTextScroll->ptPrevMouse.x = GET_X_LPARAM(lParam);
                pTextScroll->ptPrevMouse.y = GET_Y_LPARAM(lParam);
                pTextScroll->isMouseDown = TRUE;
                if (pTextScroll->isHortzScrollPelletHot)
                {
                    pTextScroll->isHortzScrollPelletActive = TRUE;
                }
                else
                {
                    pTextScroll->isVertScrollPelletActive = TRUE;
                }
                PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_RENDERREADY, 0L, 0L);
            }
            return 0L;
        
        case WM_LBUTTONUP:
            if (pTextScroll->isMouseDown)
            {
                ReleaseCapture();
                pTextScroll->isHortzScrollPelletActive =FALSE;
                pTextScroll->isVertScrollPelletActive=FALSE;
                PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_RENDERSTOP, 0L, 0L);
                pTextScroll->isMouseDown =FALSE;
            }
            return 0L;

        case WM_NCMOUSEMOVE:
            DEBUG_STRING("WM_NCMOUSEMOVE");
            if (pTextScroll->isMouseDown)
            {
                ReleaseCapture();
                ClipCursor(NULL);
                pTextScroll->isHortzScrollPelletActive = FALSE;
                pTextScroll->isVertScrollPelletActive =FALSE;
                pTextScroll->isMouseDown = FALSE;
            }
            return 0L;

        case WM_MOUSEMOVE:
            #define PT_IN_RECT(rect, x, y) (rect.left <= x && x<=rect.right && rect.top <=y && y<=rect.bottom)

            if (wParam & MK_LBUTTON
                && pTextScroll->isMouseDown)
            {
                if (pTextScroll->isHortzScrollPelletActive)
                {
                    pTextScroll->ptPrevMouse.x = GET_X_LPARAM(lParam)-pTextScroll->ptPrevMouse.x;
                    if (!((pTextScroll->ptPrevMouse.x < 0 && pTextScroll->nScrollXPos == 0)
                        || (pTextScroll->ptPrevMouse.x > 0 && pTextScroll->nScrollXPos == pTextScroll->nScrollWidth)))
                    {
                        pTextScroll->nScrollPixelXPos += pTextScroll->ptPrevMouse.x;
                        if (pTextScroll->nScrollPixelXPos < 0)
                        {
                            pTextScroll->nScrollPixelXPos =0;
                        }
                        else if (pTextScroll->nScrollPixelXPos > CALC_RECT_W(pTextScroll->rcHortzScroll) -CALC_RECT_W(pTextScroll->rcHortzScrollPellet))
                        {
                            pTextScroll->nScrollPixelXPos = CALC_RECT_W(pTextScroll->rcHortzScroll) - CALC_RECT_W(pTextScroll->rcHortzScrollPellet);
                        }
                    }

                    SetRect(&pTextScroll->rcHortzScrollPellet, pTextScroll->nScrollPixelXPos, pTextScroll->rcHortzScrollPellet.top, pTextScroll->nScrollPixelXPos
                        +CALC_RECT_W(pTextScroll->rcHortzScrollPellet), pTextScroll->rcHortzScrollPellet.bottom);
                    
                    if (pTextScroll->isOverWidth)
                    {
                        if (pTextScroll->nScrollPixelXPos == (CALC_RECT_W(pTextScroll->rcHortzScroll)- CALC_RECT_W(pTextScroll->rcHortzScrollPellet)))
                        {
                            pTextScroll->nScrollXPos =pTextScroll->nScrollWidth;
                        }
                        else if (pTextScroll->nScrollPixelXPos == 0)
                        {
                            pTextScroll->nScrollXPos = 0;
                        }
                        else
                        {
                            pTextScroll->nScrollXPos = (((double)pTextScroll->nScrollWidth)/((double)(CALC_RECT_W(pTextScroll->rcHortzScroll)
                                - CALC_RECT_W(pTextScroll->rcHortzScrollPellet)))) * ((double)pTextScroll->nScrollPixelXPos);
                        }
                    }
                    else
                    {
                        pTextScroll->nScrollXPos = pTextScroll->nScrollPixelXPos;
                    }
                }
                else if (pTextScroll->isVertScrollPelletActive)
                {
                    pTextScroll->ptPrevMouse.y =GET_Y_LPARAM(lParam) -pTextScroll->ptPrevMouse.y;

                    if (!((pTextScroll->ptPrevMouse.y < 0 && pTextScroll->nScrollYPos == 0)
                        || (pTextScroll->ptPrevMouse.y > 0 && pTextScroll->nScrollYPos == pTextScroll->nScrollHeight)))
                    {
                        pTextScroll->nScrollPixelYPos += pTextScroll->ptPrevMouse.y;
                        RecaculateVertScrollPosition(pTextScroll);
                    }

                }
                pTextScroll->ptPrevMouse.x = GET_X_LPARAM(lParam);
                pTextScroll->ptPrevMouse.y = GET_Y_LPARAM(lParam);
            }
            else if (pTextScroll->nScrollHeight != 0
                || pTextScroll->nScrollWidth != 0)
            {
                DEBUG_STRING("Checking Scroll Boundings..");
                if (pTextScroll->nScrollHeight != 0
                    && PT_IN_RECT(pTextScroll->rcVertScroll, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
                {
                    if (PT_IN_RECT(pTextScroll->rcVertScrollPellet, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
                    {
                        if (!pTextScroll->isVertScrollPelletHot)
                        {
                            pTextScroll->isVertScrollHot =TRUE;
                            pTextScroll->isVertScrollPelletHot =TRUE;
                            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                            goto MouseMove_HortScroll; 
                        }
                        pTextScroll->isVertScrollPelletHot= TRUE;
                    }
                    else if (!pTextScroll->isVertScrollHot)
                    {
                        pTextScroll->isVertScrollHot=TRUE;
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                        goto MouseMove_HortScroll;
                    }
                    else if (pTextScroll->isVertScrollPelletHot)
                    {
                        pTextScroll->isVertScrollPelletHot = FALSE;
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                    }
                    goto MouseMove_HortScroll;
                }
                else
                {
                    if (pTextScroll->isVertScrollHot
                        || pTextScroll->isVertScrollHot)
                    {
                        pTextScroll->isVertScrollHot = FALSE;
                        pTextScroll->isVertScrollPelletHot = FALSE;
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                    }
                }

    MouseMove_HortScroll:
                if (pTextScroll->nScrollWidth != 0
                    && PT_IN_RECT(pTextScroll->rcHortzScroll, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
                {
                    if (PT_IN_RECT(pTextScroll->rcHortzScrollPellet, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
                    {
                        if (!pTextScroll->isHortzScrollPelletHot)
                        {
                            pTextScroll->isHortzScrollHot =TRUE;
                            pTextScroll->isHortzScrollPelletHot = TRUE;
                            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                            return 0L;
                        }
                        pTextScroll->isHortzScrollPelletHot = TRUE;
                    }
                    else if (!pTextScroll->isHortzScrollHot)
                    {
                        pTextScroll->isHortzScrollHot =TRUE;
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                        return 0L;
                    }
                    else if (pTextScroll->isHortzScrollPelletHot)
                    {
                        pTextScroll->isHortzScrollPelletHot = FALSE;
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_ONLOADRENDER, 0L, 0L);
                    }
                    return 0L;
                }   
                else
                {
                    if (pTextScroll->isHortzScrollHot
                        || pTextScroll->isHortzScrollPelletHot)
                    {
                        pTextScroll->isHortzScrollHot = FALSE;
                        pTextScroll->isHortzScrollPelletHot = FALSE;
                        PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
                    }
                }
            }
            return 0L;

        case WM_CANTHANDLEFILE:
            MessageBoxA(NULL, "This application is for demo purposes only, file cannot exceed more than 6MB", "ReadPad", MB_OK|MB_ICONINFORMATION);
            return 0L;

        case WM_LOADFILECOMPLETE:
            RecalculateScrollParamaeters(pTextScroll);
            PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_RENDER_SCROLLRENDER, 0L, 0L);
            return 0L;

        case WM_SIZE:
            pTextScroll->nWidth = LOWORD(lParam);
            pTextScroll->nHeight = HIWORD(lParam);
            pTextScroll->rcTextScroll.right = pTextScroll->nWidth - SCROLL_WIDTH;
            pTextScroll->rcTextScroll.bottom = pTextScroll->nHeight -SCROLL_WIDTH;

            pTextScroll->rcVertScroll.left = pTextScroll->rcTextScroll.right;
            pTextScroll->rcVertScroll.right = pTextScroll->nWidth;
            pTextScroll->rcVertScroll.bottom = pTextScroll->rcTextScroll.bottom;

            pTextScroll->rcHortzScroll.right = pTextScroll->nWidth;
            pTextScroll->rcHortzScroll.top = pTextScroll->rcTextScroll.bottom;
            pTextScroll->rcHortzScroll.bottom = pTextScroll->nHeight;

            ResizeBackBuffer(pTextScroll);
            RecalculateScrollParamaeters(pTextScroll);
            return 0L;
        
        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            RenderThread_DrawTextScroll(pTextScroll);
            return 0L;

        case WM_DESTROY:

#if defined (__DEBUG)
    #define WAIT_CHECK(hEvent)                                              \
        if (WAIT_TIMEOUT == WaitForSingleObject(hEvent, 1250))              \
        {                                                                   \
            DEBUG_STRING("%s wait has timed out %p",#hEvent, hEvent);         \
        }
#else
    #define WAIT_CHECK(hEvent) WaitForSingleObject(hEvent, 1250)
#endif

            if (pTextScroll)
            {
                PostThreadMessageA(pTextScroll->dwRenderThread, WM_INTERNAL_QUIT, 0L, 0L);
                WAIT_CHECK(pTextScroll->hWaitQuitRenderThreadEvent);


                PostThreadMessageA(pTextScroll->dwComputeThread, WM_INTERNAL_QUIT, 0L, 0L);
                WAIT_CHECK(pTextScroll->hWaitQuitComputeThreadEvent);

                if (pTextScroll->hFile)
                {
                    CloseHandle(pTextScroll->hFile);
                }


                ReleaseBackBuffer(pTextScroll);
                VirtualFree(pTextScroll, 0, MEM_RELEASE);
            }
            PostQuitMessage(0);
            break;
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

HMENU
InitMenu()
{
    char loadFileMenuString[] = "File",
        chooseFontMenuString[] = "Font";
    BOOL isInit=FALSE;
    HMENU hMenu = CreateMenu();
    
    if (hMenu)
    {
        MENUITEMINFOA mii =
        {
            .cbSize = sizeof(MENUITEMINFOA),
            .fMask= MIIM_TYPE|MIIM_ID|MIIM_DATA,
            .fType = MFT_STRING,
            .fState = MFS_DEFAULT,
            .wID = IDM_CHOOSEFONT,
            .dwTypeData = chooseFontMenuString,
            .cch = sizeof(chooseFontMenuString)-1
        };
        isInit = InsertMenuItemA(hMenu, 0, TRUE, &mii);

        mii.wID = IDM_LOADFILE;
        mii.dwTypeData = loadFileMenuString;
        mii.cch = sizeof(loadFileMenuString)-1;
        
        isInit = isInit && InsertMenuItemA(hMenu, 0, TRUE, &mii);
        if (!isInit)
        {
            DestroyMenu(hMenu);
            hMenu =0L;
        }
    }
    return hMenu;
}

#define WINDOW_STYLE     WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_SIZEBOX

#if defined (__DEBUG)
int
main(int argc, char *argv[])
{
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    #define lpCmdLine argv[1]
#else
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#endif
    WIN32_FIND_DATAA w32FindData;
    HANDLE hSearch = INVALID_HANDLE_VALUE;
    const char sErrorFormat[] = "Failed to locate %s! File does not exist!";
    char sBufError[MAX_PATH+1+sizeof(sErrorFormat)];

#if defined (__DEBUG)
    if (argc ==2 && lpCmdLine[0] != '\0')
#else
    if (lpCmdLine[0] != '\0')
#endif
    {
        hSearch = FindFirstFileA(lpCmdLine, &w32FindData);
        if (INVALID_HANDLE_VALUE == hSearch)
        {
            snprintf(sBufError, MAX_PATH+1+sizeof(sErrorFormat), sErrorFormat, lpCmdLine);
            MessageBoxA(NULL, sBufError, "ReadPad", MB_OK|MB_ICONERROR);
        }
        else
        {
            FindClose(hSearch);
        }
    }
    HMENU hMenu = InitMenu();
    if (hMenu)
    {
        RECT rcWindow =
        {
            .right = MAX_WIDTH,
            .bottom = MAX_HEIGHT
        };
        AdjustWindowRect(&rcWindow, WINDOW_STYLE, TRUE);
        WNDCLASSA wc =
        {
            .lpfnWndProc = WindowProc,
            .style = CS_VREDRAW|CS_HREDRAW,
            .hIcon = LoadIconA(NULL, MAKEINTRESOURCEA(IDI_APPLICATION)),
            .hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(IDC_ARROW)),
            .lpszClassName = "READPADWin32"
        };
        ATOM wndClassAtom = RegisterClassA(&wc);
        if (wndClassAtom)
        {
            HWND hWnd=  CreateWindowExA(0, (LPCSTR)(LONG_PTR)wndClassAtom, "ReadPad", WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, CALC_RECT_W(rcWindow),
                CALC_RECT_H(rcWindow), NULL, hMenu, hInstance, (INVALID_HANDLE_VALUE == hSearch) ? NULL : ((LPVOID)&w32FindData));
            if (hWnd)
            {
                MSG msg;
                while (GetMessageA(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
            }
        }
    }
    return 0;
}