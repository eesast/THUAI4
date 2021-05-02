/*
* This file may contains lots of errors from Intelligense.
* But it doesn't matter because the error only occurs when coding but not when compiling.
* So it can pass the compilation.
* The specific reason has been written at the position of one of the errors.
* So ignore the error "expression must have pointer-to-object or handle-to-C++/CLI-array type"
* as long as it can pass the compilation!
*/

#include "UI.h"
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <type_traits>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")

#using <Google.Protobuf.dll>
#include "..\PlayBackPlayerResources\resource.h"

#define COMPATIBLE_WITH_ALL_OLD_PLAYBACK_VERSION

UI::UI()
{
	for (auto&& oneTeamScore : teamScores)
	{
		oneTeamScore = 0;
	}
	appendCy = GetSystemMetrics(SM_CYMIN) + GetSystemMetrics(SM_CYMENU);
}

UI::MessageReaderWrapper::MessageReaderWrapper(System::String^ fileName)
{
	messageReader = gcnew playback::MessageReader(fileName);
}

void UI::GetPMR(System::String^ fileName, std::shared_ptr<volatile bool> messager)
{
	if (finishConstruct) return;
	bool success = false;
	try
	{
		System::Console::WriteLine(fileName);
		MessageReaderWrapper gm(fileName);
		successConstruct = true;
		gm.recentMsg = nullptr;
		pMR = &gm;
		finishConstruct = true;
		while (*messager) { std::this_thread::sleep_for(std::chrono::seconds(2)); }
		std::this_thread::sleep_for(std::chrono::seconds(5));	// Wait for all accessment to gm to end.
		success = true;
	}
	catch ([[maybe_unused]] playback::FileFormatNotLegalException^ e)
	{
		MessageBox(NULL, TEXT("This playback file has been destroyed!"), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	catch (System::Exception^ e)
	{
		auto msgBytes = System::Text::Encoding::Unicode->GetBytes(e->Message);
		auto len = msgBytes->Length;
		uint8_t* pMsg = nullptr;
		try
		{
			pMsg = new uint8_t[len + sizeof(WCHAR)];
			for (decltype(len) i = 0; i < len; ++i)
			{
				pMsg[i] = msgBytes[i];
			}
			for (int i = 0; i < sizeof(WCHAR); ++i)
			{
				pMsg[len + i] = 0;
			}
			MessageBoxW(NULL, (WCHAR*)pMsg, L"Error", MB_OK | MB_ICONERROR);
		}
		catch (...)
		{
			MessageBox(NULL, TEXT("Unknown error!"), TEXT("Error"), MB_OK | MB_ICONERROR);
		}
		finally { delete[] pMsg; }
	}
	catch (std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		MessageBox(NULL, TEXT("Unknown error!"), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	finally
	{
		if (!success) finishConstruct = true;
	}
}

int UI::Begin(System::String^ initialFileName)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = MAKEINTRESOURCE(IDM_MAINMENU);
	wcex.lpszClassName = TEXT("THUAIPlayBackWindow");
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	width = GetSystemMetrics(SM_CXMIN);
	height = GetSystemMetrics(SM_CYMIN);

	if (initialFileName != nullptr)
	{
		auto fileNameBytes = System::Text::Encoding::Unicode->GetBytes(initialFileName);
		uint8_t* fileNameStr = new uint8_t[fileNameBytes->Length + 2];
		ZeroMemory(fileNameStr, fileNameBytes->Length + 2);
		for (int i = 0; i < fileNameBytes->Length; ++i)
		{
			fileNameStr[i] = fileNameBytes[i];
		}
		m_initialFileName = (WCHAR*)fileNameStr;
		delete[] fileNameStr;
	}

	Init(GetModuleHandle(NULL), SW_NORMAL, 0, 0, width, height,
		WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
		TEXT("THUAI Playback"), wcex);

	MSG msg;

	HMODULE hRes = LoadLibrary(TEXT("PlayBackPlayerResources.dll"));
	if (hRes == NULL)
	{
		MessageBox(m_hWnd, TEXT("Cannot load PlayBackPlayerResources.dll"), TEXT("Error"), MB_OK);
		::exit(1);
	}

	HACCEL hAccel = LoadAccelerators(hRes, MAKEINTRESOURCE(IDA_MAINMENUACCEL));
	HMENU hMenu = LoadMenu(hRes, MAKEINTRESOURCE(IDM_MAINMENU));
	SetMenu(m_hWnd, hMenu);
	hBmBkGnd = (HBITMAP)LoadImage(hRes, MAKEINTRESOURCE(IDB_BKGND), IMAGE_BITMAP, 0, 0, 0);
	if (hBmBkGnd == NULL)
	{
		if (MessageBox(m_hWnd, TEXT("The library PlayBackPlayerResources.dll has been destroyed, continue?"), TEXT("Error"), MB_YESNO | MB_ICONERROR) != IDYES)
		{
			exit(1);
		}
	}
	else
	{
		GetObject(hBmBkGnd, sizeof(bkGnd), &bkGnd);
	}

	if (!chooseFileDlg.Begin(hRes, MAKEINTRESOURCE(IDD_CHOOSESPEEDDLG), m_hWnd))
	{
		if (MessageBox(m_hWnd, TEXT("The library PlayBackPlayerResources.dll might have been destroyed, continue?"), TEXT("Error"), MB_YESNO | MB_ICONERROR) != IDYES)
		{
			exit(1);
		}
	}

	FreeLibrary(hRes);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (IsDialogMessage(chooseFileDlg.GetHangle(), &msg)) continue;
		if (!TranslateAccelerator(m_hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

void UI::ChooseFile(bool first)
{
chooseFile:
	WCHAR fileName[MAX_PATH + 2] = { 0 };
	OPENFILENAMEW ofn;
	ZeroMemory((void*)&ofn, sizeof(ofn));
	WCHAR filter[] = L"THUAI Playback Files (*.thuaipb)\0*.thuaipb\0\0";

	if (first)
	{
		if (m_initialFileName != L"")
		{
			if (m_initialFileName.length() > MAX_PATH)
			{
				MessageBox(m_hWnd, (L"Cannot find playback file: " + m_initialFileName).c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
				return;
			}
			wcscpy(fileName, m_initialFileName.c_str());
			goto beginOpenFile;
		}
	}

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = NULL;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;

	if (!GetOpenFileNameW(&ofn))
	{
		return;
	}

	beginOpenFile:

	finishConstruct = false;
	auto newMessager = std::make_shared<volatile bool>(true);

	std::thread([this, &fileName, newMessager]
	{
		auto fileNameBytes = gcnew array<System::Byte, 1>(static_cast<int>(wcslen(fileName) * sizeof(WCHAR)));
		uint8_t* p = (uint8_t*)fileName;
		for (int i = 0; i < fileNameBytes->Length; ++i, ++p)
		{
			fileNameBytes[i] = *p;
		}
		GetPMR(System::Text::Encoding::Unicode->GetString(fileNameBytes), newMessager);
	}).detach();

	while (!finishConstruct) std::this_thread::sleep_for(std::chrono::milliseconds(1));

	if (!successConstruct)
	{
		finishConstruct = false;
		if (!first) goto chooseFile;
		else return;
	}
	successConstruct = false;

	*msgParserMessager = false;
	msgParserMessager = newMessager;

	std::thread
	(
		[this, newMessager]
		{
			playback::MessageReader^ mr = pMR->MessageReader();

			uint64_t timeExceedCount = 0UL;
			uint64_t MaxTolerantTimeExceedCount = 5;
			auto beginTime = System::Environment::TickCount64;
			auto nextTime = beginTime + chooseFileDlg.TimeInterval();

			while (*newMessager)
			{
				Communication::Proto::MessageToClient^ msg = nullptr;
				for (unsigned i = 0; i < mr->teamCount; ++i)
				{
					for (unsigned j = 0; j < mr->playerCount; ++j)
					{
						msg = mr->ReadOne();
						if (msg == nullptr) goto endParse;
						teamScores[msg->TeamID] = msg->TeamScore;
						pMR->recentMsg = msg;
					}
				}

				//if (msg != nullptr)
				//{
				//	//WINDOWINFO wi;
				//	//ZeroMemory(&wi, sizeof(wi));
				//	//wi.cbSize = sizeof(wi);
				//	//GetWindowInfo(m_hWnd, &wi);
				//	//if (wi.rcWindow.right - wi.rcWindow.left != width || wi.rcWindow.bottom - wi.rcWindow.top != height)
				//	//{
				//	//	MoveWindow(m_hWnd, wi.rcWindow.left, wi.rcWindow.top, width, height, FALSE);
				//	//}
				//}

				InvalidateRect(m_hWnd, NULL, FALSE);

				auto nowTime = System::Environment::TickCount64;
				if (nextTime >= nowTime)
				{
					timeExceedCount = 0ull;
					System::Threading::Thread::Sleep((int)(nextTime - nowTime));
				}
				else
				{
					++timeExceedCount;
					if (timeExceedCount > MaxTolerantTimeExceedCount)
					{
						System::Console::WriteLine(L"Error: Your computer runs so slow that it cannot parse the message so fast!");
						timeExceedCount = 0UL;
						nextTime = System::Environment::TickCount64;
					}
				}

				nextTime += chooseFileDlg.TimeInterval();
			}

			goto playOver;

		endParse:
			
			auto orgPMR = pMR;
			MessageBox(m_hWnd, TEXT("Game over!"), TEXT("Game over!"), MB_OK);
			if (orgPMR == pMR)
			{
				pMR = nullptr;
				InvalidateRect(m_hWnd, NULL, FALSE);
			}

		playOver:;
		}
	).detach();
}

bool UI::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{

		MoveWindow(hWnd, 0, 0, 600 * 2 + 15, 600 + appendCy, FALSE);
		if (m_initialFileName != L"") ChooseFile(true);

		break;
	}
	case WM_SIZE:
	{
		int wndWidth = LOWORD(lParam) + GetSystemMetrics(SM_CXBORDER);
		int wndHeight = HIWORD(lParam);
		int widCalBasic = wndWidth / 2;
		int heightCalBasic = wndHeight;
		basicSize = widCalBasic < heightCalBasic ? widCalBasic : heightCalBasic;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDM_ABOUT:
		{
			MessageBox(hWnd, TEXT("EESAST\nCopyright (C) 2021"), TEXT("About"), MB_OK | MB_ICONINFORMATION);
			break;
		}
		case IDM_EXIT:
		{
			SendMessage(hWnd, WM_DESTROY, wParam, lParam);
			break;
		}
		case IDM_CHOOSEFILE:
		{
			ChooseFile();
			break;
		}
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT clrec;
		GetClientRect(hWnd, &clrec);

		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hBmMem = CreateCompatibleBitmap(hdc, clrec.right - clrec.left, clrec.bottom - clrec.top);
		HBITMAP hBmMemOld = (HBITMAP)SelectObject(hdcMem, hBmMem);

		auto pMR = this->pMR;

		if (pMR != nullptr)
		{

			auto recentMsg = pMR->recentMsg;
			if (recentMsg == nullptr) goto endPaint;

			{
				auto cellColor = recentMsg->CellColors;

				/*
				* !! This is a very fuck phenomenon that below will cause an error from Intelligense when coding.
				* Because calling the default indexer 'cellColor->default[0]' is ok before cpp11
				* but not after cpp11 because 'default' becomes a keyword in cpp11.
				* So after cpp11 we can use [] directly as 'cellColor[0]' instead of using the word 'default' to call the default indexer.
				* Unfortunately and fuckingly, the Intelligense hasn't been updated by Microsoft
				* so the intelligense will give an error since this syntax is not allowed before cpp11,
				* although we are writing cpp17 codes.
				* So ignore these errors "expression must have pointer-to-object or handle-to-C++/CLI-array type"
				* from Intelligense as long as it can pass the compilation!
				*/
				int rows = cellColor->Count, cols = cellColor[0]->RowColors->Count;

				int tmpWidth = (pixelPerCell.x = basicSize / cols) * cols;
				appendCx = 15 + tmpWidth;
				width = tmpWidth + appendCx;
				height = (pixelPerCell.y = basicSize / rows) * rows + appendCy;
			}

			HPEN hPenNull = (HPEN)GetStockObject(PS_NULL);
			SelectObject(hdcMem, hPenNull);
			HBRUSH hbrBkGnd = (HBRUSH)GetStockObject(BLACK_BRUSH);
			HBRUSH hbrOld = (HBRUSH)SelectObject(hdcMem, hbrBkGnd);
			Rectangle(hdcMem, clrec.left, clrec.top, clrec.right, clrec.bottom);     //绘制背景

			//绘制地图颜色

			HBRUSH hbrColor1 = CreateSolidBrush(RGB(100, 0, 0));
			HBRUSH hbrColor2 = CreateSolidBrush(RGB(0, 100, 0));
			HBRUSH hbrColor3 = CreateSolidBrush(RGB(0, 0, 100));
			HBRUSH hbrColor4 = CreateSolidBrush(RGB(200, 200, 200));
			auto cellColor = recentMsg->CellColors;
			int rows = cellColor->Count, cols = cellColor[0]->RowColors->Count;

			for (int i = 0, topPos = 0; i < rows; ++i, topPos += pixelPerCell.y)
			{
				for (int j = 0, leftPos = 0; j < cols; ++j, leftPos += pixelPerCell.x)
				{
					switch (cellColor[i]->RowColors[j])
					{
					case Communication::Proto::ColorType::Color1: SelectObject(hdcMem, hbrColor1); break;
					case Communication::Proto::ColorType::Color2: SelectObject(hdcMem, hbrColor2); break;
					case Communication::Proto::ColorType::Color3: SelectObject(hdcMem, hbrColor3); break;
					case Communication::Proto::ColorType::Color4: SelectObject(hdcMem, hbrColor4); break;
					default: continue;
					}
					Rectangle(hdcMem, leftPos, topPos, leftPos + pixelPerCell.x, topPos + pixelPerCell.y);
				}
			}

			DeleteObject(hbrColor1);
			DeleteObject(hbrColor2);
			DeleteObject(hbrColor3);
			DeleteObject(hbrColor4);

			// 道具转换到宽字符串
			static const std::unordered_map<int, std::wstring> propToWStr
			{
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Bike,					L"SharedBike"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Amplifier,			L"Horn"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::JinKeLa,				L"SchoolBag"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Rice,					L"HappyHotPot"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::NegativeFeedback,		L"Shield"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Totem,				L"Clothes"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Phaser,				L"Javelin"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Dirt,					L"Puddle"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Attenuator,			L"MusicPlayer"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Divider,				L"Mail"),
				std::pair<int, std::wstring>((int)Communication::Proto::PropType::Null,					L"Null")
			};
			// 职业转换到宽字符串
			static const std::unordered_map<int, std::wstring> jobToWStr
			{
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job0, 	L"OrdinaryJob"),
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job1,	L"HappyMan"),
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job2, 	L"LazyGoat"),
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job3, 	L"PurpleFish"),
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job4, 	L"MonkeyDoctor"),
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job5,	L"EggMan"),
				std::pair<int, std::wstring>((int)Communication::Proto::JobType::Job6, 	L"PrincessIronFan")
			};
			// 道具转换到字符串
			static const std::unordered_map<int, std::_tstring> propToStr
			{
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Bike,				TEXT("SharedBike")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Amplifier,			TEXT("Horn")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::JinKeLa,				TEXT("SchoolBag")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Rice,				TEXT("HappyHotPot")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::NegativeFeedback,	TEXT("Shield")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Totem,				TEXT("Clothes")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Phaser,				TEXT("Javelin")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Dirt,				TEXT("Puddle")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Attenuator,			TEXT("MusicPlayer")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Divider,				TEXT("Mail")),
				std::pair<int, std::_tstring>((int)Communication::Proto::PropType::Null,				TEXT("Laid Mine"))
			};
			//道具转换到字符
			static const std::unordered_map<int, TCHAR> propToChar
			{
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Bike,				TEXT('B')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Amplifier,			TEXT('H')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::JinKeLa,				TEXT('S')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Rice,				TEXT('P')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::NegativeFeedback,	TEXT('D')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Totem,				TEXT('C')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Phaser,				TEXT('J')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Dirt,				TEXT('P')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Attenuator,			TEXT('U')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Divider,				TEXT('M')),
				std::pair<int, TCHAR>((int)Communication::Proto::PropType::Null,				TEXT(' '))
			};

			//显示人物信息

			HFONT hfInfo = CreateFont
			(
				basicSize / 40,
				0,
				0,
				0,
				FW_NORMAL,
				0,
				0,
				0,
				GB2312_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				TEXT("楷书")
			);
			HFONT hfOld = (HFONT)SelectObject(hdcMem, hfInfo);
			SetBkColor(hdcMem, RGB(0, 0, 0));
			SetTextColor(hdcMem, RGB(255, 255, 255));

			std::wostringstream wsout;

			wsout.imbue(std::locale("chs"));

			auto generatePlayerInfoStr = [&wsout](Communication::Proto::GameObjInfo^ hPlayer, uint64_t playerID)
			{
				if (hPlayer == nullptr) return;
				wsout << L"玩家" << playerID + 1 << L"：" << ((hPlayer->IsDying) ? L"离场" : L"在场") << L"\n生命：" << hPlayer->Hp << L"\n剩余子弹数：" << hPlayer->BulletNum << L"\n";
				wsout << L"移动速度：" << hPlayer->MoveSpeed << L"\n";
				wsout << L"攻击力：" << hPlayer->Ap << L"\n";
				wsout << L"道具类型：" << propToWStr.find((int)hPlayer->PropType)->second << L"\n职业：" << jobToWStr.find((int)hPlayer->JobType)->second << L"\n";
				/*wsout << L"拥有盾牌：" << (hPlayer->HasShield ? L"是" : L"否") << L"\n";
				wsout << L"拥有标枪：" << (hPlayer->HasSpear ? L"是" : L"否") << L"\n";
				wsout << L"拥有衣服：" << (hPlayer->HasTotem ? L"是" : L"否") << L"\n";*/
				wsout << '\n';
			};

			auto teamCount = recentMsg->PlayerGUIDs->Count;
			auto playerCountPerTeam = recentMsg->PlayerGUIDs[0]->TeammateGUIDs->Count;
			auto totalPlayerCount = teamCount * playerCountPerTeam;
			auto guidToInfo = gcnew System::Collections::Generic::Dictionary<uint64_t, Communication::Proto::GameObjInfo^>();
			for each (Communication::Proto::GameObjInfo ^ gameObj in recentMsg->GameObjs)
			{
				if (gameObj->GameObjType != Communication::Proto::GameObjType::Character) continue;
				guidToInfo->Add(gameObj->Guid, gameObj);
				if (guidToInfo->Count == totalPlayerCount) break;
			}

			int actualWidth = width - appendCx;
			int sigleTeamWidth = actualWidth / 3;

			for (int i = 0; i < teamCount; ++i)
			{
				wsout << L"队伍" << i + 1 << L"分数：" << teamScores[i] << L"\n\n";
				for (int j = 0; j < playerCountPerTeam; ++j)
				{
					generatePlayerInfoStr(guidToInfo[recentMsg->PlayerGUIDs[i]->TeammateGUIDs[j]], j);
				}

				
				RECT textRect = { actualWidth + 15 + sigleTeamWidth * i, 15, actualWidth + 15 + sigleTeamWidth * (i + 1), height };
				DrawTextW(hdcMem, wsout.str().c_str(), static_cast<int>(wsout.str().length()), &textRect, 0);

				wsout.clear();
				wsout.str(L"");
			}

			wsout << L"道具图例：\n";
			for (auto& p2c : propToChar)
			{
				wsout << p2c.second << ": " << propToStr.find(p2c.first)->second << '\n';
			}
			RECT textRect = { actualWidth + 15 + sigleTeamWidth * teamCount, 15, actualWidth + 15 + sigleTeamWidth * (teamCount + 1), height };
			DrawTextW(hdcMem, wsout.str().c_str(), static_cast<int>(wsout.str().length()), &textRect, 0);
			wsout.clear();
			wsout.str(L"");

			SelectObject(hdcMem, hfOld);
			DeleteObject(hfInfo);

			//绘制游戏对象

			HFONT hfUnpickedProp = CreateFont
			(
				basicSize / 60,
				0,
				0,
				0,
				FW_HEAVY,
				0,
				0,
				0,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				TEXT("Consolas")
			);

			SelectObject(hdcMem, hfUnpickedProp);

			COLORREF unpickedPropColor = RGB(234, 150, 122);
			SetBkColor(hdcMem, unpickedPropColor);
			SetTextColor(hdcMem, RGB(0, 0, 0));

			HBRUSH hbrPlayer[4][8] =
			{
				{ CreateSolidBrush(RGB(255, 0, 0)), CreateSolidBrush(RGB(255, 60, 0)), CreateSolidBrush(RGB(255, 120, 0)), CreateSolidBrush(RGB(255, 180, 0))},
				{ CreateSolidBrush(RGB(0, 255, 0)), CreateSolidBrush(RGB(0, 255, 60)), CreateSolidBrush(RGB(0, 255, 120)), CreateSolidBrush(RGB(0, 255, 180))},
				{ CreateSolidBrush(RGB(0, 0, 255)), CreateSolidBrush(RGB(60, 0, 255)), CreateSolidBrush(RGB(120, 0, 255)), CreateSolidBrush(RGB(180, 0, 255))},
				{ CreateSolidBrush(RGB(255, 255, 255)), CreateSolidBrush(RGB(205, 205, 205)), CreateSolidBrush(RGB(155, 155, 155)), CreateSolidBrush(RGB(105, 105, 105)), }
			};
			HBRUSH hbrWall = CreateSolidBrush(RGB(100, 100, 100));
			HBRUSH hbrBullet[4] = { CreateSolidBrush(RGB(240, 0, 0)), CreateSolidBrush(RGB(0, 240, 0)), CreateSolidBrush(RGB(0, 0, 240)), CreateSolidBrush(RGB(240, 240, 0)) };
			HBRUSH hbrPropUnpicked = CreateSolidBrush(unpickedPropColor);

			HPEN hPenPlayerDirect = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

			auto gameObjList = recentMsg->GameObjs;
			int rowAllGrid = 1000 * rows;
			int colAllGrid = 1000 * cols;

			std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>> guidToID;
			for (uint64_t i = 0ull; i < (uint64_t)teamCount; ++i)
			{
				for (uint64_t j = 0ull; j < (uint64_t)playerCountPerTeam; ++j)
				{
					guidToID.insert(std::make_pair(recentMsg->PlayerGUIDs[i]->TeammateGUIDs[j], std::make_pair(i, j)));
				}
			}

			for each (Communication::Proto::GameObjInfo ^ gameObj in gameObjList)
			{
				if (gameObj->GameObjType == Communication::Proto::GameObjType::Character)
				{
					if (!(gameObj->IsDying))
					{
						int rad = gameObj->Radius;
						auto x = gameObj->X;
						auto y = gameObj->Y;
						SelectObject(hdcMem, hbrPlayer[(int)(gameObj->TeamID)][(int)guidToID[gameObj->Guid].second]);
						Ellipse(hdcMem, (y - rad) * (width - appendCx) / colAllGrid, (x - rad) * (height - appendCy) / rowAllGrid, (y + rad) * (width - appendCx) / colAllGrid, (x + rad) * (height - appendCy) / rowAllGrid);
						int centerX = y * (width - appendCx) / colAllGrid, centerY = x * (height - appendCy) / rowAllGrid;
						double angle = gameObj->FacingDirection;
						int paintRadius = rad * (width - appendCx) / colAllGrid;
						MoveToEx(hdcMem, centerX, centerY, NULL);
						SelectObject(hdcMem, hPenPlayerDirect);
						LineTo(hdcMem, centerX + (int)(paintRadius * System::Math::Sin(angle)), centerY + (int)(paintRadius * System::Math::Cos(angle)));
						SelectObject(hdcMem, hPenNull);
					}
				}
				else
				{
					switch (gameObj->GameObjType)
					{
					case Communication::Proto::GameObjType::Bullet:
						SelectObject(hdcMem, hbrBullet[(int)gameObj->TeamID]);
						break;
					case Communication::Proto::GameObjType::Wall:
						SelectObject(hdcMem, hbrWall);
						break;
					case Communication::Proto::GameObjType::Prop:
						SelectObject(hdcMem, hbrPropUnpicked);
						break;
					case Communication::Proto::GameObjType::BirthPoint:
						goto notPaint;
						break;
					}
					{
						int rad = gameObj->Radius;
						int x = gameObj->X;
						int y = gameObj->Y;

						static_assert(std::is_same_v<decltype(Ellipse), decltype(Rectangle)>, "The type of the paint functions are not the same!");

						decltype(Ellipse)* PaintFunc = nullptr;

						switch (gameObj->ShapeType)
						{
						case Communication::Proto::ShapeType::Circle: PaintFunc = &Ellipse; break;
						case Communication::Proto::ShapeType::Square: PaintFunc = &Rectangle; break;
						}

#ifdef COMPATIBLE_WITH_ALL_OLD_PLAYBACK_VERSION
#pragma region 由于通信上有些问题，通信得到的物体全是圆形，为了兼容旧的回放文件，设计本段，在新的回放文件生成时应删去本段

						switch (gameObj->GameObjType)
						{
						case Communication::Proto::GameObjType::Bullet:
							PaintFunc = &Ellipse;
							break;
						case Communication::Proto::GameObjType::Wall:
							PaintFunc = &Rectangle;
							break;
						case Communication::Proto::GameObjType::Prop:
							PaintFunc = &Ellipse;
							break;
						case Communication::Proto::GameObjType::BirthPoint:
							goto notPaint;
							break;
						}

#pragma endregion
#endif // COMPATIBLE_WITH_ALL_OLD_PLAYBACK_VERSION

						RECT rect;
						rect.left = (y - rad) * (width - appendCx) / colAllGrid;
						rect.top = (x - rad) * (height - appendCy) / rowAllGrid;
						rect.right = (y + rad) * (width - appendCx) / colAllGrid;
						rect.bottom = (x + rad) * (height - appendCy) / rowAllGrid;
						PaintFunc(hdcMem, rect.left, rect.top, rect.right, rect.bottom);
						if (gameObj->GameObjType == Communication::Proto::GameObjType::Prop && gameObj->IsLaid == false)
						{
							TCHAR ch = propToChar.find((int)(gameObj->PropType))->second;
							DrawText(hdcMem, &ch, 1, &rect, DT_CENTER | DT_VCENTER);
						}
					}
				notPaint:;
				}
			}

			SelectObject(hdcMem, hfOld);
			DeleteObject(hfUnpickedProp);
			DeleteObject(hPenPlayerDirect);

			SelectObject(hdcMem, hbrOld);
			DeleteObject(hbrPropUnpicked);
			for (int i = 0; i < sizeof(hbrBullet) / sizeof(hbrBullet[0]); ++i)
			{
				DeleteObject(hbrBullet[i]);
			}
			DeleteObject(hbrWall);
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 8; ++j)
					if (hbrPlayer[i][j]) { DeleteObject(hbrPlayer[i][j]); }
			}
			
		}
		else
		{
			HDC hdcBmBkgnd = CreateCompatibleDC(hdc);
			HBITMAP hBmBkgndOld = (HBITMAP)SelectObject(hdcBmBkgnd, hBmBkGnd);
			double rate = bkGnd.bmWidth / bkGnd.bmHeight;
			double actualRate = (clrec.right - clrec.left) / (double)(clrec.bottom - clrec.top);
			if (rate < actualRate)
			{
				int paintHeight = (clrec.right - clrec.left) / rate;
				int paintYDest = (paintHeight - (clrec.bottom - clrec.top)) / 2;
				StretchBlt(hdcMem, 0, -paintYDest, clrec.right - clrec.left, paintHeight, hdcBmBkgnd, 0, 0, bkGnd.bmWidth, bkGnd.bmHeight, SRCCOPY);
			}
			else
			{
				int paintWidth = (clrec.bottom - clrec.top) * rate;
				int paintXDest = (paintWidth - (clrec.right - clrec.left)) / 2;
				StretchBlt(hdcMem, -paintXDest, 0, paintWidth, clrec.bottom - clrec.top, hdcBmBkgnd, 0, 0, bkGnd.bmWidth, bkGnd.bmHeight, SRCCOPY);
			}
			SelectObject(hdcBmBkgnd, hBmBkgndOld);
			DeleteDC(hdcBmBkgnd);
		}
	endPaint:
		BitBlt(hdc, 0, 0, clrec.right - clrec.left, clrec.bottom - clrec.top, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hBmMemOld);
		DeleteObject(hBmMem);
		DeleteDC(hdcMem);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
	{
		//KillTimer(hWnd, paintTimerID);
		PostQuitMessage(0);
		break;
	}
	default:
	defaultMsgProc:
		return false;
	}
	return true;
}

