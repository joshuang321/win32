#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <string>

#include <Python.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD

	PyObject *pszClassName,
		*pszWindowName;

	int dwStyle;
	unsigned int x, y,
		cx, cy;
} ChildWindowObject;

typedef struct {
	PyObject_HEAD

	PyObject *pszWindowName,
		*childWnds;
	unsigned int cx, cy;
} MainWindowObject;

WCHAR pszMainWindowName[] = L"MainWindow_win32pythonproto";
LRESULT WINAPI MainWindowProc(HWND, UINT, WPARAM, LPARAM);

static void ChildWindow_dealloc(ChildWindowObject *self) {

	Py_XDECREF(self->pszClassName);
	Py_XDECREF(self->pszWindowName);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static void MainWindow_dealloc(MainWindowObject *self) {

	Py_XDECREF(self->pszWindowName);
	Py_XDECREF(self->childWnds);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ChildWindow_new(PyTypeObject *type,
		PyObject *args,
		PyObject *kwds) {

	ChildWindowObject *self = NULL;

	self = (ChildWindowObject *)type->tp_alloc(type, 0);
	if (self) {

		self->pszClassName = PyUnicode_FromString("");
		if (!self->pszClassName) {

			Py_DECREF(self);
			return NULL;
		}

		self->pszWindowName = PyUnicode_FromString("");

		if (!self->pszWindowName) {

			Py_DECREF(self);
			return NULL;
		}

		self->dwStyle = 0;
		self->x = 0;
			self->y = 0;
		self->cx = 0;
			self->cy = 0;
	}

	return (PyObject *)self;
}

static PyObject *MainWindow_new(PyTypeObject *type,
	PyObject *args,
	PyObject *kwds) {

	MainWindowObject *self = NULL;

	self = (MainWindowObject *)type->tp_alloc(type, 0);
	if (self) {

		self->pszWindowName = PyUnicode_FromString("");
		if (!self->pszWindowName) {

			Py_DECREF(self);
			return NULL;
		}

		self->childWnds = PyList_New(0);

		if (!self->childWnds) {
			
			Py_DECREF(self);
			return NULL;
		}

	}

	self->cx = 0;
		self->cy = 0;

	return (PyObject *)self;
}

static int ChildWindow_init(ChildWindowObject *self,
		PyObject *args,
		PyObject *kwds) {

	static char *kwlist[] = { (char *)"pszClassName", (char *)"pszWindowName",
		(char *)"x",
			(char *)"y",
		(char *)"cx",
			(char *)"cy",
		(char *)"dwStyle",
		NULL };

	PyObject *pszClassName = NULL,
		*pszWindowName = NULL,
		*temp = NULL;


	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|UUIIIII", kwlist,
			&pszClassName,
			&pszWindowName,
			&self->x,
				&self->y,
			&self->cx,
				&self->cy,
			&self->dwStyle))
		return -1;

	if (pszClassName) {

		temp = self->pszClassName;
		Py_INCREF(pszClassName);
		self->pszClassName = pszClassName;
		Py_XDECREF(temp);
	}

	if (pszWindowName) {

		temp = self->pszWindowName;
		Py_INCREF(pszWindowName);
		self->pszWindowName = pszWindowName;
		Py_XDECREF(temp);
	}
	return 0;
}

static int MainWindow_init(MainWindowObject *self,
		PyObject *args,
		PyObject *kwds) {

	static char *kwlist[] = { (char *)"pszWindowName", (char *)"cx",
		(char *)"cy",
		NULL };

	PyObject *pszWindowName = NULL,
		*temp = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|UII", kwlist,
			&pszWindowName,
			&self->cx,
				&self->cy))
		return -1;

	if (pszWindowName) {

		temp = self->pszWindowName;
		Py_INCREF(pszWindowName);
		self->pszWindowName = pszWindowName;
		Py_XDECREF(temp);
	}

	return 0;
}

static PyMemberDef ChildWindow_members[] = {

	{ "pszClassName", T_OBJECT_EX, offsetof(ChildWindowObject, pszClassName),
		READONLY,
		"Class Name" },
	{ "pszWindowName", T_OBJECT_EX, offsetof(ChildWindowObject, pszWindowName),
		READONLY,
		"Window Name" },
	{ "dwStyle", T_INT, offsetof(ChildWindowObject, dwStyle), READONLY,
		"dwStyle" },
	{ "x", T_UINT, offsetof(ChildWindowObject, x), READONLY, "x" },
		{ "y", T_UINT, offsetof(ChildWindowObject, y), READONLY, "y" },
	{ "cx", T_UINT, offsetof(ChildWindowObject, cx), READONLY, "cx" },
		{ "cy", T_UINT, offsetof(ChildWindowObject, cy), READONLY, "cy" },
	{ NULL }
};

