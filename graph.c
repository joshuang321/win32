#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#define IDM_PROM 4000
#define MAX_CXCLIENT 4000

struct function
{
    float coefficient;
    float power;
    float basel;
    float valuel;
    float baseexp;
};
float natloge(float);
BOOL Prom(struct function *arg);
float Graph(int,float,struct function *arg);
BOOL RePaintGraph(HDC,float,long,int,float,float,struct function *arg);
BOOL RePaintScale(HDC,long,long,float,float,float);
LRESULT CALLBACK yep(HWND,UINT,WPARAM,LPARAM);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
 PSTR szCmdLine, int iCmdShow)
{
char option;
HWND hwnd;
MSG msg;
WNDCLASSEX graph;
system("cls");
TCHAR WindowName1[] = TEXT("Graph");
	
	graph.cbSize = sizeof(WNDCLASSEX);
    graph.style         = CS_HREDRAW | CS_VREDRAW;
    graph.lpfnWndProc   = (WNDPROC)yep;
    graph.cbClsExtra    = 0;
    graph.cbWndExtra    = 0;
    graph.hInstance     = hInstance;
    graph.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    graph.hCursor       = LoadCursor(NULL,IDC_ARROW);
    graph.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
    graph.lpszMenuName  = NULL;
    graph.lpszClassName = WindowName1;
    graph.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
	
    
    if(!RegisterClassEx(&graph)) {
        return 0;
	}
    hwnd = CreateWindowEx(WS_EX_COMPOSITED,
	WindowName1,WindowName1,WS_OVERLAPPED,
    CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL, hInstance, NULL) ; 
	
    ShowWindow(hwnd,SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);
    while (GetMessage (&msg,NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK yep(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    static struct function function;
    HMENU hMenu;
    HMENU hDrop;
    HDC hdc;
    static int FLOAT;
    static float SCALE;
    static long X;
    
    static float cxRatio;
    static float cyRatio;
    
    static long cxClient;
    static long cyClient;
    
    static POINT mouse1;
    static POINT mouse2;
    
    static HINSTANCE hInstance;
    PAINTSTRUCT ps;
    switch (message)
    {
        case WM_CREATE:
          hMenu   = CreateMenu();
          hDrop   = CreateMenu();
          AppendMenu(hDrop,MF_STRING,IDM_PROM,"Prom Console");
          AppendMenu(hMenu,MF_POPUP,hDrop,"PeepoGlad");
          SetMenu(hwnd,hMenu);
          function.coefficient = 3.0;
          function.power       = 3.0;
          function.basel       = 0.0;
          function.valuel      = 0.0;
          function.baseexp     = 0.0;
          cxRatio              = 0.5;
          cyRatio              = 0.5;
          FLOAT                = 40;
          return 0;
        case WM_LBUTTONDOWN:
          mouse1.x = LOWORD(lParam);
          mouse1.y = HIWORD(lParam);
          return 0;
        case WM_MOUSEMOVE:
          if (wParam & MK_LBUTTON)
          {
              mouse2.x = LOWORD(lParam) - mouse1.x;
              mouse2.y = HIWORD(lParam) - mouse1.y;
              X    -= mouse2.x;
              cxRatio += (float)mouse2.x/(float)cxClient;
              cyRatio += (float)mouse2.y/(float)cxClient;
              mouse1.x = LOWORD(lParam);
              mouse1.y = HIWORD(lParam);
          }
          InvalidateRect(hwnd,NULL,TRUE);
          return 0;
        
        case WM_SIZE:
          cxClient = LOWORD(lParam);
          cyClient = HIWORD(lParam);
          SCALE    = (float)FLOAT/cxClient;
          InvalidateRect(hwnd,NULL, TRUE);
          return 0;
        case WM_MOUSEWHEEL:
          if ((short)HIWORD(wParam) > 0 && FLOAT <90)
              FLOAT++;
          else if ((short)HIWORD(wParam) < 0 && FLOAT >10)
              FLOAT--;
          SCALE = (float)FLOAT/cxClient;
          InvalidateRect(hwnd,NULL, TRUE);
          return 0;
        case WM_COMMAND:
          hMenu = GetMenu(hwnd);
          switch(LOWORD(wParam))
          {
        case IDM_PROM:
              Prom(&function);
              break;
          }
          InvalidateRect(hwnd,NULL, TRUE);
          return 0;
        case WM_PAINT:
          hdc = BeginPaint(hwnd, &ps);
          SetViewportOrgEx(hdc,cxClient * cxRatio,cyClient * cyRatio,NULL); 
          RePaintScale(hdc,cxClient,cyClient,cxRatio,cyRatio,SCALE);
          // RePaintGraph(hdc,SCALE,X,cxClient,cyRatio,cyClient,&function);
          EndPaint(hwnd, &ps);
         return 0;

        case WM_DESTROY:
          return 0;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);
}

BOOL RePaintGraph(HDC hdc,float scale,long x,int cxClient,float cyRatio,float cyClient,struct function *function)
{
    POINT points[MAX_CXCLIENT];
    int X = (float) x - cxClient/2.0;
	const float MAXY = (float)(cyRatio * cyClient) + (cxClient/2);
    
    for (int i =0;i<cxClient;i++)
    {
        points[i].x = X + i;
        points[i].y =-Graph(X+i,scale,function) * function->coefficient;
    }

    HPEN hPen = CreatePen(PS_SOLID,5,RGB(255,0,0));
    SelectObject(hdc,hPen);    
    Polyline(hdc,points,cxClient);
    DeleteObject(hPen);
}

RePaintScale(HDC hdc,long cxClient,long cyClient,float cxRatio,float cyRatio,float SCALE)
{
    int unit = 10;
    int intx;
    int inty;
    POINT position;
    position.x = (1-cxRatio)*cxClient;
    position.y = cyRatio*cyClient;
    long scale = 1/SCALE *unit/2;
    
    if (! position.x%scale)
        position.x -= position.x%scale;
    if (! position.y%scale)
        position.y -= position.y%scale;
    intx = position.x/scale;
    inty = position.y/scale;
    for (int i =0;i<cxClient/scale +2;i++)
    {
        MoveToEx(hdc,(-i+intx+1) * scale,-10,NULL);
        LineTo(hdc,(-i+intx+1) * scale,10);
    }
    for (int i =0;i<cyClient/scale +2;i++)
    {
        MoveToEx(hdc,-10,-(-i+inty+1)* scale,NULL);
        LineTo(hdc,10,-(-i+inty+1)* scale);
    }
    
    MoveToEx(hdc,0,-cyRatio * cyClient,NULL);
    LineTo(hdc,0,(1 - cyRatio) * cyClient);
    MoveToEx(hdc,-(cxRatio) * cxClient,0,NULL);
    LineTo(hdc,(1 - cxRatio) * cxClient,0); 
}

float Graph(int x,float scale,struct function *function)
{ 

    float result;
    float X = (float)x * scale;
    
    if (function->power >0)
    {
        result = pow(X,function->power);
    }
    else if (function->baseexp >0)
        result = pow(function->baseexp,X);
    else if (function->valuel >0)
    {
        result = natloge(function->valuel)/natloge(X);
    }
    else if (function ->basel >0)
    {
        result = natloge(X)/natloge(function->basel);
    }
        
    float scaling = result/scale;
    return scaling;
}

float natloge(float x)
{
    float result = 0;
    float a;
    x--;
    for (float i = 1;i<100;i++)
    {
        result += pow(-1.0,i+1) * pow(x,i)/i;
    }
    return result;
}

BOOL Prom(struct function *function)
{
    char reply[20];
    system("cls");
    


    printf("What function would you like?\n\n");
    printf("a^x\n");
    printf("x^a\n");
    printf("logax\n");
    printf("logxa\n\n");
    gets(reply);
    fflush(stdin);
    system("cls");
    if(!strcmp(reply,"a^x"))
    {
        printf("coefficient: ");
        scanf("%f",&function->coefficient);
        fflush(stdin);
        printf("base: ");
        scanf("%f",&function->baseexp);
        fflush(stdin);
        function->valuel = 0;
        function->basel  = 0;
        function->power = 0;
    }
    else if(!strcmp(reply,"x^a"))
    {
        printf("coefficient: ");
        scanf("%f",&function->coefficient);
        fflush(stdin);
        printf("power: ");
        scanf("%f",&function->power);
        fflush(stdin);
        function->valuel   = 0;
        function->baseexp = 0;
        function->basel  = 0;
    }
    else if(!strcmp(reply,"logax"))
    {
        printf("coefficient: ");
        scanf("%f",&function->coefficient);
        fflush(stdin);
        printf("base :");
        scanf("%f",&function->basel);
        fflush(stdin);
        function->valuel   = 0;
        function->baseexp = 0;
        function->power = 0;
    }
        
    else if(!strcmp(reply,"logxa"))
    {
        printf("coefficient: ");
        scanf("%f",&function->coefficient);
        fflush(stdin);
        printf("value: ");
        scanf("%f",&function->valuel);
        fflush(stdin);
        function->basel    = 0;
        function->power    = 0;
        function->baseexp = 0;
    }
    else
    {
        printf("Error!");
        Sleep(1500);
    }
    system("cls");
    return FALSE;
}
