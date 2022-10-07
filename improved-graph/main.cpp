#include "headers/stdafx.h"

typedef struct tagGraphPoint GraphSize;

//		Defined as value per 100 pixels
typedef struct tagGraphVpp {

	int multiIndicator;
	float value;
} GraphVpp;

typedef struct tagGraphWorldTransform {

	POINT graphPt;
	GraphVpp Vpp;
} GraphWorldTransform;

#define IDF_SINE 		0
#define IDF_COSINE 		1
#define IDF_TANGENT 	2
#define IDF_EXP 		3
#define IDF_EXPE 		4
#define IDF_LOG			5
#define IDF_LOGE		6
#define IDF_POWER		7

typedef struct tagGraphFunction {

	int functionType;
	double coefficient,
		power,
		base,
		value,
		baseExp;
	Pen *functionStrokePen;
} GraphFunction;

typedef struct tagMarginDrawInfo {

	Pen *axisStrokePen,
		*marginStrokePen;
	FontFamily *marginFontFamily;
	Font *marginFont;
	SolidBrush *fontSolidBrush;	
} MarginDrawInfo;

typedef struct tagAppplicationData {

	GraphWorldTransform graphTransform;
	POINT prevMousePt;
	SIZE screenSize;
	GraphFunction graphFunction;
	MarginDrawInfo marginDrawInfo;
} ApplicationData;

const LPCWSTR functionTypeStringArray[] = {

	L"Sin(x)",
		L"Cos(x)",
	L"Tan(x)",
	L"A^(x)",
	L"Exp^(x)",
		L"Logb(x)",
	L"Ln(x)",
	L"x^A"
};

const LPWSTR lpszWindowName = L"Graph";

LRESULT WINAPI GraphWindowProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR FunctionDialogProc(HWND, UINT, WPARAM, LPARAM);

void IncrementIndicator(GraphVpp&);
	void DecrementIndicator(GraphVpp&);

void StartPaint(PAINTSTRUCT *, GraphFunction&, GraphWorldTransform&,
	SIZE&, MarginDrawInfo&);
void PaintGraphMargins(Graphics&, RECT&, GraphWorldTransform&,
	SIZE&, MarginDrawInfo&);
void PaintFunction(Graphics&, GraphFunction&, std::vector<Point>);
void GenerateFunctionPoints(GraphFunction&, std::vector<Point>&,
	RECT&, GraphWorldTransform&, SIZE&);

#define WINDOW_WIDTH 	1278
#define WINDOW_HEIGHT	720

int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR pCmdLine,
	int nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);
		UNREFERENCED_PARAMETER(nCmdShow);

	HWND hWnd = NULL;
	MSG Msg = { 0 };
	ATOM lpszClassName = 0;
	WNDCLASSEXW wcex = { 0 };
	GdiplusStartupInput gdiStartupInput = { 0 };
	ULONG_PTR gdiplusToken = 0L;

#if defined (__DEBUG)
	InitDebug();
#endif
	BufferedPaintInit();
	GdiplusStartup(&gdiplusToken, &gdiStartupInput, NULL);

	wcex.cbSize 		= sizeof(WNDCLASSEXW);
	wcex.style 			= CS_VREDRAW;
	wcex.lpfnWndProc 	= GraphWindowProc;
	wcex.hInstance 		= hInstance;
	wcex.hIcon 			= LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wcex.hCursor 		= LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wcex.lpszMenuName 	= MAKEINTRESOURCEW(IDM_GRAPH);
	wcex.lpszClassName 	= lpszWindowName;
	wcex.hIconSm 		= LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);

	lpszClassName = RegisterClassExW(&wcex);
	if (lpszClassName) {

		hWnd = CreateWindowExW(0, (LPCWSTR)(DWORD64)lpszClassName,
			lpszWindowName,
			WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			NULL,
			NULL,
			hInstance,
			NULL);
		if (hWnd) {

			while (GetMessageW(&Msg, NULL, 0, 0)) {

				TranslateMessage(&Msg); DispatchMessageW(&Msg);
			}
		}
	} else {

		MessageBoxW(NULL, L"Failed to register Window! Please restart.",
			lpszWindowName,
			MB_ICONERROR | MB_OK);
		return 1;
	}

	GdiplusShutdown(gdiplusToken);
	BufferedPaintUnInit();
