#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <iostream>
#include <sstream>

UINT classStyles[] = { CS_BYTEALIGNCLIENT, CS_BYTEALIGNWINDOW, CS_CLASSDC, CS_DBLCLKS, CS_DROPSHADOW, CS_GLOBALCLASS, CS_HREDRAW,
    CS_NOCLOSE, CS_OWNDC, CS_PARENTDC, CS_SAVEBITS, CS_VREDRAW };
const char* lpszClassStyles[] = { "CS_BYTEALIGNCLIENT", "CS_BYTEALIGNWINDOW", "CS_CLASSDC", "CS_DBCLKS", "CS_DROPSHADOW", "CS_GLOBALCLASS", "CS_HREDRAW",
    "CS_NOCLOSE", "CS_OWNDC", "CS_PARENTDC", "CS_SAVEBITS", "CS_VREDRAW" };

int main(int argc, char **argv)
{
    WNDCLASSEXA wcex = { sizeof(WNDCLASSEXA) };
    unsigned char lpszwcexStyles[300];
    std::stringstream lpszStream;

    if (2 != argc)
    {
        std::cout << "Invalid Arguements!" << std::endl;
        return -1;
    }
    if (!GetClassInfoExA(GetModuleHandleA(NULL), argv[1], &wcex))
    {
        std::cout << "Couldn't find global window class!" << std::endl;
        return -1;
    }
    for (int i =0; i<_countof(classStyles); i++)
        if (classStyles[i] & wcex.style) lpszStream << lpszClassStyles << ", ";
    lpszStream.seekp(lpszStream.tellp() - std::streampos(2));
    std::printf("style: %s\r\n"
                "lpfnWndProc: %llx\r\n"
                "cbClsExtra: %d\r\n"
                "cbWndExtra: %d\r\n"
                "hInstance: %llx\r\n"
                "hIcon: %llx\r\n"
                "hbrBackground: %llx\r\n"
                "lpszMenuName: %llx\r\n"
                "lpszClassName: %llx\r\n"
                "hIconSm: %llx\r\n",
                lpszStream.str().c_str(), wcex.lpfnWndProc, wcex.cbClsExtra, wcex.cbWndExtra, wcex.hInstance,
                wcex.hIcon, wcex.hCursor, wcex.hbrBackground, wcex.lpszMenuName, wcex.lpszClassName,
                wcex.hIconSm);
    return 0;
}
