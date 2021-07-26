//////////////////////////////
//
//  封装Windows API
//

#ifndef BASIC_WINDOW_H

#define BASIC_WINDOW_H

#include "Win32Basic.h"

//错误提示

static LPCTSTR c_lpszError = TEXT("Error");								//错误

//窗口抽象类
class BasicWindow
{

protected:

	//关于窗口

	HINSTANCE m_hInst;					//当前实例
	HWND m_hWnd = NULL;				//主窗口句柄

	//创建窗口
	BOOL Init
	(
		HINSTANCE hInstance, int nCmdShow,
		int x, int y, int cx, int cy, DWORD dwStyle,
		LPCTSTR c_lpszWndTitle, WNDCLASSEX wcex, HWND hWndParent
	);

	//处理了消息返回true，没有处理则返回false
	virtual bool MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

private:

	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, int x, int y, int cx, int cy, DWORD dwStyle, LPCTSTR c_lpszWndClassName, LPCTSTR c_c_lpszWndTitle, HWND hWndParent);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

#endif // #ifndef BASIC_WINDOW_H
