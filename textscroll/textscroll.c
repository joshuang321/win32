#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define FNT_HEIGHT  18

ABC abcWidths[0x60];

static void
DrawTextScroll(HDC hDCWnd, HDC hDCMem, int cx, int cy, char* buf, int nCount, int *nLinePos, int nLinePosIndex, int nLineRenderStartOffset, int nLineRenderEndOffset,
    int nPixelOffset, int nCursorPos)
{
    printf("Draw called\r\n");
    PatBlt(hDCMem, 0, 0, cx, cy, WHITENESS);
    int  y=nPixelOffset;
    
    for (;
        nLineRenderStartOffset<(nLineRenderEndOffset-1);
        )
    {
        TextOutA(hDCMem, 0, y, buf+nLinePos[nLineRenderStartOffset] , nLinePos[nLineRenderStartOffset+1] - nLinePos[nLineRenderStartOffset]);
        y+=FNT_HEIGHT;
        nLineRenderStartOffset++;
    }
    TextOutA(hDCMem, 0, y, buf+nLinePos[nLineRenderStartOffset], nCount - nLinePos[nLineRenderStartOffset]);

    MoveToEx(hDCMem, nCursorPos, y, NULL);
    LineTo(hDCMem, nCursorPos, y+FNT_HEIGHT);

    BitBlt(hDCWnd, 0, 0, cx, cy, hDCMem, 0, 0, SRCCOPY);
}

static void
RecalculateRenderParams(int nLinePosIndex, int nHeight, int *pnLineRenderStartOffset, int *pnLineRenderEndOffset, int *pnPixelOffset)
{
    if ((nLinePosIndex+1) * FNT_HEIGHT > nHeight)
    {
        *pnPixelOffset =(nLinePosIndex+1) *FNT_HEIGHT - nHeight;
        *pnLineRenderStartOffset = *pnPixelOffset/FNT_HEIGHT;
        *pnLineRenderEndOffset = nLinePosIndex+1;
        *pnPixelOffset -= *pnLineRenderStartOffset * FNT_HEIGHT;
        *pnPixelOffset = -*pnPixelOffset;
    }
    else
    {
        *pnPixelOffset = 0;
        *pnLineRenderEndOffset = nLinePosIndex+1;
    }
}

static void
CalculateRenderParamsOffScrollPos(int nYScrollPos)
{

}

static void inline
IncrementLinePosition(HWND hWnd, int nPos, int *nLinePosIndex, int *nLinePos)
{
    nLinePos[++(*nLinePosIndex)] = nPos;   
}

