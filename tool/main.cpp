#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Commctrl.h>
#include <commdlg.h>
#include <stdlib.h>
#include <objbase.h>
#include <shlobj_core.h>
#include <string>
#include <objidl.h>
#include <Gdiplus.h>

using namespace Gdiplus;

struct PictureData {

	WCHAR pszFolderDir[MAX_PATH];
	HWND hWndPicture;
	UINT uCurIndex;
	BYTE openType;
};

struct WindowFrameData {

	int cxMax,
			cyMax;
};

struct PictureWindowInitData {
	WCHAR *pszFileName;
	int cxOffset,
			cyOffset;
};

struct PictureWindowData {
	
	Image *pImage;
	int cxMax,
			cyMax,
		cxOffset,
			cyOffset;
};

#define B_NONE 		(BYTE)0
#define B_FILE		(BYTE)1
#define B_FOLDER 	(BYTE)2

#define WM_SETVIEW	(WM_USER+0x100)

#define IDC_TAB		(HMENU)0x100
#define IDC_TABWND	(HMENU)0x101

#define IDC_CHOOSEFONT 	(HMENU)0x102
#define IDC_FONTCOPY	(HMENU)0x103
#define IDC_EDITCPY		(HMENU)0x104

#define IDC_EDITWIDTH	(HMENU)0x105
#define IDC_EDITHEIGHT	(HMENU)0x106
#define IDC_BUTTONWND	(HMENU)0x107

#define IDC_CHOOSEFILE			(HMENU)0x108
#define IDC_CHOOSEFOLDER		(HMENU)0x109
#define IDC_STATICCHOOSE		(HMENU)0x10A
#define IDC_STATICCHOOSEINFO	(HMENU)0x10B
#define IDC_FILELISTVIEW		(HMENU)0x10C

#define IDC_STATICWIDTH			(HMENU)0x10D
#define IDC_STATICHEIGHT		(HMENU)0x10E
#define IDC_STATICXOFFSET		(HMENU)0x10F
#define IDC_STATICYOFFSET		(HMENU)0x110
#define IDC_EDITXOFFSET			(HMENU)0x111
#define IDC_EDITYOFFSET			(HMENU)0x112
#define IDC_OPENPICTURE			(HMENU)0x113
#define IDC_STATICIMGGROUPCTL	(HMENU)0x114

#define IDI_FONT	0
#define IDI_WINDOW	1
#define IDI_PICTURE	2

const PWSTR pszWindowName = L"tool";
LRESULT WINAPI MainWindowProc(HWND, UINT, WPARAM, LPARAM);

const PWSTR pszFontWindowName = L"win32_Font",
	pszWindowFrameWindowName = L"win32_WindowFrame",
	pszPictureWindowName = L"win32_Picture",
	pszWindowFramePopupWindowName = L"win32_WindowFramePopup",
	pszPicturePopupWindowName = L"win32_PicturePopup";
LRESULT WINAPI FontWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI WindowFrameWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI PictureWindowProc(HWND, UINT, WPARAM, LPARAM);

LRESULT WINAPI WindowFrameProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK BrowsePictureFolderCallbackProc(HWND, UINT, LPARAM, LPARAM);
LRESULT WINAPI PicturePopupWindowProc(HWND, UINT, WPARAM, LPARAM);

const std::wstring codeNewLine = std::wstring(L";\r\n");

ATOM pszPictureClassName = 0,
	pszWindowFrameClassName = 0,
	pszFontClassName = 0,
	pszWindowFramePopupClassName = 0,
	pszPicturePopupClassName = 0;