static PyMemberDef MainWindow_members[] = {
	{ "pszWindowName", T_OBJECT_EX, offsetof(MainWindowObject, pszWindowName),
		READONLY,
		"Window Name" },
	{ "childWnds", T_OBJECT_EX, offsetof(MainWindowObject, childWnds),
		0,
		"Child Windows" },
	{ "cx", T_UINT, offsetof(MainWindowObject, cx), READONLY, "cx" },
		{ "cy", T_UINT, offsetof(MainWindowObject, cy), READONLY, "cy" },
	{ NULL }
};

static PyTypeObject ChildWindowType = {

	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "win32pythonproto.ChildWindow",
	.tp_basicsize = sizeof(ChildWindowObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)ChildWindow_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = PyDoc_STR("Child prototype Window"),
	.tp_members = ChildWindow_members,
	.tp_init = (initproc)ChildWindow_init,
	.tp_new = ChildWindow_new
};

static PyObject *MainWindow_addChild(MainWindowObject *self,
	PyObject *childWndObj) {

	if (PyObject_TypeCheck(childWndObj, &ChildWindowType)) {

		Py_INCREF(childWndObj);
		PyList_Append(self->childWnds, childWndObj);
	}

	Py_RETURN_NONE;
}

static PyObject *MainWindow_openWindow(MainWindowObject *self) {

	Py_ssize_t szWndLen = 0Ui64;
	wchar_t *pszWindowName = NULL,
		*pszClassName = NULL;
	RECT rcClient = { 0 };
	HWND hWnd = NULL;
	MSG Msg = { 0 };
	ChildWindowObject *pChildWindowObj = NULL;

	if (self->pszWindowName) {

		pszWindowName = PyUnicode_AsWideCharString(self->pszWindowName,
			&szWndLen);
		if (pszWindowName) {

			rcClient.right = self->cx;
			rcClient.bottom = self->cy;
			AdjustWindowRect(&rcClient, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				FALSE);

			hWnd = CreateWindowExW(0, pszMainWindowName, pszWindowName,
				WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				rcClient.right - rcClient.left,
					rcClient.bottom - rcClient.top,
				NULL,
				NULL,
				GetModuleHandleW(NULL),
				NULL);

			PyMem_Free(pszWindowName);

			if (hWnd) {

				for (Py_ssize_t i=0;
					i < PyList_Size(self->childWnds);
					i++) {

					pChildWindowObj = (ChildWindowObject *)PyList_GetItem(
						self->childWnds,
						i);
					Py_INCREF(pChildWindowObj);

					pszWindowName = PyUnicode_AsWideCharString(
						pChildWindowObj->pszWindowName,
						&szWndLen);
					pszClassName = PyUnicode_AsWideCharString(
						pChildWindowObj->pszClassName,
						&szWndLen);

					CreateWindowExW(0, pszClassName, pszWindowName,
						WS_CHILD | WS_VISIBLE | pChildWindowObj->dwStyle,
						pChildWindowObj->x, pChildWindowObj->y,
						pChildWindowObj->cx, pChildWindowObj->cy,
						hWnd,
						NULL,
						GetModuleHandleW(NULL),
						NULL);

					Py_DECREF(pChildWindowObj);
					PyMem_Free(pszWindowName);
					PyMem_Free(pszClassName);
				}

				while (GetMessageW(&Msg, NULL, 0, 0)) {

					TranslateMessage(&Msg);
						DispatchMessageW(&Msg);
				}
			}
		}
	}

	Py_RETURN_NONE;
}

static PyMethodDef MainWindow_methods[] = {
	{ "addChild", (PyCFunction)MainWindow_addChild, METH_O,
		"Add a Child Window to the Main Window" },
	{ "openWindow", (PyCFunction)MainWindow_openWindow, METH_NOARGS,
		"Opens the Main Window" },
	{ NULL }
};