LRESULT WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PAINTSTRUCT ps;
    static HDC hDCMem;
    static HGDIOBJ hBmpOldMem,
        hFntOldMem;
    static RECT rcClient;
    static HDC hDCWnd;
    
    static char buf[512];
    
    static int nCount=0;
    static int nLinePos[512];
    static int nLinePosIndex =0;
    static int nLineRenderStartOffset =0;
    static int nLineRenderEndOffset =1;
    static int nPixelOffset =0;

    static int nHorzPixelOffset =0;
    static int nCursorPos = 0;
    static int nPrevSpacePos = -1;

    switch (uMsg)
    {
        case WM_CREATE:
            {
                hDCWnd = GetDC(hWnd);
                
                nLinePos[nLinePosIndex] =0;
                GetClientRect(hWnd, &rcClient);
                hDCMem=  CreateCompatibleDC(hDCWnd);
                hBmpOldMem = SelectObject(hDCMem, CreateCompatibleBitmap(hDCWnd, rcClient.right, rcClient.bottom));
                hFntOldMem = SelectObject(hDCMem, CreateFontA(FNT_HEIGHT, 0,0,0, FW_REGULAR, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY,VARIABLE_PITCH, "Arial"));
                ReleaseDC(hWnd, hDCWnd);
                GetCharABCWidthsA(hDCMem, 0x20, 0x7F, abcWidths);
                for (int i=0;
                    i<0x60;
                    i++)
                {
                    abcWidths[i].abcA += abcWidths[i].abcB + abcWidths[i].abcC;
                    printf("%d\r\n",abcWidths[i].abcA);
                }
            }
            return 0;

        case WM_CHAR:
            {
                if ((char)wParam >  0x1F &&
                    nCount  < 512)
                {
                    short nCharCount = lParam &0x7FFF;
                    if ((char)wParam == 0x20)
                    {
                        nPrevSpacePos = nCount+nCharCount-1;
                    }

                    for (short i=0;
                        i<nCharCount;
                        i++)
                    {
                        if ((nCursorPos + abcWidths[(char)wParam -0x20].abcA) > rcClient.right)
                        {
                            if (nPrevSpacePos > -1
                                && nPrevSpacePos > nLinePos[nLinePosIndex])
                            {
                                nLinePos[++nLinePosIndex] = nPrevSpacePos+1;
                                RecalculateRenderParams(nLinePosIndex, rcClient.bottom, &nLineRenderStartOffset, &nLineRenderEndOffset, &nPixelOffset);
                                nCursorPos =0;
                                for (int k=nPrevSpacePos+1;
                                    k<nCount;
                                    k++)
                                {
                                    nCursorPos += abcWidths[buf[k] -0x20].abcA;
                                }
                            }
                            else
                            {
                                nLinePos[++nLinePosIndex] = nCount;
                                RecalculateRenderParams(nLinePosIndex, rcClient.bottom, &nLineRenderStartOffset, &nLineRenderEndOffset, &nPixelOffset);
                                nCursorPos =0;
                            }
                        }
                        buf[nCount++] = (char)wParam;
                        nCursorPos+= abcWidths[(char)wParam -0x20].abcA;
                    }
                }
                else if ((char)wParam  == 0x8
                    && nCount > 0)
                {
                    int i=0;
                    if ((nCount-1) == nPrevSpacePos)
                    {
                        for (i=nCount-2;
                            i > (nLinePosIndex>0 ? nLinePos[nLinePosIndex] : -1);
                            i--)
                        {
                            nPrevSpacePos = i;
                            break;
                        }
                        if (i== (nLinePosIndex>0 ? nLinePos[nLinePosIndex] : -1))
                        {
                            nPrevSpacePos=-1;
                        }
                    }
                    if (nCount == nLinePos[nLinePosIndex])
                    {
                        nCursorPos= 0;
                        if (nLinePosIndex)
                        {
                            nLinePosIndex--;
                            for (int i=nCount;
                                i>nLinePos[nLinePosIndex];
                                i--)
                            {
                                nCursorPos += abcWidths[buf[i] -0x20].abcA;
                            }
                        }
                    }
                    else
                    {
                        nCursorPos -= abcWidths[buf[--nCount] -0x20].abcA;
                    }
                    RecalculateRenderParams(nLinePosIndex, rcClient.bottom, &nLineRenderStartOffset, &nLineRenderEndOffset, &nPixelOffset);
                }
                else if ((char)wParam == 0xA
                    || (char)wParam == 0xD)
                {
                    IncrementLinePosition(hWnd, nCount, &nLinePosIndex, nLinePos);
                    nCursorPos = 0;
                    RecalculateRenderParams(nLinePosIndex, rcClient.bottom, &nLineRenderStartOffset, &nLineRenderEndOffset, &nPixelOffset);
                }
            }
            hDCWnd = GetDC(hWnd);
            DrawTextScroll(hDCWnd, hDCMem, rcClient.right, rcClient.bottom, buf, nCount, nLinePos, nLinePosIndex, nLineRenderStartOffset, nLineRenderEndOffset,
                nPixelOffset, nCursorPos);
            ReleaseDC(hWnd, hDCWnd);
            return 0;

        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            hDCWnd = GetDC(hWnd);
            DrawTextScroll(hDCWnd, hDCMem, rcClient.right, rcClient.bottom, buf, nCount, nLinePos, nLinePosIndex, nLineRenderStartOffset, nLineRenderEndOffset,
                nPixelOffset, nCursorPos);
            ReleaseDC(hWnd, hDCWnd);
            return 0;
        
        case WM_DESTROY:
            DeleteObject(SelectObject(hDCMem, hBmpOldMem));
            DeleteObject(SelectObject(hDCMem, hFntOldMem));
            DeleteDC(hDCMem);
            PostQuitMessage(0);
            break;
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

#if 0
int
main(int argc, char *argv[])
{
    HINSTANCE hInstance = GetModuleHandleA(NULL);
#else
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdline, int nCmdShow)
{
#endif
    MSG msg;
    WNDCLASSA wc =
    {
        .lpfnWndProc=WindowProc,
        .hIcon = LoadIconA(NULL, MAKEINTRESOURCEA(IDI_APPLICATION)),
        .hInstance = hInstance,
        .lpszClassName = "TEXTSCROLLV1"
    };
    ATOM wWndClassAtom = RegisterClassA(&wc);
    if (wWndClassAtom)
    {
        HWND hWnd = CreateWindowExA(0, (LPSTR)(LONG_PTR)wWndClassAtom, "Text Scroll Example", WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_VSCROLL, CW_USEDEFAULT,
            CW_USEDEFAULT, 400, 300, NULL, NULL, hInstance, NULL);
        if (hWnd)
        {
            while (GetMessageA(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }
    return 0;
}