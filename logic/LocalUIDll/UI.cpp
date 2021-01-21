#include "UI.h"
#include <thread>
#include <atomic>

namespace
{
    value class MapWrapper
    {
    public:
        THUnity2D::Map^ map;
    };

    MapWrapper* pMW = nullptr;
    std::atomic<bool> finishGcNew = false;

    void GetPMW()
    {
        MapWrapper mp;
        mp.map = gcnew THUnity2D::Map(THUnity2D::Mapinfo::map, 1);
        pMW = &mp;
        finishGcNew = true;
        while (true) { std::this_thread::sleep_for(std::chrono::seconds(10)); }
    }
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
         TEXT("THUnity"), wcex);

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
        std::thread thrPMW
        (
            []()
            {
                GetPMW();
            }
        );
        
        thrPMW.detach();

        while (!finishGcNew);

        int basicSize = 600;
        int rows = pMW->map->Rows;
        int cols = pMW->map->Cols;

        width = (pixelPerCell.x = basicSize / cols) * cols;
        height = (pixelPerCell.y = basicSize / rows) * rows + (appendCy = GetSystemMetrics(SM_CYMIN));

        //Debug***********************
        //System::Console::WriteLine("width: {0}; height: {1}", width, height);

        playerID = pMW->map->AddPlayer(THUnity2D::Map::PlayerInitInfo(THUnity2D::XYPosition(2500, 2500), THUnity2D::JobType::job0, 0));
        
        MoveWindow(hWnd, 0, 0, width + 15, height, FALSE);

        HDC hdc = GetDC(hWnd);
        hdcMem = CreateCompatibleDC(hdc);
        hBmMem = CreateCompatibleBitmap(hdc, width, height);
        ReleaseDC(hWnd, hdc);

        SetTimer(hWnd, paintTimerID, 50, NULL);

        std::thread thrGame([]() {pMW->map->StartGame(1000 * 60 * 10); });
        thrGame.detach();

		std::thread thrCheckKey
		(
			[this]()
			{

				double direct[16] = { 0 };
				int time[16] = { 0 };

				const int WKey = 0x1;
				const int AKey = 0x2;
				const int SKey = 0x4;
				const int DKey = 0x8;

				for (int i = 1; i < sizeof(time) / (sizeof(decltype(time[0]))); ++i)
				{
					time[i] = 1000;
				}

				direct[WKey] = System::Math::PI;
				direct[AKey] = -System::Math::PI * 0.5;
				direct[SKey] = 0.0;
				direct[DKey] = System::Math::PI * 0.5;
				direct[WKey | AKey] = System::Math::PI * 0.25 * 5;
				direct[WKey | DKey] = System::Math::PI * 0.25 * 3;
				direct[SKey | AKey] = System::Math::PI * 0.25 * 7;
				direct[SKey | DKey] = System::Math::PI * 0.25;

				while (true)
				{

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

					int key = 0;
					bool WPress = GetKeyState('W') < 0,
						APress = GetKeyState('A') < 0,
						SPress = GetKeyState('S') < 0,
						DPress = GetKeyState('D') < 0;
					if (WPress) key |= WKey;
					if (APress) key |= AKey;
					if (SPress) key |= SKey;
					if (DPress) key |= DKey;

					bool JPress = GetKeyState('J') < 0;

					if (JPress)
					{
						if (key) pMW->map->Attack(playerID, time[key] * 1000, direct[key]);
					}
					else if (key)
					{
						pMW->map->MovePlayer(playerID, time[key], direct[key]);
					}
				}
			});
        thrCheckKey.detach();

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (finishGcNew)
        {
            HBITMAP hBmOld = (HBITMAP)SelectObject(hdcMem, hBmMem);
            HPEN hPen = (HPEN)GetStockObject(PS_NULL);
            SelectObject(hdcMem, hPen);
            HBRUSH hbrBkGnd = CreateSolidBrush(RGB(0, 0, 0));
            HBRUSH hbrOld = (HBRUSH)SelectObject(hdcMem, hbrBkGnd);
            Rectangle(hdcMem, 0, 0, width, height);

            HBRUSH hbrPlayer = CreateSolidBrush(RGB(255, 0, 0));
            HBRUSH hbrWall = CreateSolidBrush(RGB(100, 100, 100));
            HBRUSH hbrBullet = CreateSolidBrush(RGB(0, 255, 0));

            auto gameObjList = pMW->map->GetGameObject();
            int rowAllGrid = pMW->map->numOfGridPerCell * pMW->map->Rows;
            int colAllGrid = pMW->map->numOfGridPerCell * pMW->map->Cols;
            for each (THUnity2D::GameObject^ gameObj in gameObjList)
            {
                if (gameObj->GetGameObjType() == THUnity2D::GameObject::GameObjType::character)
                {
                    int rad = gameObj->Radius;
                    auto [x, y] = gameObj->Position;
                    SelectObject(hdcMem, hbrPlayer);
                    Ellipse(hdcMem, (y - rad) * width / colAllGrid, (x - rad) * (height - appendCy) / rowAllGrid, (y + rad) * width / colAllGrid, (x + rad) * (height - appendCy) / rowAllGrid);
                }
                else
                {
                    int rad = gameObj->Radius;
                    auto [x, y] = gameObj->Position;

                    THUnity2D::Obj^ obj = (THUnity2D::Obj^)gameObj;
                    switch (obj->objType)
                    {
                    case THUnity2D::ObjType::bullet:
                        SelectObject(hdcMem, hbrBullet);
                        break;
                    case THUnity2D::ObjType::wall:
                        SelectObject(hdcMem, hbrWall);
                        break;
                    }

                    Ellipse(hdcMem, (y - rad) * width / colAllGrid, (x - rad) * (height - appendCy) / rowAllGrid, (y + rad) * width / colAllGrid, (x + rad) * (height - appendCy) / rowAllGrid);
                }
            }

            SelectObject(hdcMem, hbrOld);
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
