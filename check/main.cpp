#include "stdafx.h"

#define MAX_PSZCHECK 	100
#define IDC_ADDNEW		0x100
#define IDC_CHANGEVIEW	0x101
#define IDC_DONE		0x102
#define IDC_CHECKSTART	0x103

#define WM_DONE			WM_USER+0x1

#define IDC_STATE_WINDOW	0
#define IDC_STATE_RIGHT		1
#define IDC_STATE_LEFT		2

typedef struct CHECKLIST {

	SYSTEMTIME sysTimeDue;
	WCHAR pszCheck[MAX_PSZCHECK*2+1];

	CHECKLIST(SYSTEMTIME sysTimeDue,
		BYTE *pbwstrStart) : sysTimeDue(sysTimeDue) {

		wcscpy_s(pszCheck, (const wchar_t *)pbwstrStart);
	}

	CHECKLIST(SYSTEMTIME sysTimeDue) : sysTimeDue(sysTimeDue) { }


} CHECKLIST, *PCHECKLIST;

typedef struct APPPERSISTDATA {

	std::list<CHECKLIST> checkList;
	SIZE szWnd;

	ATOM pszClassName;

	HBITMAP hBitmapAlignLeft;
	HBITMAP hBitmapAlignRight;
	HBITMAP hBitmapWindow;
	WINDOWPLACEMENT wpPrev;
} APPPERSISTDATA, *PAPPPERSISTDATA;

const WCHAR pszMainWindowName[] = L"Check",
	pszCheckCtrlWindowName[] = L"Check_CheckCtrl",
	pszCheckDatFilename[] = L"check.dat";

LRESULT WINAPI MainWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI CheckChildWindowProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR AddCheckDialogProc(HWND, UINT, WPARAM, LPARAM);

void LoadFile(std::list<CHECKLIST> &);
void UnLoadFile(std::list<CHECKLIST> &);