#if defined (__DEBUG)
	StopDebug();
#endif

	return 0;
}

LRESULT WINAPI GraphWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	ApplicationData* lpApplicationData = (ApplicationData*)
		GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	switch (uMsg) {

		case WM_CREATE: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"WM_CREATE"));
#endif
			RECT rcClient = { 0 };

			lpApplicationData = (ApplicationData*)HeapAlloc(GetProcessHeap(),
				0,
				sizeof(ApplicationData));
			
			lpApplicationData->graphTransform.graphPt.x = 0;
				lpApplicationData->graphTransform.graphPt.y = 0;

			lpApplicationData->graphTransform.Vpp.multiIndicator = 0;
			lpApplicationData->graphTransform.Vpp.value = 3.14F;
			lpApplicationData->graphFunction.functionType = IDF_SINE;
			lpApplicationData->graphFunction.coefficient = 3.14F;
			lpApplicationData->graphFunction.functionStrokePen = new Pen(
				Color(255, 0, 0),
				2);

			if (GetClientRect(hWnd, &rcClient)) {

				lpApplicationData->screenSize.cx = rcClient.right;
					lpApplicationData->screenSize.cy = rcClient.bottom;
			} else {

				return -1;
			}

			lpApplicationData->marginDrawInfo.axisStrokePen = new Pen(
				Color(0, 0, 0),
				2);
			lpApplicationData->marginDrawInfo.marginStrokePen = new Pen(
				Color(200, 200, 200),
				1);
			lpApplicationData->marginDrawInfo.marginFontFamily = new
				FontFamily(L"Arial");
			lpApplicationData->marginDrawInfo.marginFont = new Font(
				lpApplicationData->marginDrawInfo.marginFontFamily,
				14,
				FontStyleRegular,
				UnitPixel);
			lpApplicationData->marginDrawInfo.fontSolidBrush = new
				SolidBrush(Color(0, 0, 0));

			SetWindowLongPtrW(hWnd, GWLP_USERDATA,
				(LONG_PTR)lpApplicationData);
		}
		return 0;

		case WM_COMMAND: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"WM_COMMAND"));
#endif
			switch (LOWORD(wParam)) {

				case IDM_GRAPH_CHANGE: {

#if defined (__DEBUG)
					LogDebug(std::wstring(L"IDM_GRAPH_CHANGE"));
#endif
					INT_PTR iPtrDlgResult;
					iPtrDlgResult = DialogBoxParamW((HINSTANCE)GetWindowLongPtrW(
						hWnd,
						GWLP_HINSTANCE),
						MAKEINTRESOURCEW(IDD_FUNCTION),
						hWnd,
						FunctionDialogProc,
						(LPARAM)&(lpApplicationData->graphFunction));

					if (IDOK == iPtrDlgResult) {

						InvalidateRect(hWnd, NULL, FALSE);
					}
#if defined (__DEBUG)
					else if (-1 == iPtrDlgResult) {

						LogDebug(std::wstring(L"CreateDialogParamW Failed!"));
					}
#endif
				}
				break;

				case IDM_GRAPH_COLOR: {

#if defined (__DEBUG)
					LogDebug(std::wstring(L"IDM_GRAPH_COLOR"));
#endif

					static COLORREF custColors[16];
					CHOOSECOLORW chooseColor = { 0 };
					Color penColor;
					lpApplicationData->graphFunction.functionStrokePen->
						GetColor(&penColor);

					chooseColor.lStructSize = sizeof(CHOOSECOLORW);
					chooseColor.hwndOwner = hWnd;
					chooseColor.rgbResult = penColor.ToCOLORREF();
					chooseColor.lpCustColors = custColors;
					chooseColor.Flags = CC_RGBINIT | CC_PREVENTFULLOPEN |
						CC_SOLIDCOLOR;
					if (ChooseColorW(&chooseColor)) {

						penColor.SetFromCOLORREF(chooseColor.rgbResult);
						lpApplicationData->graphFunction.functionStrokePen->
							SetColor(penColor);
						InvalidateRect(hWnd, NULL, FALSE);
					}
				}
				break;
			}
		}
		return 0;

		case WM_SIZE: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"WM_SIZE"));
