#ifndef UI_H

#define UI_H

#include "BasicWindow.h"

class UI : public BasicWindow
{
public:

	int Begin();

private:

	virtual bool MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

	int width = 0;
	int height = 0;
	POINT pixelPerCell;
	int appendCy = 0;

	HDC hdcMem = NULL;
	HBITMAP hBmMem = NULL;

	int paintTimerID = 9999;

	long long playerID = 0;
};

#endif