int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	INT nShowCmd) {

	INITCOMMONCONTROLSEX icex = { 0 };
	WNDCLASSEXW wcex = { 0 };
	ATOM pszClassName = 0;
	HWND hWnd = NULL;
	RECT rcAdjustRect = { 0 };
	MSG Msg = { 0 };

	icex.dwSize = (DWORD)sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_BAR_CLASSES | ICC_DATE_CLASSES;

	InitCommonControlsEx(&icex);

	wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.lpfnWndProc = MainWindowProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(hInstance, (LPCWSTR)MAKEINTRESOURCEW(IDI_APP));
	wcex.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszClassName = pszMainWindowName;
	wcex.hIconSm = LoadIconW(hInstance, (LPCWSTR)MAKEINTRESOURCEW(IDI_APP));

	pszClassName = RegisterClassExW(&wcex);

	if (pszClassName) {

		rcAdjustRect.right = 325;
		rcAdjustRect.bottom = 600;

		AdjustWindowRect(&rcAdjustRect, WS_OVERLAPPEDWINDOW,
			FALSE);

		hWnd = CreateWindowExW(0, (PWSTR)(DWORD64)pszClassName,
			pszMainWindowName,
			WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_VSCROLL,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rcAdjustRect.right - rcAdjustRect.left,
				rcAdjustRect.bottom - rcAdjustRect.top,
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

	return 0;
}

LRESULT WINAPI MainWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			PAPPPERSISTDATA pAppPersistData = NULL;
				RECT rcClient = { 0 };
			SCROLLINFO si = { 0 };

			WNDCLASSEXW wcex = { 0 };
			int y = 65,
				nIndex = 0;

			HWND hWndButton = NULL;
			SIZE szIdealButton = { 0 };

			pCSw = (LPCREATESTRUCTW)lParam;
			pAppPersistData = new APPPERSISTDATA;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)
				pAppPersistData);

			wcex.cbSize = sizeof(WNDCLASSEXW);
			wcex.lpfnWndProc = CheckChildWindowProc;
			wcex.hInstance = pCSw->hInstance;
			wcex.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
			wcex.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			wcex.lpszClassName = pszCheckCtrlWindowName;
			wcex.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);

			pAppPersistData->pszClassName = RegisterClassExW(&wcex);

			pAppPersistData->szWnd.cx = pCSw->cx;
				pAppPersistData->szWnd.cy = pCSw->cy;
			pAppPersistData->wpPrev.length = sizeof(WINDOWPLACEMENT);

			LoadFile(pAppPersistData->checkList);

			hWndButton = CreateWindowExW(0, WC_BUTTONW, L"Add Task",
				WS_VISIBLE | WS_CHILDWINDOW,
				12, 10,
				0, 0,
				hWnd,
				(HMENU)IDC_ADDNEW,
				pCSw->hInstance,
				NULL);

			SendMessageW(hWndButton, BCM_GETIDEALSIZE, (WPARAM)NULL,
				(LPARAM)&szIdealButton);

			SetWindowPos(hWndButton, NULL, 0, 0, szIdealButton.cx + 20,
				szIdealButton.cy,
				SWP_NOZORDER | SWP_NOMOVE);

			pAppPersistData->hBitmapAlignLeft = (HBITMAP)LoadImageW(
				pCSw->hInstance,
				MAKEINTRESOURCEW(IDB_ALIGNLEFT),
				IMAGE_BITMAP,
				24, 24,
				LR_DEFAULTCOLOR);

			pAppPersistData->hBitmapAlignRight = (HBITMAP)LoadImageW(
				pCSw->hInstance,
				MAKEINTRESOURCEW(IDB_ALIGNRIGHT),
				IMAGE_BITMAP,
				24, 24,
				LR_DEFAULTCOLOR);

			pAppPersistData->hBitmapWindow = (HBITMAP)LoadImageW(pCSw
				->hInstance,
				MAKEINTRESOURCEW(IDB_WINDOW),
				IMAGE_BITMAP,
				24, 24,
				LR_DEFAULTCOLOR);

			hWndButton = CreateWindowExW(0, WC_BUTTONW, NULL, WS_VISIBLE |
				WS_CHILDWINDOW | BS_BITMAP | BS_PUSHLIKE,
				pCSw->cx - 80, 10,
				24, 24,
				hWnd,
				(HMENU)IDC_CHANGEVIEW,
				pCSw->hInstance,
				NULL);

			SendMessageW(hWndButton, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
				(LPARAM)pAppPersistData->hBitmapWindow);

			SetWindowLongPtrW(hWndButton, GWLP_USERDATA, (LONG_PTR)0);

			for (auto it = pAppPersistData->checkList.begin();
				pAppPersistData->checkList.end() != it;
				it++) {
				
				CreateWindowExW(0, (LPCWSTR)(DWORD64)pAppPersistData
					->pszClassName,
					NULL,
					WS_VISIBLE | WS_CHILDWINDOW,
					0, y,
					300, 60,
					hWnd,
					(HMENU)IDC_CHECKSTART + nIndex,
					pCSw->hInstance,
					(LPVOID)&it);

				y += 65;
				nIndex++;
			}

			GetClientRect(hWnd, &rcClient);
			si.fMask = SIF_RANGE;
			si.nMax = rcClient.bottom > y ? 0 :
				(y - rcClient.bottom);

			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		}
		return 0;

		case WM_VSCROLL: {

			SCROLLINFO si = { 0 };
			int yPos = 0;

    	    si.cbSize = sizeof (si);
	        si.fMask  = SIF_ALL;
	        GetScrollInfo (hWnd, SB_VERT, &si);
	        si.fMask = SIF_POS;

	        yPos = si.nPos;
	        switch (LOWORD (wParam))
	        {

	        case SB_TOP:
	            si.nPos = si.nMin;
	            break;
	              
	        case SB_BOTTOM:
	            si.nPos = si.nMax;
	            break;
	              
	        case SB_LINEUP:
	            si.nPos -= 1;
	            break;
	              
	        case SB_LINEDOWN:
	            si.nPos += 1;
	            break;
	              
	        case SB_PAGEUP:
	            si.nPos -= si.nPage;
	            break;
	              
	        case SB_PAGEDOWN:
	            si.nPos += si.nPage;
	            break;
	              
	        case SB_THUMBTRACK:
	            si.nPos = si.nTrackPos;
	            break;
	        }

	        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	        GetScrollInfo(hWnd, SB_VERT, &si);

	        if (yPos != si.nPos) {

	        	ScrollWindow(hWnd, 0, yPos - si.nPos, NULL, NULL);
	        	UpdateWindow(hWnd);
	        }
    	}
    	return 0;

		case WM_GETMINMAXINFO: {

			PMINMAXINFO pmmi = NULL;
			PAPPPERSISTDATA pAppPersistData = NULL;

			pAppPersistData = (PAPPPERSISTDATA)GetWindowLongPtrW(hWnd,
				GWLP_USERDATA);
			if (pAppPersistData) {

				pmmi = (PMINMAXINFO)lParam;

				pmmi->ptMaxSize.x = pAppPersistData->szWnd.cx;
					pmmi->ptMaxSize.y = pAppPersistData->szWnd.cy;
				pmmi->ptMinTrackSize.x = pAppPersistData->szWnd.cx;
					pmmi->ptMinTrackSize.y = pAppPersistData->szWnd.cy;
				pmmi->ptMaxTrackSize.x = pAppPersistData->szWnd.cx;
					pmmi->ptMaxTrackSize.y = pAppPersistData->szWnd.cy;
			}
		}
		return 0;

		case WM_COMMAND: {

			PAPPPERSISTDATA pAppPersistData = NULL;

			switch (LOWORD(wParam)) {

				case IDC_ADDNEW: {

					switch (HIWORD(wParam)) {

						case BN_CLICKED: {

							INT_PTR iRet = 0;
							std::list<CHECKLIST>::iterator curAddedIt;
							RECT rcClient = { 0 };
								SCROLLINFO si = { 0 };

							pAppPersistData = (PAPPPERSISTDATA)
								GetWindowLongPtrW(hWnd, GWLP_USERDATA);

							if (pAppPersistData) {

								iRet = DialogBoxParamW((HINSTANCE)
									GetWindowLongPtrW(hWnd, GWLP_HINSTANCE),
									MAKEINTRESOURCEW(IDD_ADDTASK),
									hWnd,
									AddCheckDialogProc,
									(LPARAM)&pAppPersistData->checkList);

								if (IDOK == iRet) {

									curAddedIt = pAppPersistData->
										checkList.begin();

									std::advance(curAddedIt,
										pAppPersistData->checkList
										.size() - 1);

									CreateWindowExW(0, (LPCWSTR)(DWORD64)
										pAppPersistData->pszClassName,
										NULL,
										WS_VISIBLE | WS_CHILDWINDOW,
										0, 65 * pAppPersistData->checkList
										.size(),
										300, 60,
										hWnd,
										(HMENU)IDC_CHECKSTART +
										(pAppPersistData->checkList
											.size() -1),
										(HINSTANCE)GetWindowLongPtrW(hWnd,
											GWLP_HINSTANCE),
										(LPVOID)&curAddedIt);

									GetClientRect(hWnd, &rcClient);
									si.fMask = SIF_RANGE;
									si.nMax = (65 * pAppPersistData
										->checkList.size()) - rcClient
										.bottom;

									si.nMax = (si.nMax < 0) ? 0 : si.nMax;
									SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
								}
							}
						}
						break;
					}
				}
				break;

				case IDC_CHANGEVIEW: {

					switch (HIWORD(wParam)) {

						case BN_CLICKED: {

							HWND hWndButton = NULL;
							LONG_PTR lpState = 0L;
							
							MONITORINFO mi = { 0 };
							DWORD dwStyle = 0;

							mi.cbSize = sizeof(MONITORINFO);

							pAppPersistData = (PAPPPERSISTDATA)
								GetWindowLongPtrW(hWnd, GWLP_USERDATA);

							if (pAppPersistData) {

								hWndButton = GetDlgItem(hWnd,
									IDC_CHANGEVIEW);
								
								lpState = GetWindowLongPtrW(hWndButton,
									GWLP_USERDATA);
								lpState = ++lpState%3;

								SetWindowLongPtrW(hWndButton,
									GWLP_USERDATA,
									lpState);

								switch ((unsigned long long)lpState) {

									case IDC_STATE_WINDOW: 
										SendMessageW(hWndButton,
											BM_SETIMAGE,
											(WPARAM)IMAGE_BITMAP,
											(LPARAM)pAppPersistData
											->hBitmapWindow);

										SetWindowLongPtrW(hWnd, GWL_STYLE,
											dwStyle |
											WS_OVERLAPPEDWINDOW);
										SetWindowPlacement(hWnd,
											&pAppPersistData->
											wpPrev);
										SetWindowPos(hWnd, NULL, 0, 0,
											0, 0,
											SWP_NOZORDER | SWP_NOMOVE
											| SWP_NOSIZE | SWP_NOOWNERZORDER
											| SWP_FRAMECHANGED);

										InvalidateRect(hWnd, NULL, FALSE);
										ShowScrollBar(hWnd, SB_VERT, TRUE);
										break;

									case IDC_STATE_RIGHT: {
											
										SendMessageW(hWndButton,
											BM_SETIMAGE,
											(WPARAM)IMAGE_BITMAP,
											(LPARAM)pAppPersistData
											->hBitmapAlignRight);
										dwStyle = (DWORD)GetWindowLongPtrW(
											hWnd,
											GWL_STYLE);

										GetWindowPlacement(hWnd, &
											pAppPersistData->wpPrev);
										GetMonitorInfoW(MonitorFromWindow(
											hWnd, MONITOR_DEFAULTTOPRIMARY),
											&mi);

										SetWindowLongPtrW(hWnd, GWL_STYLE,
											dwStyle & ~WS_OVERLAPPEDWINDOW);

										SetWindowPos(hWnd, HWND_TOPMOST, mi
											.rcMonitor.right -
											pAppPersistData->szWnd.cx, 0,
												pAppPersistData->szWnd.cx,
											mi.rcMonitor.bottom,
											SWP_NOOWNERZORDER |
											SWP_FRAMECHANGED);

									}
									break;

									case IDC_STATE_LEFT: {
										
										SendMessageW(hWndButton,
											BM_SETIMAGE,
											(WPARAM)IMAGE_BITMAP,
											(LPARAM)pAppPersistData
											->hBitmapAlignLeft);

										GetMonitorInfoW(MonitorFromWindow(
											hWnd, MONITOR_DEFAULTTOPRIMARY),
											&mi);

										SetWindowPos(hWnd, NULL,
											0, 0,
											0, 0,
											SWP_NOZORDER | SWP_NOSIZE);
									}
									break;
								}
							}
						}
					}
				}
				break;
			}
		}
		return 0;

		case WM_DONE: {

			PAPPPERSISTDATA pAppPersistData = NULL;
			HWND hWndChild = NULL;
			int nIndex = 0;
				RECT rcClient = { 0 };
			SCROLLINFO si = { 0 };

			pAppPersistData = (PAPPPERSISTDATA)GetWindowLongPtrW(hWnd,
				GWLP_USERDATA);

			if (pAppPersistData) {

				nIndex = (lParam - IDC_CHECKSTART) / 4;
				auto it = pAppPersistData->checkList.begin();
				std::advance(it, nIndex);
				pAppPersistData->checkList.erase(it);

				for (int i = nIndex;
					i < pAppPersistData->checkList.size();
					i++) {

					hWndChild = GetDlgItem(hWnd, IDC_CHECKSTART +
						(i + 1) * 4);
					
					SetWindowLongPtrW(hWndChild, GWLP_ID, (LONG_PTR)
						IDC_CHECKSTART + i * 4);

					SetWindowPos(hWndChild, NULL, 0, 0, 0, 0, SWP_NOMOVE
						| SWP_NOSIZE | SWP_NOZORDER
						| SWP_FRAMECHANGED);

					SetWindowPos(hWndChild, NULL, 0, (i + 1) * 65, 0, 0,
						SWP_NOZORDER | SWP_NOSIZE);
				}

				GetClientRect(hWnd, &rcClient);
				si.fMask = SIF_RANGE;
				si.nMax = (65 * pAppPersistData->checkList.size()) -
					rcClient.bottom;

				si.nMax = (si.nMax < 0) ? 0 : si.nMax;
				SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
			}
		}
		return 1;

		case WM_DESTROY: {

			PAPPPERSISTDATA pAppPersistData = NULL;

			pAppPersistData = (PAPPPERSISTDATA)GetWindowLongPtrW(hWnd,
				GWLP_USERDATA);

			DeleteObject(pAppPersistData->hBitmapAlignLeft);
			DeleteObject(pAppPersistData->hBitmapAlignRight);
			DeleteObject(pAppPersistData->hBitmapWindow);

			UnLoadFile(pAppPersistData->checkList);
			delete pAppPersistData;

			PostQuitMessage(0);
		}
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI CheckChildWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE: {

			LPCREATESTRUCTW pCSw = NULL;
			std::list<CHECKLIST>::iterator *pIt = NULL;
			HWND hWndStatic = NULL,
				hWndButton = NULL;
			SIZE szIdealButton = { 0 };

			HWND hWndTip = NULL;
			TTTOOLINFOW tttiW = { 0 };

			SYSTEMTIME sysTimeDue = { 0 };
				WCHAR *pwDateFormatBuf = NULL;
			INT nCount = 0;

			pCSw = (LPCREATESTRUCTW)lParam;
			pIt = (std::list<CHECKLIST>::iterator *)pCSw->lpCreateParams;

			hWndStatic = CreateWindowExW(0, WC_STATICW, (*(*pIt)).pszCheck,
				WS_VISIBLE | WS_CHILDWINDOW | SS_CENTER | SS_ENDELLIPSIS,
				12, 4,
				200, 24,
				hWnd,
				NULL,
				pCSw->hInstance,
				NULL);

			hWndButton = CreateWindowExW(0, WC_BUTTONW, L"Done",
				WS_VISIBLE | WS_CHILDWINDOW,
				0, 0,
				0, 0,
				hWnd,
				(HMENU)IDC_DONE,
				pCSw->hInstance,
				NULL);

			SendMessageW(hWndButton, BCM_GETIDEALSIZE, (WPARAM)0, (LPARAM)
				&szIdealButton);

			szIdealButton.cx += 20;
			SetWindowPos(hWndButton, NULL, pCSw->cx - 12 - szIdealButton
				.cx, 0,
				szIdealButton.cx, szIdealButton.cy,
				SWP_NOZORDER);

			hWndTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL,
				WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
				CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd,
				NULL,
				pCSw->hInstance,
				NULL);

			tttiW.cbSize = (UINT)sizeof(TTTOOLINFOW);
			tttiW.hwnd = pCSw->hwndParent;
			tttiW.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			tttiW.uId= (UINT_PTR)hWnd;
			tttiW.lpszText = (*(*pIt)).pszCheck;

			SendMessageW(hWndTip, TTM_ADDTOOLW, 0, (LPARAM)&tttiW);

			sysTimeDue = (*(*pIt)).sysTimeDue;
			nCount = GetDateFormatW(LOCALE_USER_DEFAULT, 0, &sysTimeDue,
				L"dd/MM/yyyy",
				NULL,
				0);

			if (nCount > 0) {

				pwDateFormatBuf = new WCHAR[nCount + 1];
				if (pwDateFormatBuf) {

					GetDateFormatW(LOCALE_USER_DEFAULT, 0, &sysTimeDue,
						L"dd/MM/yyyy",
						pwDateFormatBuf,
						nCount + 1);

					CreateWindowExW(0, WC_STATICW, pwDateFormatBuf,
						WS_VISIBLE |  WS_CHILDWINDOW | SS_CENTER,
						12, 30,
						200, 24,
						hWnd,
						NULL,
						pCSw->hInstance,
						NULL);

					delete pwDateFormatBuf;
				}
			}
		}
		return 0;

		case WM_COMMAND: {

			switch (LOWORD(wParam)) {

				case IDC_DONE: {

					SendMessageW(GetParent(hWnd), WM_DONE,
						(WPARAM)hWnd,
						(LPARAM)GetWindowLongPtrW(hWnd, GWLP_ID));
					DestroyWindow(hWnd);
				}
				break;
			}
		}
		return 0;


		case WM_CTLCOLORSTATIC: return (LRESULT)GetStockObject(WHITE_BRUSH);
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT_PTR AddCheckDialogProc(HWND hDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	switch (uMsg) {

		case WM_INITDIALOG: {

			SYSTEMTIME sysRange[2] = { 0 };

			GetSystemTime(sysRange);
			sysRange[0].wHour = 0;
			sysRange[0].wMinute = 0;
			sysRange[0].wSecond = 0;
			sysRange[0].wMilliseconds = 0;

			SendMessageW(GetDlgItem(hDlg, IDC_DATETIMEPICKER),
				DTM_SETRANGE,
				(WPARAM)GDTR_MIN,
				(LPARAM)sysRange);

			SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
				SendMessageW(hDlg, DM_SETDEFID, (WPARAM)IDOK, (LPARAM)0);
			SendMessageW(GetDlgItem(hDlg, IDC_EDITTASK), EM_SETLIMITTEXT,
				(WPARAM)100,
				(LPARAM)NULL);
		}
		return TRUE;

		case WM_COMMAND: {

			switch (LOWORD(wParam)) {

				case IDOK: {

					std::list<CHECKLIST> *pCheckList = NULL;
					HWND hWndEditTask = NULL;
					WCHAR pszCheckName[MAX_PSZCHECK*2+1];
					SYSTEMTIME sysTimeDue = { 0 };

					hWndEditTask = GetDlgItem(hDlg, IDC_EDITTASK);
					pCheckList = (std::list<CHECKLIST> *)
							GetWindowLongPtrW(hDlg,
								GWLP_USERDATA);

					if (GetWindowTextLengthW(hWndEditTask) > 0 ) {

						GetWindowTextW(hWndEditTask, pszCheckName, 101);

						for (auto it = pCheckList->begin();
							pCheckList->end() != it;
							it++) {
							
							if (!wcscmp((*it).pszCheck, pszCheckName)) {
								
								MessageBoxW(hDlg, L"Task name already"
									L" exists!",
									pszMainWindowName,
									MB_OK);
								return TRUE;
							}
						}

						SendMessageW(GetDlgItem(hDlg, IDC_DATETIMEPICKER),
							DTM_GETSYSTEMTIME,
							(WPARAM)NULL,
							(LPARAM)&sysTimeDue);
						pCheckList->emplace_back(sysTimeDue, (BYTE *)
							pszCheckName);

					} else {

						MessageBoxW(hDlg, L"Please provide a name for the"
							L" task!",
							pszMainWindowName,
							MB_OK);
						return TRUE;
					}
				}

				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					break;
			}
		}
		return TRUE;
	}

	return FALSE;
}

