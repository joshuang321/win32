rc resource.rc
cl /Fe"check" /std:c++20 main.cpp /link user32.lib gdi32.lib ComCtl32.lib resource.res