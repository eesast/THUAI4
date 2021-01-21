#include "BasicWindow.h"

BOOL BasicWindow::Init
(
    HINSTANCE hInstance, int nCmdShow,
    int x, int y, int cx, int cy, DWORD dwStyle,
    LPCTSTR c_lpszWndTitle, WNDCLASSEX wcex
)
{
    wcex.lpfnWndProc = WndProc;
    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, TEXT("Register failed!"), c_lpszError, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ִ��Ӧ�ó����ʼ��:
    if (!InitInstance(hInstance, nCmdShow, x, y, cx, cy, dwStyle, wcex.lpszClassName, c_lpszWndTitle))
    {
        MessageBox(NULL, TEXT("Window creating failed!"), c_lpszError, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}


//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ������������ھ��������������
//

BOOL BasicWindow::InitInstance(HINSTANCE hInstance, int nCmdShow, int x, int y, int cx, int cy, DWORD dwStyle, LPCTSTR c_lpszWndClassName, LPCTSTR c_lpszWndTitle)
{
    m_hInst = hInstance;  //��ʵ������洢�ڳ�Ա������

    HWND hWnd = CreateWindow(c_lpszWndClassName, c_lpszWndTitle, dwStyle,
                             x, y, cx, cy, NULL, NULL, hInstance, this);

    if (!hWnd)
    {
        return FALSE;
    }

    m_hWnd = hWnd;    //�������ھ���洢�ڳ�Ա������

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK BasicWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BasicWindow* thisWnd = NULL;
    if (message == WM_CREATE)
    {
        thisWnd = (BasicWindow*)(((LPCREATESTRUCT)lParam)->lpCreateParams);     //��ȡ���ڶ���ָ��
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