#endif
			lpApplicationData->screenSize.cx = LOWORD(lParam);
				lpApplicationData->screenSize.cy = HIWORD(lParam);
		}
		return 0;


		case WM_LBUTTONDOWN: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"WM_LBUTTONDOWN"));
#endif
			lpApplicationData->prevMousePt.x = LOWORD(lParam);
				lpApplicationData->prevMousePt.y = HIWORD(lParam);
		}
		return 0;

		case WM_MOUSEMOVE: {

			if (wParam & MK_LBUTTON) {

				lpApplicationData->graphTransform.graphPt.x -=
					LOWORD(lParam) - lpApplicationData->prevMousePt.x;
				
				lpApplicationData->graphTransform.graphPt.y +=
					HIWORD(lParam) - lpApplicationData->prevMousePt.y;

				lpApplicationData->prevMousePt.x = LOWORD(lParam);
					lpApplicationData->prevMousePt.y = HIWORD(lParam);

#if defined (__DEBUG)
				LogDebug(std::wstring(L"curX: ") + std::to_wstring(
					lpApplicationData->graphTransform.graphPt.x));

					LogDebug(std::wstring(L"curY: ") + std::to_wstring(
						lpApplicationData->graphTransform.graphPt.y));
#endif
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		return 0;

		case WM_MOUSEWHEEL: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"WM_MOUSEWHEEL"));
#endif
			if ((short)HIWORD(wParam) > 0) {

				DecrementIndicator(lpApplicationData->graphTransform.Vpp);
				InvalidateRect(hWnd, NULL, FALSE);
			} else if ((short)HIWORD(wParam) < 0) {

				
				IncrementIndicator(lpApplicationData->graphTransform.Vpp);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		return 0;

		case WM_PAINT: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"WM_PAINT"));
#endif
			if (lpApplicationData) {

				PAINTSTRUCT ps = { 0 };

				BeginPaint(hWnd, &ps);
					StartPaint(&ps, lpApplicationData->graphFunction,
						lpApplicationData->graphTransform,
						lpApplicationData->screenSize,
						lpApplicationData->marginDrawInfo);
				EndPaint (hWnd, &ps);
			}
		}
		return 0;

		case WM_DESTROY: {

#if defined (__DEBUG)			
			LogDebug(std::wstring(L"WM_DESTROY"));
#endif
			if (lpApplicationData) {

				delete lpApplicationData->marginDrawInfo.axisStrokePen;
				delete lpApplicationData->marginDrawInfo.marginStrokePen;
				delete lpApplicationData->marginDrawInfo.marginFontFamily;
				delete lpApplicationData->marginDrawInfo.marginFont;
				delete lpApplicationData->marginDrawInfo.fontSolidBrush;

				HeapFree(GetProcessHeap(), 0, (LPVOID)lpApplicationData);
			}
			PostQuitMessage(0);
		}
		return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void IncrementIndicator(GraphVpp& refVpp) {

	if (0 == refVpp.multiIndicator ||
		2 == refVpp.multiIndicator) {

		refVpp.value *= 2.0F;
	} else {

		refVpp.value *= 2.5F;
	}
	refVpp.multiIndicator = (2 < ++refVpp.multiIndicator) ?
		0 : refVpp.multiIndicator;
}

void DecrementIndicator(GraphVpp& refVpp) {

	if (0 == refVpp.multiIndicator ||
		1 == refVpp.multiIndicator) {

		refVpp.value *= 0.5F;
	} else {

		refVpp.value *= 0.4F;
	}

	refVpp.multiIndicator = (0 > --refVpp.multiIndicator) ?
		2 : refVpp.multiIndicator;
}

