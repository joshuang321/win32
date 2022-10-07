cl /c debug.cpp /link Kernel32.lib
rc resource.rc
cl /EHsc /Fe"graph" /W4 main.cpp /link user32.lib Gdiplus.lib Uxtheme.lib Comdlg32.lib resource.res