debug:
	cl /DWIN32_LEAN_AND_MEAN /DDEBUG /Fe:killwnd /EHsc /std:c++20 main.cpp /W4 /link user32.lib kernel32.lib /MANIFESTUAC:"level='highestAvailable' uiAccess='true'"
	del *.obj

release:
	cl /DWIN32_LEAN_AND_MEAN /DNDEBUG /Fe:killwnd /EHsc /std:c++20 main.cpp /link user32.lib kernel32.lib /MANIFESTUAC:"level='highestAvailable' uiAccess='true'"
	del *.obj