void StartPaint(PAINTSTRUCT* ps,
	GraphFunction& refGraphFunction,
	GraphWorldTransform& refGraphTransform,
	SIZE& refScreenSize,
	MarginDrawInfo& refMarginDrawInfo) {


	BP_PAINTPARAMS bpPaintParams = { 0 };
	HDC hDCDraw = NULL;
	HPAINTBUFFER hPaintBuffer = NULL;

	bpPaintParams.cbSize 			= sizeof(BP_PAINTPARAMS);
	bpPaintParams.dwFlags 			= BPPF_ERASE;
	bpPaintParams.prcExclude 		= NULL;
	bpPaintParams.pBlendFunction 	= NULL;

	
	hPaintBuffer = BeginBufferedPaint(ps->hdc, &(ps->rcPaint),
		BPBF_COMPATIBLEBITMAP,
		&bpPaintParams,
		&hDCDraw);

	if (hPaintBuffer) {

		Graphics graphics(hDCDraw);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.Clear(Color(255, 255, 255));

		PaintGraphMargins(graphics, ps->rcPaint, refGraphTransform,
			refScreenSize, refMarginDrawInfo);

		std::vector<Point> graphFunctionPts;
		GenerateFunctionPoints(refGraphFunction , graphFunctionPts,
			ps->rcPaint,
			refGraphTransform,
			refScreenSize);
		PaintFunction(graphics, refGraphFunction, graphFunctionPts);
	}
	EndBufferedPaint(hPaintBuffer, TRUE);
}

#define MARGIN_OFFSET 4
#define MARGIN_TEXT_OFFSET 2

