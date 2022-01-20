#define UNICODE
#define _UNICODE

#include <windows.h>
#include <math.h>

typedef struct _fPoint3D fPoint3D, *LPfPoint3D;
typedef struct _Quad3D Quad3D, *LPQuad3D;
typedef struct _Cube3D Cube3D, *LPCube3D;
typedef struct _DrawGraphicsInfo DrawGraphicsInfo, *LPDrawGraphicsInfo;

struct _fPoint3D {
    double x, y, z;
};

struct _Quad3D {
    fPoint3D pt1, pt2, pt3, pt4;
};

struct _Cube3D {
    Quad3D FaceFront, FaceLeft, FaceRight, FaceBack;
};

struct _DrawGraphicsInfo {
    HDC hDCFront, hDCBack;
    HBITMAP hBmpOldFront, hBmpOldBack;
};

LRESULT WindowProc(HWND, UINT, WPARAM, LPARAM);

DrawGraphicsInfo dwGi;

SwitchBuffers(HDC*, HDC*);
void WorldPtToPagePt(LPXFORM, LPfPoint3D, int);

fPoint3D CrossProduct(LPfPoint3D, LPfPoint3D);
double DotProduct(LPfPoint3D, LPfPoint3D);
double vabs(LPfPoint3D);

double angle = 0.0;
short cxScreen, cyScreen;
XFORM CenterTranslate;
fPoint3D eyeAxis = { 0.0, 0.0, 1.0 };

int WINAPI
wWinMain(HINSTANCE hInstance,
HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    WNDCLASS wndcls;
    HWND hWnd;
    MSG msg;
    fPoint3D ptArray[48];
    XFORM xzRotate;
    LPQuad3D FaceArray;
    double FaceAngle;
    fPoint3D v1, v2;
    HDC hDC;
    HBRUSH oldBrush;
    
    ZeroMemory(&CenterTranslate, sizeof(XFORM));
    CenterTranslate.eM11 = CenterTranslate.eM22 = 1.0;

    Cube3D Cube = { { {-250.0, 250.0, 250.0}, {250.0, 250.0, 250.0},
            {250.0, -250.0, 250.0}, {-250.0, -250.0, 250.0} },
            { {-250.0, 250.0, -250.0}, {-250.0, 250.0, 250.0},
            {-250.0, -250.0, 250.0}, {-250.0, -250.0, -250.0} },
            { {250.0, 250.0, 250.0}, {250.0, 250.0, -250.0},
            {250.0, -250.0, -250.0}, {250.0, -250.0, -250.0} },
            { {250.0, 250.0, -250.0}, {-250.0, 250.0, -250.0},
            {-250.0, -250.0, -250.0}, {250.0, -250.0, -250.0} } };

    wndcls.style = CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = WindowProc;
    wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
    wndcls.hInstance = hInstance;
    wndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndcls.hbrBackground = GetStockObject(WHITE_BRUSH);
    wndcls.lpszMenuName = NULL;
    wndcls.lpszClassName = L"window";
    
    RegisterClass(&wndcls);

    hWnd = CreateWindow(L"window",
                L"mainwindow",
                WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                GetDesktopWindow(),
                NULL, 
                hInstance,
                NULL);

    UpdateWindow(hWnd);

    angle = 0.6;

    while (msg.message != WM_QUIT) {
        angle += 0.0001;

        xzRotate.eM11 = (FLOAT) cos(angle);
    xzRotate.eM12 = (FLOAT) sin(angle);
    xzRotate.eM21 = - xzRotate.eM12;
    xzRotate.eM22 = xzRotate.eM11;

    memcpy(ptArray, &Cube, sizeof(Cube3D));

    WorldPtToPagePt(&xzRotate, ptArray, 48);
    FaceArray = (LPQuad3D) ptArray;
        
    hDC = GetDC(hWnd);

    SetGraphicsMode(hDC, GM_ADVANCED);
    SetWorldTransform(hDC, &CenterTranslate);

    for (int i =0; i<4; i++) {
        v1.x = FaceArray[i].pt2.x - FaceArray[i].pt1.x;
        v1.y = FaceArray[i].pt2.y - FaceArray[i].pt1.y;
        v1.z = FaceArray[i].pt2.z - FaceArray[i].pt1.z;
        
        v2.x = FaceArray[i].pt3.x - FaceArray[i].pt1.x;
        v2.y = FaceArray[i].pt3.y - FaceArray[i].pt1.y;
        v2.z = FaceArray[i].pt3.z - FaceArray[i].pt1.z;

        v2 = CrossProduct(&v1, &v2);

        FaceAngle = DotProduct(&v2, &eyeAxis);

        if (FaceAngle <= 0)
            continue;

        switch (i) {
            case 0:
                oldBrush = SelectObject(hDC, CreateSolidBrush(RGB(0, 0, 255)));
                break;
            case 1:
                oldBrush = SelectObject(hDC, CreateSolidBrush(RGB(0, 255, 0)));
                break;
            case 2:
                oldBrush = SelectObject(hDC, CreateSolidBrush(RGB(255, 0, 0)));
                break;
            case 3:
                oldBrush = SelectObject(hDC, CreateSolidBrush(RGB(255, 0, 255)));
                break;
        }

        Rectangle(hDC, FaceArray[i].pt1.x, FaceArray[i].pt1.y,
            FaceArray[i].pt3.x, FaceArray[i].pt3.y);

         DeleteObject(SelectObject(hDC, oldBrush));
    }

    ModifyWorldTransform(hDC, NULL, MWT_IDENTITY);
    SetGraphicsMode(hDC, GM_COMPATIBLE);
    ReleaseDC(hWnd, hDC);

        while (PeekMessage(&msg, NULL, 0 ,0 , PM_REMOVE)) {

            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

LRESULT
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {

        case WM_SIZE:
            cxScreen = LOWORD(lParam);
            cyScreen = HIWORD(lParam);
            CenterTranslate.eDx = (FLOAT) LOWORD(lParam) /2.0f;
            CenterTranslate.eDy = (FLOAT) HIWORD(lParam) /2.0f;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void
WorldPtToPagePt(LPXFORM xzRotate, LPfPoint3D lpPt3D, int ptCount) {
    for (int i=0; i<ptCount; i++) {
        lpPt3D[i].x = (double) xzRotate->eM11 * lpPt3D[i].x
            + (double) xzRotate->eM21 * lpPt3D[i].z;
        lpPt3D[i].z = (double) xzRotate->eM12 * lpPt3D[i].x
            + (double) xzRotate->eM22 * lpPt3D[i].z;
        lpPt3D[i].y = -lpPt3D[i].y;
    }
}

fPoint3D
CrossProduct(LPfPoint3D a, LPfPoint3D b) {
    return (fPoint3D) { a->y * b->z - a->z * b->y,
                a->z * b->x - a->x * b->z,
                a->x * b->y - a->y * b->x };
}

double
DotProduct(LPfPoint3D a, LPfPoint3D b) {
    return  a->x * b->x + a->y * b->y + a->z * b->z;
}

double
vabs(LPfPoint3D a) {
    return sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
}