static PyTypeObject MainWindowType = {

	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "win32pythonproto.MainWindow",
	.tp_basicsize = sizeof(MainWindowObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)MainWindow_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = PyDoc_STR("Main prototype Window"),
	.tp_methods = MainWindow_methods,
	.tp_members = MainWindow_members,
	.tp_init = (initproc)MainWindow_init,
	.tp_new = MainWindow_new
};

static PyModuleDef PythonProtoModule = {

	.m_base = PyModuleDef_HEAD_INIT,
	.m_name = "win32pythonproto",
	.m_doc = "Python-C extension to create prototype GUIs for Windows",
	.m_size = -1
};

LRESULT WINAPI MainWindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	if (WM_CREATE == uMsg) {

		LPCREATESTRUCTW pCSw = NULL;
		SIZE *pSize = NULL;

		pCSw = (LPCREATESTRUCTW)lParam;
		pSize = (SIZE *)PyMem_Malloc(sizeof(SIZE));
		if (pSize) {

			pSize->cx = pCSw->cx;
				pSize->cy = pCSw->cy;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pSize);
			return 0;
		}
		return -1;
	}
	else if (WM_GETMINMAXINFO == uMsg) {

		SIZE *pSize = NULL;
		PMINMAXINFO pmmi = NULL;

		pSize = (SIZE *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		pmmi = (PMINMAXINFO)lParam;

		if (pSize) {

			pmmi->ptMaxSize.x = pSize->cx;
				pmmi->ptMaxSize.y = pSize->cy;
			pmmi->ptMinTrackSize.x = pSize->cx;
				pmmi->ptMinTrackSize.y = pSize->cy;
			pmmi->ptMaxTrackSize.x = pSize->cx;
				pmmi->ptMaxTrackSize.y = pSize->cy;
		}
	}
	else if (WM_DESTROY == uMsg) {

		LONG_PTR pMem = NULL;

		pMem = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		if (pMem)
			PyMem_Free((void *)pMem);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int InitControlStylesConstant(PyObject *m) {

	if (!PyModule_AddIntMacro(m, WS_BORDER) &&
		!PyModule_AddIntMacro(m, BS_CHECKBOX) &&
		!PyModule_AddIntMacro(m, BS_AUTOCHECKBOX) &&
		!PyModule_AddIntMacro(m, LVS_LIST) &&
		!PyModule_AddIntMacro(m, CBS_DROPDOWNLIST) &&
		!PyModule_AddIntMacro(m, SS_LEFT)) {

		return 1;
	}

	return 0;
}

int InitControlConstants(PyObject *m) {

	if (!PyModule_AddStringMacro(m, WC_BUTTON) &&
		!PyModule_AddStringMacro(m, WC_LISTVIEW) &&
		!PyModule_AddStringMacro(m, WC_COMBOBOX) &&
		!PyModule_AddStringMacro(m, WC_STATIC) &&
		!PyModule_AddStringMacro(m, WC_EDIT) &&
		InitControlStylesConstant(m)) {

		return 1;
	}

	return 0;
}

PyMODINIT_FUNC PyInit_win32pythonproto(void) {

	PyObject *m = NULL;
	WNDCLASSEXW wcex = { 0 };
	ATOM pszClassName = 0;

	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.lpfnWndProc = MainWindowProc;
	wcex.hInstance = (HINSTANCE)GetModuleHandleW(NULL);
	wcex.hIcon = LoadIconW(NULL, (PWSTR)IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, (PWSTR)IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszClassName = pszMainWindowName;
	wcex.hIconSm = LoadIconW(NULL, (PWSTR)IDI_APPLICATION);

	pszClassName = RegisterClassExW(&wcex);

	if (pszClassName &&
		!PyType_Ready(&ChildWindowType) &&
		!PyType_Ready(&MainWindowType)) {

		m = PyModule_Create(&PythonProtoModule);
		if (m) {

			Py_INCREF(&ChildWindowType);
			if (!PyModule_AddObject(m, "ChildWindow", (PyObject *)
					&ChildWindowType)) {

				Py_INCREF(&MainWindowType);
				if (!PyModule_AddObject(m, "MainWindow", (PyObject *)
						&MainWindowType)) {

					if (InitControlConstants(m)) {

						return m;
					}
				}		
				Py_DECREF(&MainWindowType);
			}
			Py_DECREF(&ChildWindowType);
		}
		Py_XDECREF(m);
	}

	return NULL;
}