void PaintGraphMargins(Graphics& refGraphics,
	RECT& refUpdateRect,
	GraphWorldTransform& refGraphTransform,
	SIZE& refScreenSize,
	MarginDrawInfo& refMarginDrawInfo) {

	int yXAxis = refScreenSize.cy / 2 + refGraphTransform.graphPt.y,
			xYAxis = refScreenSize.cx / 2 - refGraphTransform.graphPt.x,

		yXMarginStart = yXAxis - MARGIN_OFFSET,
		yXMarginEnd = yXAxis + MARGIN_OFFSET,

		xYMarginStart = xYAxis - MARGIN_OFFSET,
		xYMarginEnd = xYAxis + MARGIN_OFFSET,
		y = 0,
			x = 0,
		i = 0;

	std::wstring nWstring;
	const PointF zeroPt(0.0F, 0.0F);
	PointF drawStringPt;
	RectF boundingBox;

	
	refGraphics.DrawLine(refMarginDrawInfo.axisStrokePen, refUpdateRect.left,
		yXAxis,
		refUpdateRect.right,
		yXAxis);
	refGraphics.DrawLine(refMarginDrawInfo.axisStrokePen, xYAxis, refUpdateRect
		.top,
		xYAxis,
		refUpdateRect.bottom);

	for (y = yXAxis - 100, i = 1;
		y > 0;
		y -= 100, i++) {

		if (y <= refScreenSize.cy) {

			refGraphics.DrawLine(refMarginDrawInfo.marginStrokePen,
				refUpdateRect.left,
				y,
				refUpdateRect.right,
				y);
			refGraphics.DrawLine(refMarginDrawInfo.axisStrokePen,
				xYMarginStart,
				y,
				xYMarginEnd,
				y);

			nWstring = std::to_wstring((float)i * refGraphTransform.Vpp
				.value);
			refGraphics.MeasureString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				zeroPt,
				&boundingBox);
			drawStringPt = PointF((float)(xYMarginEnd + MARGIN_TEXT_OFFSET),
				y - boundingBox.GetBottom() / 2.0F);

			refGraphics.DrawString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				drawStringPt,
				refMarginDrawInfo.fontSolidBrush);
		}

	}

	for (y = yXAxis + 100, i = -1;
		y < refScreenSize.cy;
		y += 100, i--) {

		if (y >= 0) {

			refGraphics.DrawLine(refMarginDrawInfo.marginStrokePen,
				refUpdateRect.left,
				y,
				refUpdateRect.right,
				y);
			refGraphics.DrawLine(refMarginDrawInfo.axisStrokePen,
				xYMarginStart,
				y,
				xYMarginEnd,
				y);

			nWstring = std::to_wstring((float)i * refGraphTransform.Vpp
				.value);
			refGraphics.MeasureString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				zeroPt,
				&boundingBox);
			drawStringPt = PointF((float)(xYMarginEnd + MARGIN_TEXT_OFFSET),
				y - boundingBox.GetBottom() / 2.0F);

			refGraphics.DrawString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				drawStringPt,
				refMarginDrawInfo.fontSolidBrush);
		}
	}

	for (x = xYAxis - 100, i = -1;
		x > 0;
		x -= 100, i--) {

		if (x <= refScreenSize.cx) {

			refGraphics.DrawLine(refMarginDrawInfo.marginStrokePen, x,
				refUpdateRect.top,
				x,
				refUpdateRect.bottom);
			refGraphics.DrawLine(refMarginDrawInfo.axisStrokePen, x,
				yXMarginStart,
				x,
				yXMarginEnd);

			nWstring = std::to_wstring((float)i * refGraphTransform.Vpp
				.value);
			refGraphics.MeasureString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				zeroPt,
				&boundingBox);
			drawStringPt = PointF(x - boundingBox.GetRight() / 2.0F,
				(float)(yXMarginEnd + MARGIN_TEXT_OFFSET));
			
			refGraphics.DrawString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				drawStringPt,
				refMarginDrawInfo.fontSolidBrush);
		}
	}

	for (x = xYAxis + 100, i = 1;
		x < refScreenSize.cx;
		x += 100, i++) {

		if (x >= 0) {
			refGraphics.DrawLine(refMarginDrawInfo.marginStrokePen, x,
				refUpdateRect.top,
				x,
				refUpdateRect.bottom);
			refGraphics.DrawLine(refMarginDrawInfo.axisStrokePen, x,
				yXMarginStart,
				x,
				yXMarginEnd);

			nWstring = std::to_wstring((float)i * refGraphTransform.Vpp
				.value);
			refGraphics.MeasureString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				zeroPt,
				&boundingBox);
			drawStringPt = PointF(x - boundingBox.GetRight() / 2.0F,
				(float)(yXMarginEnd + MARGIN_TEXT_OFFSET));

			refGraphics.DrawString((const WCHAR *)nWstring.c_str(),
				(INT)nWstring.length(),
				refMarginDrawInfo.marginFont,
				drawStringPt,
				refMarginDrawInfo.fontSolidBrush);
		}
	}
}

#define PIXEL_OFFSET 2

void GenerateFunctionPoints(GraphFunction& refGraphFunction,
	std::vector<Point>& refVecGraphPts,
	RECT& refUpdateRect,
	GraphWorldTransform& refGraphTransform,
	SIZE& refScreenSize) {

	UNREFERENCED_PARAMETER(refUpdateRect);
	refVecGraphPts.reserve((size_t)(refScreenSize.cx / PIXEL_OFFSET));

	int xPixel = 0,
			yPixel = 0;

	double pixelStep = refGraphTransform.Vpp.value / (100 / PIXEL_OFFSET),
		x = ((refGraphTransform.graphPt.x - refScreenSize.cx
			/ 2) * (refGraphTransform.Vpp.value / 100.0F)),
			y = 0;
	
	Point pt;

#if defined (__DEBUG)
	std::wstring ptWString;
#endif

	switch (refGraphFunction.functionType) {

		case IDF_SINE: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_SINE"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * sin(x);

				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x += pixelStep;
			}
		}
		break;

		case IDF_COSINE: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_COSINE"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * cos(x);

				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x += pixelStep;
			}
		}	
		break;
		
		case IDF_TANGENT: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_TANGENT"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * tan(x);
				
				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x+= pixelStep;
			}
		}
		break;
		
		case IDF_EXP: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_EXP"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * pow(refGraphFunction.
					baseExp, x);

				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x += pixelStep;
			}
		}
		break;
		
		case IDF_EXPE: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_EXPE"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * exp(x);
				
				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x += pixelStep;
			}
		}
		break;
		
		case IDF_LOG: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_LOG"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * (log(refGraphFunction.
					value) / log(x));
				
				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;
				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x += pixelStep;
			}
		}
		break;
		
		case IDF_LOGE: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_LOGE"));
