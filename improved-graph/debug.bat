cl /c debug.cpp /link Kernel32.lib
rc resource.rc
cl /EHsc /Fe"graph" /W4 /D__DEBUG=1 main.cpp /link user32.lib Gdiplus.lib Uxtheme.lib Comdlg32.lib debug.obj resource.res