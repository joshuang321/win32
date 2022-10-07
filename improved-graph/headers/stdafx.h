#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <uxtheme.h>
#include <commdlg.h>

#include <vector>
#include <string>

using namespace Gdiplus;

#if defined(__DEBUG)
#include "headers/debug.h"
#endif

#include "resource.h"