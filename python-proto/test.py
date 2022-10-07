import win32pythonproto as win32

button = win32.ChildWindow(win32.WC_BUTTON, u"Hello Button?", 0, 0,
	500, 100)
listview = win32.ChildWindow(win32.WC_LISTVIEW, u"", 0, 100, 500, 100,
	win32.WS_BORDER | win32.LVS_LIST)
combobox = win32.ChildWindow(win32.WC_COMBOBOX, u"", 0, 200, 500, 100,
	win32.CBS_DROPDOWNLIST)
static = win32.ChildWindow(win32.WC_STATIC, u"Static", 0, 300, 500, 100,
	win32.WS_BORDER | win32.SS_LEFT)
edit = win32.ChildWindow(win32.WC_EDIT, u"Edit", 0, 400, 500, 100,
	win32.WS_BORDER)

Wnd = win32.MainWindow(u"Hello", 500, 500)
Wnd.addChild(button)
Wnd.addChild(listview)
Wnd.addChild(combobox)
Wnd.addChild(static)
Wnd.addChild(edit)

print(Wnd.cx)

Wnd.openWindow()