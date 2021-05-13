#include "..\..\logic\LocalUIDll\BasicWindow.h"

BOOL BasicWindow::Init
(
	HINSTANCE hInstance, int nCmdShow,
	int x, int y, int cx, int cy, DWORD dwStyle,
	LPCTSTR c_lpszWndTitle, WNDCLASSEX wcex, HWND hWndParent
)
{
	wcex.lpfnWndProc = WndProc;
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, TEXT("Register failed!"), c_lpszError, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow, x, y, cx, cy, dwStyle, wcex.lpszClassName, c_lpszWndTitle, hWndParent))
	{
		MessageBox(NULL, TEXT("Window creating failed!"), c_lpszError, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}


//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄、主窗口句柄并创建主窗口
//

BOOL BasicWindow::InitInstance(HINSTANCE hInstance, int nCmdShow, int x, int y, int cx, int cy, DWORD dwStyle, LPCTSTR c_lpszWndClassName, LPCTSTR c_lpszWndTitle, HWND hWndParent)
{
	m_hInst = hInstance;  //将实例句柄存储在成员变量中

	HWND hWnd = CreateWindow(c_lpszWndClassName, c_lpszWndTitle, dwStyle,
		x, y, cx, cy, hWndParent, NULL, hInstance, this);

	if (!hWnd)
	{
		return FALSE;
	}

	m_hWnd = hWnd;    //将主窗口句柄存储在成员变量中

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK BasicWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BasicWindow* thisWnd = NULL;
	if (message == WM_CREATE)
	{
		thisWnd = (BasicWindow*)(((LPCREATESTRUCT)lParam)->lpCreateParams);     //获取窗口对象指针
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)thisWnd);
	}
	else thisWnd = (BasicWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (thisWnd == NULL) return DefWindowProc(hWnd, message, wParam, lParam);
	if (!thisWnd->MessageProc(hWnd, message, wParam, lParam))
	{
		if (message == WM_DESTROY)
		{
			thisWnd->m_hWnd = NULL;
			PostQuitMessage(0);
			return 0;
		}
		else return DefWindowProc(hWnd, message, wParam, lParam);
	}
	if (message == WM_DESTROY) thisWnd->m_hWnd = NULL;
	return 0;
}

