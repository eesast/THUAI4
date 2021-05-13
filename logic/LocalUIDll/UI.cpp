#include "UI.h"
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <type_traits>

UI::GameWrapper::GameWrapper(array<unsigned int, 2>^ mapResource, int numOfTeam)
{
	game = gcnew Gaming::Game(mapResource, numOfTeam);
}

void UI::GetPGM(array<unsigned int, 2>^ mapResource, int numOfTeam)
{
	if (pGM != nullptr) return;
	GameWrapper gm(mapResource, numOfTeam);
	pGM = &gm;
	while (true) { std::this_thread::sleep_for(std::chrono::seconds(10)); }
	pGM = nullptr;
}

int UI::Begin()
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
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("THUnityWindow");
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	width = GetSystemMetrics(SM_CXMIN);
	height = GetSystemMetrics(SM_CYMIN);

	Init(GetModuleHandle(NULL), SW_NORMAL, 0, 0, width, height,
		 WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
		 TEXT("THUnity"), wcex, GetConsoleWindow());

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

bool UI::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		std::thread([this] { GetPGM(Gaming::MapInfo::defaultMap, 2); }).detach();

		while (pGM == nullptr) std::this_thread::sleep_for(std::chrono::milliseconds(1));
		Gaming::Game^ game = const_cast<GameWrapper*>(pGM)->Game();

		int basicSize = 600;
		int rows = game->GameMap->Rows;
		int cols = game->GameMap->Cols;

		width = (pixelPerCell.x = basicSize / cols) * cols;
		width += appendCx = width / 2;
		height = (pixelPerCell.y = basicSize / rows) * rows + (appendCy = GetSystemMetrics(SM_CYMIN));

		int job1 = 0, job2 = 1;
		std::ifstream fin("job.txt");
		if (fin)
		{
			fin >> job1 >> job2;
			fin.close();

			if (job1 < 0 || job1 >= 7) job1 = 0;
			if (job2 < 0 || job2 >= 7) job2 = 0;
		}

		player1ID = game->AddPlayer(Gaming::Game::PlayerInitInfo(0u, THUnity2D::ObjClasses::JobType(job1), 0));
		player2ID = game->AddPlayer(Gaming::Game::PlayerInitInfo(1u, THUnity2D::ObjClasses::JobType(job2), 1));
		
		MoveWindow(hWnd, 0, 0, width + 15, height, FALSE);

		HDC hdc = GetDC(hWnd);
		hdcMem = CreateCompatibleDC(hdc);
		hBmMem = CreateCompatibleBitmap(hdc, width, height);
		ReleaseDC(hWnd, hdc);

		SetTimer(hWnd, paintTimerID, 50, NULL);

		std::thread([hWnd, this]()
			{
				Gaming::Game^ game = const_cast<GameWrapper*>(pGM)->Game();

				game->StartGame(1000 * 60 * 10);
				
				int score1 = game->GetTeamScore(0LL);
				int score2 = game->GetTeamScore(1LL);
				std::_tostringstream sout;
				if constexpr (!std::is_same_v<decltype(sout), std::ostringstream>) sout.imbue(std::locale("chs"));
				sout << TEXT("游戏结束！队伍一分数：") << score1 << TEXT("；队伍二分数：") << score2;
				MessageBox(hWnd, sout.str().c_str(), TEXT("游戏结束！"), MB_OK);
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}).detach();

		auto UsrControl = [this](long long playerID, int up, int left, int down, int right, int atk, int pick, int use, int throwprop)
		{
			double direct[16] = { 0 };
			int time[16] = { 0 };

			const int WKey = 0x1;
			const int AKey = 0x2;
			const int SKey = 0x4;
			const int DKey = 0x8;

			for (int i = 1; i < sizeof(time) / (sizeof(decltype(time[0]))); ++i)
			{
				time[i] = 50;
			}

			direct[WKey] = System::Math::PI;
			direct[AKey] = -System::Math::PI * 0.5;
			direct[SKey] = 0.0;
			direct[DKey] = System::Math::PI * 0.5;
			////direct[DKey] = System::Math::PI * 0.5 + System::Math::PI / 6.0;
			direct[WKey | AKey] = System::Math::PI * 0.25 * 5;
			direct[WKey | DKey] = System::Math::PI * 0.25 * 3;
			direct[SKey | AKey] = System::Math::PI * 0.25 * 7;
			direct[SKey | DKey] = System::Math::PI * 0.25;

			Gaming::Game^ game = const_cast<GameWrapper*>(pGM)->Game();

			while (true)
			{

				std::this_thread::sleep_for(std::chrono::milliseconds(1));

				int key = 0;
				bool WPress = GetKeyState(up) < 0,
					APress = GetKeyState(left) < 0,
					SPress = GetKeyState(down) < 0,
					DPress = GetKeyState(right) < 0;
				if (WPress) key |= WKey;
				if (APress) key |= AKey;
				if (SPress) key |= SKey;
				if (DPress) key |= DKey;

				bool JPress = GetKeyState(atk) < 0;
				bool PPress = GetKeyState(pick) < 0;
				bool UPress = GetKeyState(use) < 0;
				bool TPress = GetKeyState(throwprop) < 0;

				if (PPress)
				{
					for (int i = THUnity2D::ObjClasses::Prop::MinPropTypeNum; i <= THUnity2D::ObjClasses::Prop::MaxPropTypeNum; ++i)
					{
						if (game->Pick(playerID, static_cast<THUnity2D::ObjClasses::PropType>(i))) break;
					}
				}
				else if (UPress)
				{
					game->Use(playerID);
				}
				else if (JPress)
				{
					if (key && 
						game->Attack(playerID, time[key] * 20, direct[key])) std::this_thread::sleep_for(std::chrono::milliseconds(300));
				}
				else if (TPress)
				{
					if (key) game->Throw(playerID, time[key] * 50, direct[key]);
				}
				else if (key)
				{
					game->MovePlayer(playerID, time[key], direct[key]);
				}
			}
		};

		std::thread(UsrControl, player1ID, 'W', 'A', 'S', 'D', 'J', 'P', 'U', 'T').detach();
		std::thread(UsrControl, player2ID, VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT, VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3).detach();

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (pGM != nullptr)
		{

			Gaming::Game^ game = const_cast<GameWrapper*>(pGM)->Game();

			HBITMAP hBmOld = (HBITMAP)SelectObject(hdcMem, hBmMem);
			HPEN hPenNull = (HPEN)GetStockObject(PS_NULL);
			SelectObject(hdcMem, hPenNull);
			HBRUSH hbrBkGnd = CreateSolidBrush(RGB(0, 0, 0));
			HBRUSH hbrOld = (HBRUSH)SelectObject(hdcMem, hbrBkGnd);
			Rectangle(hdcMem, 0, 0, width, height);     //绘制背景

			//绘制地图颜色

			HBRUSH hbrColor1 = CreateSolidBrush(RGB(100, 0, 0));
			HBRUSH hbrColor2 = CreateSolidBrush(RGB(0, 100, 0));
			HBRUSH hbrColor3 = CreateSolidBrush(RGB(0, 0, 100));
			HBRUSH hbrColor4 = CreateSolidBrush(RGB(100, 100, 0));

			auto cellColor = game->GameMap->CellColor;
			int rows = game->GameMap->Rows, cols = game->GameMap->Cols;
			for (int i = 0, topPos = 0; i < rows; ++i, topPos += pixelPerCell.y)
			{
				for (int j = 0, leftPos = 0; j < cols; ++j, leftPos += pixelPerCell.x)
				{
					switch (cellColor[i, j])
					{
					case Gaming::Map::ColorType::Color1: SelectObject(hdcMem, hbrColor1); break;
					case Gaming::Map::ColorType::Color2: SelectObject(hdcMem, hbrColor2); break;
					case Gaming::Map::ColorType::Color3: SelectObject(hdcMem, hbrColor3); break;
					case Gaming::Map::ColorType::Color4: SelectObject(hdcMem, hbrColor4); break;
					default: continue;
					}
					Rectangle(hdcMem, leftPos, topPos, leftPos + pixelPerCell.x, topPos + pixelPerCell.y);
				}
			}

			DeleteObject(hbrColor1);
			DeleteObject(hbrColor2);
			DeleteObject(hbrColor3);
			DeleteObject(hbrColor4);

			//显示人物信息
			
			HFONT hfInfo = CreateFont
			(
				15,
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

			auto generatePlayerInfoStr = [&wsout](THUnity2D::ObjClasses::Character^ hPlayer, long long id)
			{
				wsout << L"玩家" << std::to_wstring(id) << L"：\n生命：" << hPlayer->HP << L"\n剩余子弹数：" << hPlayer->BulletNum << L"\n分数：" << hPlayer->Score << L"\n";
				wsout << L"移动速度：" << hPlayer->MoveSpeed << L"\n";
				wsout << L"攻击力：" << hPlayer->AP << L"\n";
				wsout << L"拥有盾牌：" << (hPlayer->HasShield ? L"是" : L"否") << L"\n";
				wsout << L"拥有标枪：" << (hPlayer->HasSpear ? L"是" : L"否") << L"\n";
				wsout << L"拥有衣服：" << (hPlayer->HasTotem ? L"是" : L"否") << L"\n";
				wsout << '\n';
			};

			wsout << L"队伍1：分数：" << game->GetTeamScore(0) << '\n';
			auto hPlayer1 = game->GetPlayerFromTeam(player1ID), hPlayer2 = game->GetPlayerFromTeam(player2ID);
			generatePlayerInfoStr(hPlayer1, 1);

			wsout << L"队伍2：分数：" << game->GetTeamScore(1) << '\n';
			generatePlayerInfoStr(hPlayer2, 2);

			DrawTextW(hdcMem, wsout.str().c_str(), static_cast<int>(wsout.str().length()), &RECT({ width - appendCx + 20, 20, width, height }), 0);

			SelectObject(hdcMem, hfOld);
			DeleteObject(hfInfo);

			//绘制游戏对象

			//道具转换到字符
			static const std::unordered_map<int, TCHAR> propToChar
			{
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Bike, TEXT('B')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Amplifier, TEXT('A')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::JinKeLa, TEXT('J')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Rice, TEXT('R')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Shield, TEXT('N')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Totem, TEXT('T')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Phaser, TEXT('P')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Dirt, TEXT('D')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Attenuator, TEXT('S')),
				std::pair<int, TCHAR>((int)THUnity2D::ObjClasses::PropType::Divider, TEXT('F'))
			};

			HFONT hfUnpickedProp = CreateFont
			(
				10,
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

			COLORREF unpickedPropColor = RGB(234, 150, 122);
			SelectObject(hdcMem, hfUnpickedProp);
			SetBkColor(hdcMem, RGB(255, 0, 0));
			SetTextColor(hdcMem, RGB(0, 0, 0));

			HBRUSH hbrPlayer = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH hbrWall = CreateSolidBrush(RGB(100, 100, 100));
			HBRUSH hbrBullet = CreateSolidBrush(RGB(255, 215, 0));
			HBRUSH hbrPropUnpicked = CreateSolidBrush(unpickedPropColor);

			HPEN hPenPlayerDirect = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

			auto gameObjList = game->GetGameObject();
			int rowAllGrid = THUnity2D::Constant::numOfGridPerCell * game->GameMap->Rows;
			int colAllGrid = THUnity2D::Constant::numOfGridPerCell * game->GameMap->Cols;
			for each (THUnity2D::ObjClasses::GameObject^ gameObj in gameObjList)
			{
				if (gameObj->GetGameObjType() == THUnity2D::ObjClasses::GameObjType::Character)
				{
					if (!static_cast<THUnity2D::ObjClasses::Character^>(gameObj)->IsResetting)
					{
						int rad = gameObj->Radius;
						auto [x, y] = gameObj->Position;
						SelectObject(hdcMem, hbrPlayer);
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
					THUnity2D::ObjClasses::Obj^ obj = (THUnity2D::ObjClasses::Obj^)gameObj;
					switch (obj->ObjType)
					{
					case THUnity2D::ObjClasses::ObjType::Bullet:
						SelectObject(hdcMem, hbrBullet);
						break;
					case THUnity2D::ObjClasses::ObjType::Wall:
						SelectObject(hdcMem, hbrWall);
						break;
					case THUnity2D::ObjClasses::ObjType::Prop:
						SelectObject(hdcMem, hbrPropUnpicked);
						break;
					case THUnity2D::ObjClasses::ObjType::BirthPoint:
						goto notPaint;
						break;
					}
					{
						int rad = obj->Radius;
						auto [x, y] = obj->Position;

						static_assert(std::is_same_v<decltype(Ellipse), decltype(Rectangle)>, "The type of the paint functions are not the same!");

						decltype(Ellipse)* PaintFunc = nullptr;

						switch (obj->Shape)
						{
						case THUnity2D::Interfaces::ShapeType::Circle: PaintFunc = &Ellipse; break;
						case THUnity2D::Interfaces::ShapeType::Square: PaintFunc = &Rectangle; break;
						}
						RECT rect;
						rect.left = (y - rad) * (width - appendCx) / colAllGrid;
						rect.top = (x - rad) * (height - appendCy) / rowAllGrid;
						rect.right = (y + rad) * (width - appendCx) / colAllGrid;
						rect.bottom = (x + rad) * (height - appendCy) / rowAllGrid;
						PaintFunc(hdcMem, rect.left, rect.top, rect.right, rect.bottom);
						if (obj->ObjType == THUnity2D::ObjClasses::ObjType::Prop && ((THUnity2D::ObjClasses::Prop^)obj)->Laid == false)
						{
							THUnity2D::ObjClasses::Prop^ prop = (THUnity2D::ObjClasses::Prop^)obj;
							TCHAR ch = propToChar.find((int)(prop->GetPropType()))->second;
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
			DeleteObject(hbrBullet);
			DeleteObject(hbrWall);
			DeleteObject(hbrPlayer);
			DeleteObject(hbrBkGnd);
			BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, hBmOld);
		}
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_TIMER:
	{
		if (wParam == paintTimerID)
		{
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else goto defaultMsgProc;
		break;
	}
	case WM_DESTROY:
	{
		DeleteObject(hBmMem);
		DeleteDC(hdcMem);
		KillTimer(hWnd, paintTimerID);
		PostQuitMessage(0);
		break;
	}
	default:
	defaultMsgProc:
		return false;
	}
	return true;
}
