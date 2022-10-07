rc resource.rc
cl /Fe"tool" main.cpp /link user32.lib gdi32.lib comctl32.lib comdlg32.lib Gdiplus.lib Shell32.lib Ole32.lib resource.res