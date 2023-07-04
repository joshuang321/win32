#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>

LRESULT WINAPI WindowProc(HWND, UINT, WPARAM, LPARAM);

const char LPSZCLASSNAME[] = "CUSTOM_WNDFRAME";
HINSTANCE g_hInst;

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSA wc {};
    ATOM wWndClassAtom;
    HWND hWnd;
    MSG msg;

    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(IDC_ARROW));
    wc.hIcon = LoadIconA(NULL, MAKEINTRESOURCEA(IDI_APPLICATION));
    wc.lpszClassName = LPSZCLASSNAME;
    wc.hInstance = hInstance;
    g_hInst = hInstance;

    wWndClassAtom = RegisterClassA(&wc);

    if (wWndClassAtom)
    {
        hWnd = CreateWindowExA(0, LPSZCLASSNAME, NULL, WS_VISIBLE|WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
            hInstance, NULL);

        if (hWnd)
        {
            while (GetMessageA(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
        else
        {
            MessageBoxA(NULL, "Failed to create window!", "wndframe", MB_OK);
        }
    }
    else
    {
        MessageBoxA(NULL, "Failed to register window class!", "wndframe", MB_OK);
    }
    return 0;
}

LRESULT WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    #define CAP_WIDTH 25

    static LRESULT lRet;
    static BOOL bCallNormal;
    static RECT rcWindow,
        rcCaption { .left=1, .top=1, .right=-2, .bottom=CAP_WIDTH-1 };
    static int nWidth, nHeight;

    static HBRUSH hCaptionBrush;
    static HWND hWndClose;

    bCallNormal = !DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lRet);
    
    switch (uMsg)
    {
        case WM_CREATE:
            GetWindowRect(hWnd, &rcWindow);
            nWidth = rcWindow.right -rcWindow.left;
            nHeight = rcWindow.bottom -rcWindow.top;

            SetWindowPos(hWnd, NULL, rcWindow.left, rcWindow.top, nWidth, nHeight, SWP_FRAMECHANGED);
            hWndClose = CreateWindowExA(0, WC_BUTTONA, "X", WS_VISIBLE|WS_CHILD, nWidth-CAP_WIDTH, 3, CAP_WIDTH-5, CAP_WIDTH-5, hWnd, (HMENU)IDCLOSE, g_hInst,
                NULL);
            rcCaption.right += nWidth;

            hCaptionBrush = CreateSolidBrush(RGB(2, 1, 127));

            lRet =0;
            bCallNormal = 0;
            break;

        case  WM_PAINT:
        {
            static HDC l_hDC;
            static PAINTSTRUCT l_psPaint;
            static HGDIOBJ l_hGdiObj;
            
            BeginPaint(hWnd, &l_psPaint);
            FillRect(l_psPaint.hdc, &l_psPaint.rcPaint, (HBRUSH)GetStockObject(GRAY_BRUSH));

            FillRect(l_psPaint.hdc, &rcCaption, hCaptionBrush);

            l_hGdiObj = SelectObject(l_psPaint.hdc, (HBRUSH)GetStockObject(WHITE_PEN));
            
            MoveToEx(l_psPaint.hdc, 0, 0, NULL);
            LineTo(l_psPaint.hdc, nWidth-1, 0);
            MoveToEx(l_psPaint.hdc, 0, 0, NULL);
            LineTo(l_psPaint.hdc, 0, nHeight-1);

            SelectObject(l_psPaint.hdc, (HBRUSH)GetStockObject(BLACK_PEN));

            MoveToEx(l_psPaint.hdc, nWidth-1, nHeight-1, NULL);
            LineTo(l_psPaint.hdc, 0, nHeight-1);
            MoveToEx(l_psPaint.hdc, nWidth-1, nHeight-1, NULL);
            LineTo(l_psPaint.hdc, nWidth-1, 0);

            SelectObject(l_psPaint.hdc, l_hGdiObj);

            EndPaint(hWnd, &l_psPaint);
            lRet = 0;
        }
        break;

        case WM_COMMAND:
            if (wParam == IDCLOSE)
            {
                DestroyWindow(hWnd);
                lRet = 0;
                bCallNormal = 0;
            }
            break;

        case WM_NCCALCSIZE:
            if (wParam == TRUE)
            {
                lRet = 0;
                // No need to pass the message on to the DefWindowProc.
                bCallNormal = 0;
            }
            break;

        case WM_NCHITTEST:
        {
            static int l_screenX, l_screenY;
            l_screenX = GET_X_LPARAM(lParam);
            l_screenY = GET_Y_LPARAM(lParam);

            if (l_screenX == rcWindow.left)
            {
                if (l_screenY == rcWindow.top)
                {
                    lRet = HTTOPLEFT;
                }
                else if (l_screenY > rcWindow.top && l_screenY < rcWindow.bottom)
                {
                    lRet = HTLEFT;
                }
                else if (l_screenY == rcWindow.bottom)
                {
                    lRet = HTBOTTOMLEFT;
                }
            }
            else if (l_screenX > rcWindow.left && l_screenX < rcWindow.right)
            {
                if (l_screenY == rcWindow.top)
                {
                    lRet = HTTOP;
                }
                else if (l_screenY == rcWindow.bottom)
                {
                    lRet = HTBOTTOM;
                }
            }
            else if (l_screenX == rcWindow.right)
            {
                if (l_screenY == rcWindow.top)
                {
                    lRet = HTTOPRIGHT;
                }
                else if (l_screenY > rcWindow.top && l_screenY < rcWindow.bottom)
                {
                    lRet = HTRIGHT;
                }
                else if (l_screenY == rcWindow.bottom)
                {
                    lRet = HTBOTTOMRIGHT;
                }
            }
            else
            {
                lRet = HTNOWHERE;
            }
            bCallNormal = 0;
        }
        break;
        
        case WM_SIZE:
        case WM_SIZING:
            GetWindowRect(hWnd, &rcWindow);
            nWidth = rcWindow.right -rcWindow.left;
            nHeight = rcWindow.bottom -rcWindow.top;

            MoveWindow(hWndClose, nWidth-CAP_WIDTH, 3, CAP_WIDTH-5, CAP_WIDTH-5, FALSE);
            InvalidateRect(hWnd, NULL, TRUE);
            lRet = 0;
            break;
    }
    
    if (bCallNormal)
    {
        switch (uMsg)
        {
            case WM_PAINT:
                break;

            case WM_DESTROY:
                PostQuitMessage(0);
                DeleteObject(hCaptionBrush);
                lRet= 0;
                break;
        }
        lRet = DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }

    return lRet;
}