#endif
			for (;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * log(x);

				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}

				x += pixelStep;
			}
		}
		break;
		
		case IDF_POWER: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"IDF_POWER"));
#endif
			for(;
				xPixel < refScreenSize.cx;
				xPixel += PIXEL_OFFSET) {

				y = -refGraphFunction.coefficient * pow(x, refGraphFunction.
					power);

				y /= refGraphTransform.Vpp.value / 100.0F;
				yPixel = (int)(y + refScreenSize.cy / 2) + refGraphTransform.
					graphPt.y;

				if (yPixel >= 0 &&
					yPixel <= refScreenSize.cy)
				{
					pt = Point(xPixel, yPixel);
					refVecGraphPts.push_back(pt);
				}
#if defined (__DEBUG)
				ptWString += std::to_wstring(yPixel) + std::wstring(L", ");
#endif

				x += pixelStep;	
			}
		}
		break;
	}

#if defined (__DEBUG)
	LogDebug(ptWString);
#endif
}

void PaintFunction(Graphics& graphics,
	GraphFunction& refGraphFunction,
	std::vector<Point> vecGraphPts) {

	graphics.DrawCurve(refGraphFunction.functionStrokePen, vecGraphPts.data(),
		(INT)vecGraphPts.size());
}

INT_PTR FunctionDialogProc(HWND hDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	GraphFunction *lpGraphFunction = (GraphFunction *)GetWindowLongPtrW(hDlg,
		GWLP_USERDATA);

	switch (uMsg) {

		case WM_INITDIALOG: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"FunctionDialogProc: WM_INITDIALOG"));