BOOL BasicModelessDialog::Init(HINSTANCE hInstance, LPCTSTR c_lpszTemplateName, HWND hWndParent)
{
	m_hInst = hInstance;
	//return (BOOL)DialogBoxParam(hInstance, c_lpszTemplateName, hWndParent, DlgProc, (LPARAM)this);
	m_hDlg = CreateDialogParam(hInstance, c_lpszTemplateName, hWndParent, DlgProc, (LPARAM)this);
	return m_hDlg ? TRUE : FALSE;
}

INT_PTR CALLBACK BasicModelessDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BasicModelessDialog* thisDlg = NULL;
	if (message == WM_INITDIALOG)
	{
		thisDlg = (BasicModelessDialog*)lParam;
		thisDlg->m_hDlg = hDlg;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)thisDlg);
	}
	thisDlg = (BasicModelessDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (thisDlg) return (INT_PTR)thisDlg->MessageProc(hDlg, message, wParam, lParam);
	return 0;
}

BOOL UI::ChooseFileDialog::Begin(HINSTANCE hInst, LPCTSTR hTempName, HWND hWndParent)
{
	return Init(hInst, hTempName, hWndParent);
}

bool UI::ChooseFileDialog::MessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case IDC_OK:
		{
			if (IsButtonCheck(IDC_FOUR)) timeInterval = 50 / 4;
			else if (IsButtonCheck(IDC_TWO)) timeInterval = 50 / 2;
			else if (IsButtonCheck(IDC_ONE)) timeInterval = 50;
			else if (IsButtonCheck(IDC_HALF)) timeInterval = 50 * 2;
			else if (IsButtonCheck(IDC_QUARTER)) timeInterval = 50 * 4;
			else MessageBox(m_hDlg, TEXT("Please choose at least one speed!"), TEXT(""), MB_OK | MB_ICONWARNING);
			return true;
		}
		}
		break;
	}
	}
	return false;
}