void LoadFile(std::list<CHECKLIST> &refCheckList) {

	HANDLE hCheckListFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER lgszFile = { 0 };
	DWORD dwNumBytesRead = 0;
	BYTE *pbMemReadBuf = NULL,
		*pbMemIterate = NULL;

	hCheckListFile = CreateFileW(pszCheckDatFilename, GENERIC_READ, 0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE != hCheckListFile &&
		GetFileSizeEx(hCheckListFile, &lgszFile)) {

		pbMemReadBuf = new BYTE[lgszFile.QuadPart];
		ReadFile(hCheckListFile, (LPVOID)pbMemReadBuf, (DWORD)lgszFile
			.QuadPart,
			&dwNumBytesRead,
			NULL);

		if (pbMemReadBuf) {

			pbMemIterate = pbMemReadBuf;
			while ((pbMemIterate - pbMemReadBuf) < lgszFile.QuadPart) {

				refCheckList.emplace_back(*((SYSTEMTIME *)pbMemIterate),
					pbMemIterate + sizeof(SYSTEMTIME));

				pbMemIterate += sizeof(SYSTEMTIME);
				pbMemIterate += (wcslen((const wchar_t *)pbMemIterate)
					+ 1) * sizeof(WCHAR);
			}

			delete pbMemReadBuf;
		}

		CloseHandle(hCheckListFile);
	}
}

void UnLoadFile(std::list<CHECKLIST> &refCheckList) {

	HANDLE hCheckListFile = INVALID_HANDLE_VALUE;
	SYSTEMTIME sysTimeDue = { 0 };
	DWORD dwNumBytesWritten = 0;

	hCheckListFile = CreateFileW(pszCheckDatFilename, GENERIC_WRITE, 0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE != hCheckListFile) {

		for (auto it = refCheckList.begin();
			refCheckList.end() != it;
			it++) {

			sysTimeDue = (*it).sysTimeDue;
			WriteFile(hCheckListFile, &sysTimeDue, (DWORD)sizeof(SYSTEMTIME),
				&dwNumBytesWritten,
				NULL);

			WriteFile(hCheckListFile, (*it).pszCheck, (wcslen((*it).pszCheck)
				+ 1) * sizeof(WCHAR),
				&dwNumBytesWritten,
				NULL);
		}

		CloseHandle(hCheckListFile);
	}
}