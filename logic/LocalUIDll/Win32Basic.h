#ifndef WIN32_BASIC_H

#define WIN32_BASIC_H


//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
//#endif	// #ifndef WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#ifdef _UNICODE
#define _tstring wstring
#define _tostringstream wostringstream
#else // _UNICODE
#define _tstring string
#define _tostringstream ostringstream
#endif // _UNICODE

#endif	//#ifndef WIN32_BASIC_H
