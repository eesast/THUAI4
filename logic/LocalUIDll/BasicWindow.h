//////////////////////////////
//
//  ��װWindows API
//

#ifndef BASIC_WINDOW_H

#define BASIC_WINDOW_H

#include "Win32Basic.h"

//������ʾ

static LPCTSTR c_lpszError = TEXT("Error");								//����

//���ڳ�����
class BasicWindow
{

protected:

	//���ڴ���

	HINSTANCE m_hInst;					//��ǰʵ��
	HWND m_hWnd = NULL;				//�����ھ��

	//��������
	BOOL Init
	(
		HINSTANCE hInstance, int nCmdShow,
		int x, int y, int cx, int cy, DWORD dwStyle,
		LPCTSTR c_lpszWndTitle, WNDCLASSEX wcex
	);

	//��������Ϣ����true��û�д����򷵻�false
	virtual bool MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

private:

	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, int x, int y, int cx, int cy, DWORD dwStyle, LPCTSTR c_lpszWndClassName, LPCTSTR c_c_lpszWndTitle);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

#endif // #ifndef BASIC_WINDOW_H