#endif

			HWND hComboBox = GetDlgItem(hDlg, IDC_COMBOBOX);
			lpGraphFunction = (GraphFunction *)
				lParam;

			for (int i=0;
				i<_countof(functionTypeStringArray);
				i++){

				SendMessageW(hComboBox, CB_ADDSTRING, (LPARAM)0, (WPARAM)
					functionTypeStringArray[i]);
			}

			SendMessageW(hComboBox, CB_SETCURSEL, (WPARAM)lpGraphFunction
				->functionType,
				(LPARAM)0);
			SetWindowTextW(GetDlgItem(hDlg, IDC_EDITTEXT_COFF),
				(LPCWSTR)std::to_wstring(lpGraphFunction->coefficient)
				.c_str());

			if (IDF_EXPE != lpGraphFunction->functionType &&
					IDF_LOG != lpGraphFunction->functionType &&
				IDF_POWER != lpGraphFunction->functionType) {

				ShowWindow(GetDlgItem(hDlg, IDC_CTEXT_EXTRA), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_EDITTEXT_EXTRA), SW_HIDE);
			}

			SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)lpGraphFunction);
		}
		return TRUE;

		case WM_COMMAND: {

#if defined (__DEBUG)
			LogDebug(std::wstring(L"FunctionDialogProc: WM_COMMAND"));
#endif
			switch (LOWORD(wParam)) {

				case IDC_COMBOBOX: {

#if defined (__DEBUG)
					LogDebug(std::wstring(L"IDC_COMBOBOX"));
#endif
					switch (HIWORD(wParam)) {

						case CBN_SELCHANGE: {

#if defined (__DEBUG)
							LogDebug(std::wstring(L"CBN_SELCHANGE"));
#endif
							int itemIndex = 0;
							HWND hCTextExtra = NULL,
								hEditTextExtra = NULL;

							itemIndex = (int)SendMessageW(GetDlgItem(hDlg,
								IDC_COMBOBOX),
								CB_GETCURSEL,
								(WPARAM)0,
									(LPARAM)0);
							hCTextExtra = (HWND)GetDlgItem(hDlg,
								IDC_CTEXT_EXTRA);
							hEditTextExtra = (HWND)GetDlgItem(hDlg,
								IDC_EDITTEXT_EXTRA);
#if defined(__DEBUG)
							LogDebug(std::to_wstring(itemIndex));
#endif

							switch (itemIndex) {

								case IDF_EXP: {
#if defined (__DEBUG)

									LogDebug(std::wstring(L"IDF_EXP"));
#endif
									SetWindowTextW(hCTextExtra,
										L"Base Exponent: ");
									ShowWindow(hCTextExtra, SW_SHOW);
										ShowWindow(hEditTextExtra, SW_SHOW);
								}
								break;

								case IDF_LOG: {

#if defined (__DEBUG)
									LogDebug(std::wstring(L"IDF_LOG"));
#endif

									SetWindowTextW(hCTextExtra,
										L"Base Logarithm: ");
									ShowWindow(hCTextExtra, SW_SHOW);
										ShowWindow(hEditTextExtra, SW_SHOW);
								}
								break;

								case IDF_POWER: {

#if defined (__DEBUG)
									LogDebug(std::wstring(L"IDF_POWER"));
#endif								
									SetWindowTextW(hCTextExtra,
										L"Exponent: ");
									ShowWindow(hCTextExtra, SW_SHOW);
										ShowWindow(hEditTextExtra, SW_SHOW);
								}
								break;

								default: {

#if defined (__DEBUG)
									LogDebug(std::wstring(L"default"));
#endif
									ShowWindow(hCTextExtra, SW_HIDE);
										ShowWindow(hEditTextExtra, SW_HIDE);
								}
							}
						}
					}
				}
				break;

				case IDOK: {

#if defined (__DEBUG)
					LogDebug(std::wstring(L"IDOK"));
#endif

					LPWSTR lpszWindowString = NULL,
						lppEndPtr;
					HWND hEditTextCoefficient = NULL;
					int nWCharCount = 0;
					double newCoefficient;

					hEditTextCoefficient = GetDlgItem(hDlg, IDC_EDITTEXT_COFF);
					nWCharCount = GetWindowTextLengthW(hEditTextCoefficient);

#if defined (__DEBUG)
					LogDebug(std::to_wstring(nWCharCount));
#endif
					if (0 != nWCharCount) {

						lpszWindowString = (LPWSTR)HeapAlloc(GetProcessHeap(),
							0,
							(SIZE_T)(nWCharCount + 1) * 2Ui64);
						if (lpszWindowString &&
							nWCharCount == GetWindowTextW(hEditTextCoefficient,
								lpszWindowString,
								nWCharCount + 1)) {

							newCoefficient = std::wcstod(lpszWindowString,
								&lppEndPtr);
							HeapFree(GetProcessHeap(), 0, lpszWindowString);
#if defined (__DEBUG)
							LogDebug(std::to_wstring(newCoefficient));
#endif

							if ((lpszWindowString + nWCharCount) ==
									lppEndPtr &&
								0.0 != newCoefficient) {

								lpGraphFunction->functionType = 
									(int)SendMessageW(
										GetDlgItem(hDlg, IDC_COMBOBOX),
										CB_GETCURSEL,
										(WPARAM)0,
											(LPARAM)0);
								lpGraphFunction->coefficient = newCoefficient;
								EndDialog(hDlg, LOWORD(wParam));
							} else {

								MessageBoxW(hDlg, L"Invalid Fields!",
									L"Change Function",
									MB_OK | MB_ICONERROR);
							}
						}
					}
				}
				break;

				case IDCANCEL: {

#if defined (__DEBUG)
					LogDebug(std::wstring(L"IDCANCEL"));
#endif
					EndDialog(hDlg, LOWORD(wParam));
				}
				break;
			}
		}
		return TRUE;
	}

	return FALSE;
}