int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nShowCmd) {

	WNDCLASSEXW wcex = { 0 };
	ATOM pszClassName = 0;
	HWND hWnd = NULL;
	MSG Msg = { 0 };
	GdiplusStartupInput gdiStartupInput = { 0 };
	ULONG_PTR gdiplusToken = 0L;

	CoInitialize(NULL);
	GdiplusStartup(&gdiplusToken, &gdiStartupInput, NULL);

	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.lpfnWndProc = MainWindowProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(NULL, (PWSTR)IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, (PWSTR)IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszClassName = pszWindowName;
	wcex.hIconSm = LoadIconW(NULL, (PWSTR)IDI_APPLICATION);

	pszClassName = RegisterClassExW(&wcex);

	if (pszClassName) {

		hWnd = CreateWindowExW(0, (PWSTR)(DWORD64)pszClassName, pszWindowName, WS_VISIBLE
			| WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			600, 400,
			NULL,
			NULL,
			hInstance,
			NULL);

		if (hWnd) {

			while (GetMessageW(&Msg, NULL, 0, 0)) {

				TranslateMessage(&Msg);
					DispatchMessageW(&Msg);
			}
		}
	}

	GdiplusShutdown(gdiplusToken);
	return 0;
}


LRESULT WINAPI MainWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			HWND hWndTab = NULL;
			RECT  rcClient = { 0 };
			TCITEMW tcItemW = { 0 };

			WNDCLASSEXW wcex = { 0 };

			pCSw = (LPCREATESTRUCTW)lParam;
				GetClientRect(hWnd, &rcClient);
			hWndTab = CreateWindowExW(0, WC_TABCONTROLW, NULL, WS_VISIBLE | WS_CHILD, 0, 0,
				rcClient.right, rcClient.bottom,
				hWnd,
				IDC_TAB,
				pCSw->hInstance,
				NULL);

			tcItemW.mask = TCIF_TEXT;

			tcItemW.pszText = L"Picture";
			SendMessageW(hWndTab, TCM_INSERTITEMW, (WPARAM)0, (LPARAM)&tcItemW);

			tcItemW.pszText = L"Window";
			SendMessageW(hWndTab, TCM_INSERTITEMW, (WPARAM)0, (LPARAM)&tcItemW);

			tcItemW.pszText = L"Font";
			SendMessageW(hWndTab, TCM_INSERTITEMW, (WPARAM)0, (LPARAM)&tcItemW);
			SendMessageW(hWndTab, TCM_SETCURSEL, (WPARAM)0, (LPARAM)0);

			wcex.cbSize = sizeof(WNDCLASSEXW);
			wcex.lpfnWndProc = PictureWindowProc;
			wcex.hInstance = pCSw->hInstance;
			wcex.hIcon = LoadIconW(NULL, (PWSTR)IDI_APPLICATION);
			wcex.hCursor = LoadCursorW(NULL, (PWSTR)IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			wcex.lpszClassName = pszPictureWindowName;
			wcex.hIconSm = LoadIconW(NULL, (PWSTR)IDI_APPLICATION);

			pszPictureClassName = RegisterClassExW(&wcex);

			wcex.lpfnWndProc = WindowFrameWindowProc;
			wcex.lpszClassName = pszWindowFrameWindowName;
			pszWindowFrameClassName = RegisterClassExW(&wcex);

			wcex.lpfnWndProc = FontWindowProc;
			wcex.lpszClassName = pszFontWindowName;
			pszFontClassName = RegisterClassExW(&wcex);

			wcex.lpfnWndProc = WindowFrameProc;
			wcex.lpszClassName = pszWindowFramePopupWindowName;
			pszWindowFramePopupClassName = RegisterClassExW(&wcex);

			wcex.lpfnWndProc = PicturePopupWindowProc;
			wcex.lpszClassName = pszPicturePopupWindowName;
			pszPicturePopupClassName = RegisterClassExW(&wcex);

			SendMessageW(hWnd, WM_SETVIEW, (WPARAM)0, (LPARAM)pszFontClassName);
		}
		return 0;

		case WM_NOTIFY: {

			NMHDR *pnmHdr = (LPNMHDR)lParam;

			switch (pnmHdr->idFrom) {

				case IDC_TAB: {

					int iItem = SendMessageW(pnmHdr->hwndFrom, TCM_GETCURSEL, (WPARAM)0, (LPARAM)0);

					switch (iItem) {

						case IDI_FONT: {

							SendMessageW(hWnd, WM_SETVIEW, (WPARAM)0, (LPARAM)pszFontClassName);
						}
						break;

						case IDI_WINDOW: {

							SendMessageW(hWnd, WM_SETVIEW, (WPARAM)0, (LPARAM)
								pszWindowFrameClassName);
						}
						break;

						case IDI_PICTURE: {

							SendMessageW(hWnd, WM_SETVIEW, (WPARAM)0, (LPARAM)pszPictureClassName);
						}
						break;
					}
				}
				break;
			}
		}
		return 0;

		case WM_SETVIEW: {

			HWND hWndTab = GetDlgItem(hWnd, (int)IDC_TAB);
			RECT rcTab = { 0 };

			GetClientRect(hWndTab, &rcTab);
			SendMessageW(hWndTab, TCM_ADJUSTRECT, (WPARAM)FALSE, (LPARAM)&rcTab);
			
			DestroyWindow(GetDlgItem(hWndTab, (int)IDC_TABWND));
			CreateWindowExW(0, (PWSTR)lParam, NULL, WS_VISIBLE | WS_CHILD, rcTab.left, rcTab.top,
				rcTab.right - rcTab.left,
					rcTab.bottom - rcTab.top,
				hWndTab,
				IDC_TABWND,
				(HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE),
				NULL);
		}
		return 0;

		case WM_GETMINMAXINFO: {

			PMINMAXINFO pmmi = NULL;

			pmmi = (PMINMAXINFO)lParam;

			pmmi->ptMaxSize.x = 600;
				pmmi->ptMaxSize.y = 400;
			pmmi->ptMinTrackSize.x = 600;
				pmmi->ptMinTrackSize.y = 400;
			pmmi->ptMaxTrackSize.x = 600;
				pmmi->ptMaxTrackSize.y = 400;
		}
		return 0;

		case WM_DESTROY: {

			PostQuitMessage(0);
		}
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT FontWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			HWND hWndChooseButton = NULL;

			pCSw = (LPCREATESTRUCTW)lParam;
			CreateWindowExW(0, WC_BUTTONW, L"Choose Font", WS_VISIBLE | WS_CHILD, 10, 10, 100, 25,
				hWnd,
				IDC_CHOOSEFONT,
				pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_BUTTONW, L"Copy", WS_VISIBLE | WS_CHILD, pCSw->cx - 110, 10,
				100, 25,
				hWnd,
				IDC_FONTCOPY,
				pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_EDITW, NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_MULTILINE
				| ES_READONLY,
				10, 45,
				pCSw->cx - 20, pCSw->cy - 65,
				hWnd,
				IDC_EDITCPY,
				pCSw->hInstance,
				NULL);
		}
		return 0;


		case WM_COMMAND: {

			switch (LOWORD(wParam)) {

				case (int)IDC_CHOOSEFONT: {

					CHOOSEFONTW chooseFontW = { 0 };
					LOGFONTW lgFontw = { 0 };

					chooseFontW.lStructSize = sizeof(CHOOSEFONTW);
					chooseFontW.hwndOwner = GetParent(GetParent(hWnd));
					chooseFontW.lpLogFont = &lgFontw;
					chooseFontW.Flags = CF_SCREENFONTS;
					chooseFontW.nFontType = SCREEN_FONTTYPE;

					if (ChooseFontW(&chooseFontW)) {

						std::wstring wstr;
						wstr += std::wstring(L"LOGFONTW lgFontw = { 0 };\r\n");
						
						if (lgFontw.lfHeight)
							wstr += std::wstring(L"lgFontw.lfHeight = ") + std::to_wstring(
								lgFontw.lfHeight) +
								codeNewLine;
						
						if (lgFontw.lfEscapement)
							wstr += std::wstring(L"lgFontw.lfEscapement = ") + std::to_wstring(
								lgFontw.lfEscapement) +
								codeNewLine;

						if (lgFontw.lfOrientation)
							wstr += std::wstring(L"lgFontw.lfOrientation = ") + std::to_wstring(
								lgFontw.lfOrientation) +
								codeNewLine;

						if (lgFontw.lfWeight)
							wstr += std::wstring(L"lgFontw.lfWeight = ") + std::to_wstring(lgFontw
								.lfWeight) +
								codeNewLine;

						if (lgFontw.lfCharSet)
							wstr += std::wstring(L"lgFontw.lfCharSet = ") + std::to_wstring(lgFontw
								.lfCharSet) +
								codeNewLine;

						if (lgFontw.lfOutPrecision)
							wstr += std::wstring(L"lgFontw.lfOutPrecision = ") + std::to_wstring(
								lgFontw.lfOutPrecision) +
								codeNewLine;

						if (lgFontw.lfClipPrecision)
							wstr += std::wstring(L"lgFontw.lfClipPrecision = ") + std::to_wstring(
								lgFontw.lfClipPrecision) +
								codeNewLine;

						if (lgFontw.lfQuality)
							wstr += std::wstring(L"lgFontw.lfQuality = ") + std::to_wstring(
								lgFontw.lfQuality) +
								codeNewLine;

						if (lgFontw.lfPitchAndFamily)
							wstr += std::wstring(L"lgFontw.lfPitchAndFamily = ") + std::to_wstring(
								lgFontw.lfPitchAndFamily) +
								codeNewLine;

						wstr += std::wstring(L"wcscpy_s(lgFontw.lfFaceName, \"") + std::wstring(
							lgFontw.lfFaceName) +
							std::wstring(L"\");\r\n");


						SetWindowTextW(GetDlgItem(hWnd, (int)IDC_EDITCPY), wstr.c_str());
					}
				}
				break;

				case (int)IDC_FONTCOPY: {

					HWND hWndEditCpy = NULL;
					HANDLE hGlobalCpy = NULL;
					int nCount = 0;
					PWSTR pszGlobalCpy = NULL;

					hWndEditCpy = GetDlgItem(hWnd,(int)IDC_EDITCPY);
					OpenClipboard(GetParent(GetParent(hWnd)));
					EmptyClipboard();

					nCount = GetWindowTextLengthW(hWndEditCpy) + 1;

					hGlobalCpy = GlobalAlloc(GMEM_MOVEABLE, nCount * sizeof(WCHAR));
					pszGlobalCpy = (PWSTR)GlobalLock(hGlobalCpy);

					GetWindowTextW(hWndEditCpy, pszGlobalCpy, nCount);
					GlobalUnlock(hGlobalCpy);

					SetClipboardData(CF_UNICODETEXT, hGlobalCpy);
					
					CloseClipboard();
				}
				break;
			}
		}
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI WindowFrameWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			HWND hWndChild = NULL;
			HDC hDCWndChild = NULL;
			SIZE szWndChild = { 0 };
			int nMaxLenSize = 0,
				nMaxBredSize = 0;

			pCSw = (LPCREATESTRUCTW)lParam;
			hWndChild = CreateWindowExW(0, WC_STATICW, L"Width:", WS_VISIBLE | WS_CHILD | SS_LEFT,
				10, 10,
				0, 0,
				hWnd,
				NULL,
				pCSw->hInstance,
				NULL);

			hDCWndChild = GetDC(hWndChild);
			GetTextExtentPoint32W(hDCWndChild, L"Width:", 6, &szWndChild);
			ReleaseDC(hWnd, hDCWndChild);

			SetWindowPos(hWndChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy, SWP_NOZORDER |
				SWP_NOMOVE);

			hWndChild = CreateWindowExW(0, WC_STATICW, L"Height:", WS_VISIBLE | WS_CHILD | SS_LEFT,
				10, 20 + szWndChild.cy,
				0, 0,
				hWnd,
				NULL,
				pCSw->hInstance,
				NULL);

			nMaxLenSize = szWndChild.cx;
			nMaxBredSize = szWndChild.cy;

			hDCWndChild = GetDC(hWndChild);
			GetTextExtentPoint32W(hDCWndChild, L"Height:", 7, &szWndChild);
			ReleaseDC(hWnd, hDCWndChild);

			SetWindowPos(hWndChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy, SWP_NOZORDER |
				SWP_NOMOVE);


			if (nMaxLenSize < szWndChild.cx) nMaxLenSize = szWndChild.cx;
			if (nMaxBredSize < szWndChild.cy) nMaxBredSize = szWndChild.cy;

			CreateWindowExW(0, WC_EDITW, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				20 + nMaxLenSize, 9,
				100, nMaxBredSize + 2,
				hWnd,
				IDC_EDITWIDTH,
				pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_EDITW, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER| ES_NUMBER,
				20 + nMaxLenSize, szWndChild.cy + 19,
				100, nMaxBredSize + 2,
				hWnd,
				IDC_EDITHEIGHT,
				pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_BUTTONW, L"Open Window", WS_VISIBLE | WS_CHILD, pCSw->cx - 120,
				10,
				100, 25,
				hWnd,
				IDC_BUTTONWND,
				pCSw->hInstance,
				NULL);
		}
		return 0;

		case WM_COMMAND: {

			switch (LOWORD(wParam)) {

				case (int)IDC_BUTTONWND: {

					switch (HIWORD(wParam)) {

						case BN_CLICKED: {

							HWND hWndEditWidth = NULL,
									hWndEditHeight = NULL;
							int nCount = 0,
									nWidth = 0,
								nHeight = 0,
								nMaxWidth = 0,
								nMaxHeight = 0;
							PWSTR strBuf = NULL;

							nMaxWidth = GetSystemMetrics(SM_CXSCREEN);
							nMaxHeight = GetSystemMetrics(SM_CYSCREEN);

							hWndEditWidth = GetDlgItem(hWnd, (int)IDC_EDITWIDTH);
							hWndEditHeight = GetDlgItem(hWnd, (int)IDC_EDITHEIGHT);

							nCount = GetWindowTextLengthW(hWndEditWidth) + 1;
							strBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, nCount * sizeof(WCHAR));
							GetWindowTextW(hWndEditWidth, strBuf, nCount);

							nWidth = _wtoi(strBuf);
							HeapFree(GetProcessHeap(), 0, (LPVOID)strBuf);

							nCount = GetWindowTextLengthW(hWndEditHeight) + 1;
							strBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, nCount * sizeof(WCHAR));
							GetWindowTextW(hWndEditHeight, strBuf, nCount);

							nHeight = _wtoi(strBuf);
							HeapFree(GetProcessHeap(), 0, (LPVOID)strBuf);

							if (nWidth > 0 && nWidth <= nMaxWidth &&
									nHeight > 0 && nHeight <= nMaxHeight) {

								CreateWindowExW(0, (PWSTR)(DWORD64)pszWindowFramePopupClassName,
									L"Window Popup",
									WS_VISIBLE | WS_OVERLAPPEDWINDOW,
									CW_USEDEFAULT, CW_USEDEFAULT,
									nWidth, nHeight,
									NULL,
									NULL,
									(HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE),
									NULL);
							}
						}
						break;
					}
				}
				break;
			}
		}
		return 0;

		case WM_CTLCOLORSTATIC: return (LRESULT)GetStockObject(WHITE_BRUSH);
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI PictureWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			HWND hWndChild = NULL,
				hWndChildChild = NULL;
			HDC hDCWndChild = NULL;
			SIZE szWndChild = { 0 };
			PictureData *pPictureData = NULL;

			int nMaxLenSize = 0,
				nMaxBredSize = 0,
				nBred = 10,
				temp = 0;

			pCSw = (LPCREATESTRUCTW)lParam;
			CreateWindowExW(0, WC_BUTTONW, L"Choose Picture", WS_VISIBLE | WS_CHILD, 10, 10,
				110, 25,
				hWnd,
				IDC_CHOOSEFILE,
				pCSw->hInstance,
				NULL);

			hWndChild = CreateWindowExW(0, WC_STATICW, L"or", WS_VISIBLE | WS_CHILD, 130, 10, 0, 0,
				hWnd,
				NULL,
				pCSw->hInstance,
				NULL);

			hDCWndChild = GetDC(hWndChild);
			GetTextExtentPoint32W(hDCWndChild, L"or", 2, &szWndChild);
			ReleaseDC(hWndChild, hDCWndChild);

			SetWindowPos(hWndChild, NULL, 130, 23 - szWndChild.cy/2, szWndChild.cx, szWndChild.cy,
				SWP_NOZORDER);

			CreateWindowExW(0, WC_BUTTONW, L"Choose Folder", WS_VISIBLE | WS_CHILD, 140 +
				szWndChild.cx, 10,
				110, 25,
				hWnd,
				IDC_CHOOSEFOLDER,
				pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_STATICW, NULL, WS_CHILD | WS_VISIBLE, 10, 45, 0, 0, hWnd,
				IDC_STATICCHOOSE,
				pCSw->hInstance,
				NULL);
			CreateWindowExW(0, WC_STATICW, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL, 10, 45,
				pCSw->cx - 20, 30,
				hWnd,
				IDC_STATICCHOOSEINFO,
				pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_LISTVIEWW, NULL, WS_CHILD | WS_BORDER | LVS_LIST, 10, 45,
				pCSw->cx - 20, 90,
				hWnd,
				IDC_FILELISTVIEW,
				pCSw->hInstance,
				NULL);

			hWndChild = CreateWindowExW(0, WC_STATICW, NULL, WS_CHILD | WS_VISIBLE | SS_LEFT
				| SS_WHITERECT,
				0, 75,
				pCSw->cx, 0,
				hWnd,
				IDC_STATICIMGGROUPCTL,
				pCSw->hInstance,
				NULL);

			hWndChildChild = CreateWindowExW(0, WC_STATICW, L"X Offset:", WS_CHILD | WS_VISIBLE
				| SS_LEFT,
				10, 10,
				0, 0,
				hWndChild,
				IDC_STATICXOFFSET,
				pCSw->hInstance,
				NULL);

			hDCWndChild = GetDC(hWndChildChild);
			GetTextExtentPoint32W(hDCWndChild, L"X Offset:", 9, &szWndChild);
			ReleaseDC(hWndChildChild, hDCWndChild);

			nMaxLenSize = szWndChild.cx;
			nMaxBredSize = szWndChild.cy;
			nBred += nMaxBredSize + 10;

			SetWindowPos(hWndChildChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy, SWP_NOZORDER
				| SWP_NOMOVE);

			hWndChildChild = CreateWindowExW(0, WC_STATICW, L"Y Offset:", WS_CHILD | WS_VISIBLE
				| SS_LEFT,
				10, nBred + 10,
				0, 0,
				hWndChild,
				IDC_STATICYOFFSET,
				pCSw->hInstance,
				NULL);

			hDCWndChild = GetDC(hWndChildChild);
			GetTextExtentPoint32W(hDCWndChild, L"Y Offset:", 9, &szWndChild);
			ReleaseDC(hWndChildChild, hDCWndChild);

			SetWindowPos(hWndChildChild, NULL, 10, nBred, szWndChild.cx, szWndChild.cy, SWP_NOZORDER);

			if (nMaxLenSize < szWndChild.cx) nMaxLenSize = szWndChild.cx;
			if (nMaxBredSize < szWndChild.cy) nMaxBredSize = szWndChild.cy;
			
			nBred += nMaxBredSize+1;
			SetWindowPos(hWndChild, NULL, 0, 0, pCSw->cx, nBred, SWP_NOZORDER | SWP_NOMOVE);

			CreateWindowExW(0, WC_EDITW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
				20 + nMaxLenSize, 9,
				100, 2 + nMaxBredSize,
				hWndChild,
				IDC_EDITXOFFSET,
					pCSw->hInstance,
				NULL);

			CreateWindowExW(0, WC_EDITW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
				20 + nMaxLenSize, 19 + szWndChild.cy,
				100, 2 + nMaxBredSize,
				hWndChild,
				IDC_EDITYOFFSET,
					pCSw->hInstance,
				NULL);

			hWndChildChild = CreateWindowExW(0, WC_STATICW, L"Width:", WS_CHILD | WS_VISIBLE
				| SS_LEFT,
				130 + nMaxLenSize, 10,
				0, 0,
				hWndChild,
				IDC_STATICWIDTH,
					pCSw->hInstance,
				NULL);

			temp = szWndChild.cy;

			hDCWndChild = GetDC(hWndChildChild);
			GetTextExtentPoint32W(hDCWndChild, L"Width:", 6, &szWndChild);
			ReleaseDC(hWndChildChild, hDCWndChild);

			SetWindowPos(hWndChildChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy, SWP_NOZORDER
				| SWP_NOMOVE);

			hWndChildChild = CreateWindowExW(0, WC_STATICW, L"Height:", WS_CHILD | WS_VISIBLE
				| SS_LEFT,
				130 + nMaxLenSize, 19 + temp,
				0, 0,
				hWndChild,
				IDC_STATICHEIGHT,
				pCSw->hInstance,
				NULL);

			hDCWndChild = GetDC(hWndChildChild);
			GetTextExtentPoint32W(hDCWndChild, L"Height:", 7, &szWndChild);
			ReleaseDC(hWndChildChild, hDCWndChild);

			SetWindowPos(hWndChildChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy, SWP_NOZORDER
				| SWP_NOMOVE);

			CreateWindowExW(0, WC_BUTTONW, L"Open Picture", WS_CHILD | WS_VISIBLE, pCSw->cx - 110,
				pCSw->cy - 35,
				100, 25,
				hWnd,
				IDC_OPENPICTURE,
					pCSw->hInstance,
				NULL);

			pPictureData = new PictureData;
			pPictureData->uCurIndex = -1;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pPictureData);
		}
		return 0;

		case WM_NOTIFY: {

			NMHDR *pnmHdr = (NMHDR *)lParam;

			switch (pnmHdr->idFrom) {

				case (int)IDC_FILELISTVIEW: {

					switch (pnmHdr->code) {

						case NM_CLICK: {

							NMITEMACTIVATE *pnmItem = NULL;
							LVITEMW lvItemW = { 0 };
							PictureData *pPictureData = NULL;
							WCHAR pszFileName[MAX_PATH] = { 0 };
							std::wstring folderPath,
								strText;
							HWND hWndGrp = NULL,
								hWndChildGrp = NULL;
							HDC hDCWndChild = NULL;
							SIZE szWndChild = { 0 };

							pnmItem = (NMITEMACTIVATE *)lParam;
							pPictureData = (PictureData *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

							if (-1 != pnmItem->iItem) {

								pPictureData->uCurIndex = pnmItem->iItem;

								lvItemW.mask = LVIF_TEXT;
								lvItemW.iItem = pnmItem->iItem;
								lvItemW.pszText = pszFileName;
									lvItemW.cchTextMax = MAX_PATH;
								
								SendMessageW(pnmItem->hdr.hwndFrom, LVM_GETITEMW, (WPARAM)0,
									(LPARAM)&lvItemW);
								
								folderPath = std::wstring(pPictureData->pszFolderDir) +
									std::wstring(L"\\") +
									std::wstring(pszFileName);

								Image image(folderPath.c_str(), TRUE);
								Status status = image.GetLastStatus();

								hWndGrp = GetDlgItem(hWnd,	(int)IDC_STATICIMGGROUPCTL);

								if (Ok == status) {

									hWndChildGrp = GetDlgItem(hWndGrp,(int)IDC_STATICWIDTH);
									strText = std::wstring(L"Width: ") + std::to_wstring(image
										.GetWidth());

									SetWindowTextW(hWndChildGrp, strText.c_str());

									hDCWndChild = GetDC(hWndChildGrp);
									GetTextExtentPoint32W(hDCWndChild, strText.c_str(),
										strText.length(),
										&szWndChild);

									SetWindowPos(hWndChildGrp, NULL, 0, 0, szWndChild.cx,
										szWndChild.cy,
										SWP_NOZORDER | SWP_NOMOVE);

									hWndChildGrp = GetDlgItem(hWndGrp,(int)IDC_STATICHEIGHT);
									strText = std::wstring(L"Height: ") + std::to_wstring(image
										.GetHeight());

									SetWindowTextW(hWndChildGrp, strText.c_str());

									hDCWndChild = GetDC(hWndChildGrp);
									GetTextExtentPoint32W(hDCWndChild, strText.c_str(),
										strText.length(),
										&szWndChild);

									SetWindowPos(hWndChildGrp, NULL, 0, 0, szWndChild.cx,
										szWndChild.cy,
										SWP_NOZORDER | SWP_NOMOVE);
								}
							} else {

								hWndGrp = GetDlgItem(hWnd, (int)IDC_STATICIMGGROUPCTL);

								SetWindowTextW(GetDlgItem(hWndGrp, (int)IDC_STATICWIDTH),
									L"Width:");
								SetWindowTextW(GetDlgItem(hWndGrp, (int)IDC_STATICHEIGHT),
									L"Height:");
							}
						}
						break;
					}
				}
			}
		}
		return 0;

		case WM_COMMAND: {

			switch (LOWORD(wParam)) {

				case (int)IDC_CHOOSEFILE: {

					OPENFILENAMEW ofnW = { 0 };
					WCHAR pszFilename[MAX_PATH] = { 0 };

					SIZE szWndChild = { 0 };
					HWND hWndChild = NULL;
					HDC hDCWndChild = NULL;

					ofnW.lStructSize = sizeof(OPENFILENAMEW);
					ofnW.hwndOwner = GetParent(GetParent(hWnd));
					ofnW.lpstrFile = pszFilename;
					ofnW.nMaxFile = MAX_PATH;

					if (GetOpenFileNameW(&ofnW)) {

						hWndChild = GetDlgItem(hWnd, (int)IDC_STATICCHOOSE);
						SetWindowTextW(hWndChild, L"File Path:");

						hDCWndChild = GetDC(hWndChild);
						GetTextExtentPoint32W(hDCWndChild, L"File Path:", 10, &szWndChild);
						ReleaseDC(hWndChild, hDCWndChild);

						SetWindowPos(hWndChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy,
							SWP_NOZORDER | SWP_NOMOVE);

						hWndChild = GetDlgItem(hWnd, (int)IDC_STATICCHOOSEINFO);
						SetWindowTextW(hWndChild, pszFilename);

						SetWindowPos(hWndChild, NULL, 10, 55 + szWndChild.cy, 0, 0, SWP_NOZORDER
							| SWP_NOSIZE);

						hWndChild = GetDlgItem(hWnd, (int)IDC_FILELISTVIEW);
						ShowWindow(hWndChild, SW_HIDE);
					}
				}
				break;

				case (int)IDC_CHOOSEFOLDER: {

					BROWSEINFOW biw = { 0 };
					PictureData *pPictureData = NULL;
					WCHAR pszExt[MAX_PATH] = { 0 };
					PIDLIST_ABSOLUTE pidl = NULL;
					DWORD dwrgf = 0;
					
					SIZE szWndChild = { 0 };
					HWND hWndChild = NULL;
					HDC hDCWndChild = NULL;

					std::wstring pszDir;
						HANDLE hFind = NULL;
					WIN32_FIND_DATAW win32FA = { 0 };
					wchar_t *pChar = NULL;
					int nLen = 0;
					LVITEMW lvItemW = { 0 };

					pPictureData = (PictureData *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
					pPictureData->openType = B_FOLDER;
					pPictureData->uCurIndex = -1;

					biw.hwndOwner = GetParent(GetParent(hWnd));
					biw.pszDisplayName = pPictureData->pszFolderDir;
					biw.lpszTitle = L"Choose Picture Folder";
					biw.ulFlags = BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
					biw.lpfn = BrowsePictureFolderCallbackProc;

					pidl = SHBrowseForFolderW(&biw);
					if (SHGetPathFromIDListW(pidl, pPictureData->pszFolderDir)) {

						hWndChild = GetDlgItem(hWnd, (int)IDC_STATICCHOOSE);
						SetWindowTextW(hWndChild, L"Folder Path:");
						
						hDCWndChild = GetDC(hWndChild);
						GetTextExtentPoint32W(hDCWndChild, L"Folder Path:", 12, &szWndChild);
						ReleaseDC(hWndChild, hDCWndChild);

						SetWindowPos(hWndChild, NULL, 0, 0, szWndChild.cx, szWndChild.cy,
							SWP_NOZORDER | SWP_NOMOVE);

						hWndChild = GetDlgItem(hWnd, (int)IDC_STATICCHOOSEINFO);
						SetWindowTextW(hWndChild, pPictureData->pszFolderDir);

						SetWindowPos(hWndChild, NULL, 10, 55 + szWndChild.cy, 0, 0, SWP_NOZORDER
							| SWP_NOSIZE);

						hWndChild = GetDlgItem(hWnd, (int)IDC_FILELISTVIEW);
						ShowWindow(hWndChild, SW_SHOW);
						SetWindowPos(hWndChild, NULL, 10, 95 + szWndChild.cy, 0, 0, SWP_NOZORDER
							| SWP_NOSIZE);
						SendMessageW(hWndChild, LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0);

						hWndChild = GetDlgItem(hWnd, (int)IDC_STATICIMGGROUPCTL);
						SetWindowPos(hWndChild, NULL, 0, 195 + szWndChild.cy, 0, 0, SWP_NOZORDER
							| SWP_NOSIZE);

						lvItemW.mask = LVIF_TEXT;

						pszDir = std::wstring(pPictureData->pszFolderDir) + std::wstring(L"\\*");
						hFind = FindFirstFileW((PWSTR)pszDir.c_str(), &win32FA);

						hWndChild = GetDlgItem(hWnd, (int)IDC_FILELISTVIEW);

						if (INVALID_HANDLE_VALUE != hFind) {

							do {

								if (!(win32FA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

									nLen = wcsnlen_s(win32FA.cFileName, MAX_PATH);
									pChar = wcsrchr(win32FA.cFileName, L'.');

									if (pChar && nLen > (pChar + 1 - win32FA.cFileName)) {

										wcscpy_s(pszExt, pChar + 1);
										nLen = wcsnlen_s(pszExt, MAX_PATH);

										for (int i = 0;
											i < nLen;
											i++) {

											pszExt[i] = towlower(pszExt[i]);
										}

										if (!wcscmp(pszExt, L"bmp") ||
											!wcscmp(pszExt, L"png") ||
											!wcscmp(pszExt, L"jpeg") ||
											!wcscmp(pszExt, L"jpg")) {

											lvItemW.pszText = win32FA.cFileName;
											SendMessageW(hWndChild, LVM_INSERTITEMW, (WPARAM)0,
												(LPARAM)&lvItemW);
										}

									}
								}
							} while (0 != FindNextFileW(hFind, &win32FA));
							FindClose(hFind);
						}
					}
				}
				break;

				case (int)IDC_OPENPICTURE: {

					PictureData *pPictureData = NULL;

					pPictureData = (PictureData *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
					switch (pPictureData->openType) {


						case B_NONE: break;

						case B_FILE: {

							// Handle File ..
						}
						break;

						case B_FOLDER: {

							// Handle Folder ..
							PictureData *pPictureData = NULL;
							PictureWindowInitData picWndInitData = { 0 };
							HWND hWndGrp = NULL;
							UINT nWidth = 0,
									nHeight = 0;
							std::wstring pszPicturePath;
							RECT rcWindow = { 0 };

							WCHAR pszFileName[MAX_PATH] = { 0 };
							LVITEMW lvItemW = { 0 };

							pPictureData = (PictureData *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
							
							if ((UINT)-1 != pPictureData->uCurIndex) {

								lvItemW.mask = LVIF_TEXT;
								lvItemW.iItem = pPictureData->uCurIndex;
								lvItemW.pszText = pszFileName;
									lvItemW.cchTextMax = MAX_PATH;

								SendMessageW(GetDlgItem(hWnd, (int)IDC_FILELISTVIEW), LVM_GETITEMW,
									(WPARAM)0,
									(LPARAM)&lvItemW);

								pszPicturePath = std::wstring(pPictureData->pszFolderDir) + 
									std::wstring(L"//") + 
									std::wstring(pszFileName);

								hWndGrp = GetDlgItem(hWnd, (int)IDC_STATICIMGGROUPCTL);
								
								GetWindowTextW(GetDlgItem(hWndGrp, (int)IDC_STATICWIDTH),
									pszFileName, MAX_PATH);
								nWidth = _wtoi(pszFileName + 7);

								GetWindowTextW(GetDlgItem(hWndGrp, (int)IDC_STATICHEIGHT),
									pszFileName, MAX_PATH);
								nHeight = _wtoi(pszFileName + 8);

								GetWindowTextW(GetDlgItem(hWndGrp, (int)IDC_EDITXOFFSET),
									pszFileName, MAX_PATH);
								picWndInitData.cxOffset = _wtoi(pszFileName);

								GetWindowTextW(GetDlgItem(hWndGrp, (int)IDC_EDITYOFFSET),
									pszFileName, MAX_PATH);
								picWndInitData.cyOffset = _wtoi(pszFileName);

								if (nWidth > 0 &&
									nHeight > 0 &&
									picWndInitData.cxOffset < nWidth &&
									picWndInitData.cyOffset < nHeight) {

									rcWindow.right = nWidth - picWndInitData
											.cxOffset;
									rcWindow.bottom = nHeight- picWndInitData
												.cyOffset;
									AdjustWindowRect(&rcWindow, WS_VISIBLE |
										WS_OVERLAPPEDWINDOW,
										FALSE);

									if (pPictureData->hWndPicture &&
										IsWindow(pPictureData->hWndPicture))
										DestroyWindow(pPictureData->hWndPicture);

									picWndInitData.pszFileName = (WCHAR*)
										pszPicturePath.c_str();

									pPictureData->hWndPicture = CreateWindowExW(0, (PWSTR)
										(DWORD64)pszPicturePopupClassName,
										L"Picture Popup",
										WS_VISIBLE | WS_OVERLAPPEDWINDOW,
										CW_USEDEFAULT, CW_USEDEFAULT,
										rcWindow.right - rcWindow.left,
											rcWindow.bottom - rcWindow.top,
										NULL,
										NULL,
										(HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE),
										(LPVOID)&picWndInitData);
								}
							}
						}
						break;
					}
				}
				break;
			}
		}
		return 0;

		case WM_DESTROY: {

			PictureData *pPictureData = NULL;

			pPictureData = (PictureData *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
			if (IsWindow(pPictureData->hWndPicture))
				DestroyWindow(pPictureData->hWndPicture);

			delete pPictureData;
		}
		break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int CALLBACK BrowsePictureFolderCallbackProc(HWND hWnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData) {

	return 0;
}

LRESULT WINAPI WindowFrameProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			WindowFrameData *pWindowFrameData = NULL;

			pCSw = (LPCREATESTRUCTW)lParam;
			pWindowFrameData = new WindowFrameData;

			pWindowFrameData->cxMax = pCSw->cx;
				pWindowFrameData->cyMax = pCSw->cy;

			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)
				pWindowFrameData);
		}
		return 0;


		case WM_GETMINMAXINFO: {

			PMINMAXINFO pmmi = NULL;
			WindowFrameData *pWindowFrameData = NULL;

			pmmi = (PMINMAXINFO)lParam;
			pWindowFrameData = (WindowFrameData *)GetWindowLongPtrW(
				hWnd, GWLP_USERDATA);

			if (pWindowFrameData) {

				pmmi->ptMaxSize.x = pWindowFrameData->cxMax;
					pmmi->ptMaxSize.y = pWindowFrameData->cyMax;
				pmmi->ptMinTrackSize.x = pWindowFrameData->cxMax;
					pmmi->ptMinTrackSize.y = pWindowFrameData->cyMax;
				pmmi->ptMaxTrackSize.x = pWindowFrameData->cxMax;
					pmmi->ptMaxTrackSize.y = pWindowFrameData->cyMax;
			}
		}
		return 0;

		case WM_DESTROY: {

			WindowFrameData *pWindowFrameData = NULL;

			pWindowFrameData = (WindowFrameData *)GetWindowLongPtrW(hWnd,
				GWLP_USERDATA);
			delete pWindowFrameData;
		}
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI PicturePopupWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			PictureWindowInitData *pPicWndInitData = NULL;
			PictureWindowData  *pPictureWindowData = NULL;

			pCSw = (LPCREATESTRUCTW)lParam;
			pPicWndInitData = (PictureWindowInitData *)pCSw
				->lpCreateParams;

			pPictureWindowData = new PictureWindowData;
			pPictureWindowData->pImage = new Image((const WCHAR*)
				pPicWndInitData->pszFileName, TRUE);

			pPictureWindowData->cxMax = pCSw->cx;
				pPictureWindowData->cyMax = pCSw->cy;

			pPictureWindowData->cxOffset = pPicWndInitData->cxOffset;
				pPictureWindowData->cyOffset = pPicWndInitData->cyOffset;

			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)
				pPictureWindowData);
		}
		return 0;

		case WM_GETMINMAXINFO: {

			PMINMAXINFO pmmi = NULL;
			PictureWindowData *pPictureWindowData = NULL;

			pmmi = (PMINMAXINFO)lParam;
			pPictureWindowData = (PictureWindowData *)GetWindowLongPtrW(
				hWnd, GWLP_USERDATA);

			if (pPictureWindowData) {

				pmmi->ptMaxSize.x = pPictureWindowData->cxMax;
					pmmi->ptMaxSize.y = pPictureWindowData->cyMax;
				pmmi->ptMinTrackSize.x = pPictureWindowData->cxMax;
					pmmi->ptMinTrackSize.y = pPictureWindowData->cyMax;
				pmmi->ptMaxTrackSize.x = pPictureWindowData->cxMax;
					pmmi->ptMaxTrackSize.y = pPictureWindowData->cyMax;
			}
		}
		return 0;

		case WM_PAINT: {


			PAINTSTRUCT ps = { 0 };
			PictureWindowData *pPictureWindowData = NULL;

			pPictureWindowData = (PictureWindowData *)GetWindowLongPtrW(
				hWnd, GWLP_USERDATA);
			Point drawPt(-pPictureWindowData->cxOffset, -pPictureWindowData
				->cyOffset);

			BeginPaint(hWnd, &ps);
			Graphics graphics(ps.hdc);

			graphics.DrawImage(pPictureWindowData->pImage, drawPt);
			EndPaint(hWnd, &ps);
		}
		break;

		case WM_DESTROY: {

			PictureWindowData *pPictureWindowData = NULL;

			pPictureWindowData = (PictureWindowData *)GetWindowLongPtrW(
				hWnd, GWLP_USERDATA);
			delete pPictureWindowData->pImage;
			delete pPictureWindowData;
		}
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}