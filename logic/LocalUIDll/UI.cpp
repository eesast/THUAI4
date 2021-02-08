#include "UI.h"
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>

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
        mp.map = gcnew THUnity2D::Map(THUnity2D::MapInfo::map, 2);
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
        width += appendCx = width / 2;
        height = (pixelPerCell.y = basicSize / rows) * rows + (appendCy = GetSystemMetrics(SM_CYMIN));

        player1ID = pMW->map->AddPlayer(THUnity2D::Map::PlayerInitInfo(0u, THUnity2D::JobType::job0, 0));
        player2ID = pMW->map->AddPlayer(THUnity2D::Map::PlayerInitInfo(1u, THUnity2D::JobType::job0, 1));
        
        MoveWindow(hWnd, 0, 0, width + 15, height, FALSE);

        HDC hdc = GetDC(hWnd);
        hdcMem = CreateCompatibleDC(hdc);
        hBmMem = CreateCompatibleBitmap(hdc, width, height);
        ReleaseDC(hWnd, hdc);

        SetTimer(hWnd, paintTimerID, 50, NULL);

        std::thread([]() {pMW->map->StartGame(1000 * 60 * 10); }).detach();

        auto UsrControl = [this](long long playerID, int up, int left, int down, int right, int atk, int pick, int use)
        {
            double direct[16] = { 0 };
            int time[16] = { 0 };

            const int WKey = 0x1;
            const int AKey = 0x2;
            const int SKey = 0x4;
            const int DKey = 0x8;

            for (int i = 1; i < sizeof(time) / (sizeof(decltype(time[0]))); ++i)
            {
                time[i] = 20;
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

                if (PPress)
                {
                    for (int i = THUnity2D::Prop::MinPropTypeNum; i <= THUnity2D::Prop::MaxPropTypeNum; ++i)
                    {
                        if (pMW->map->Pick(playerID, static_cast<THUnity2D::PropType>(i), false)) break;
                    }
                }
                else if (UPress)
                {
                    pMW->map->Use(playerID);
                }
                else if (JPress)
                {
                    if (key && 
                        pMW->map->Attack(playerID, time[key] * 50, direct[key])) std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
                else if (key)
                {
                    pMW->map->MovePlayer(playerID, time[key], direct[key]);
                }
            }
        };

        std::thread(UsrControl, player1ID, 'W', 'A', 'S', 'D', 'J', 'P', 'U').detach();
        std::thread(UsrControl, player2ID, VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT, VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2).detach();

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (finishGcNew)
        {
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

            auto cellColor = pMW->map->CellColor;
            int rows = pMW->map->Rows, cols = pMW->map->Cols;
            for (int i = 0, topPos = 0; i < rows; ++i, topPos += pixelPerCell.y)
            {
                for (int j = 0, leftPos = 0; j < cols; ++j, leftPos += pixelPerCell.x)
                {
                    switch (cellColor[i, j])
                    {
                    case THUnity2D::Map::ColorType::Color1: SelectObject(hdcMem, hbrColor1); break;
                    case THUnity2D::Map::ColorType::Color2: SelectObject(hdcMem, hbrColor2); break;
                    case THUnity2D::Map::ColorType::Color3: SelectObject(hdcMem, hbrColor3); break;
                    case THUnity2D::Map::ColorType::Color4: SelectObject(hdcMem, hbrColor4); break;
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
            wsout << L"队伍1：分数：" << pMW->map->GetTeamScore(0) << '\n';
            auto hPlayer1 = pMW->map->GetPlayerFromTeam(player1ID), hPlayer2 = pMW->map->GetPlayerFromTeam(player2ID);
            wsout << L"玩家1：\n生命：" << hPlayer1->HP << L"\n剩余子弹数：" << hPlayer1->BulletNum << L"\n分数：" << hPlayer1->Score << L"\n";
            wsout << L"移动速度：" << hPlayer1->MoveSpeed << "\n";
            wsout << L"攻击力：" << hPlayer1->AP << "\n";
            wsout << '\n';

            wsout << L"队伍2：分数：" << pMW->map->GetTeamScore(1) << '\n';
            wsout << L"玩家2：\n生命：" << hPlayer2->HP << L"\n剩余子弹数：" << hPlayer2->BulletNum << L"\n分数：" << hPlayer2->HP << L"\n";
            wsout << L"移动速度：" << hPlayer2->MoveSpeed << "\n";
            wsout << L"攻击力：" << hPlayer2->AP << "\n";
            wsout << '\n';

            DrawTextW(hdcMem, wsout.str().c_str(), static_cast<int>(wsout.str().length()), &RECT({ width - appendCx + 20, 20, width, height }), 0);

            SelectObject(hdcMem, hfOld);
            DeleteObject(hfInfo);

            //绘制游戏对象

            //道具转换到字符
            static const std::unordered_map<int, TCHAR> propToChar
            {
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Bike, TEXT('B')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Amplifier, TEXT('A')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::JinKeLa, TEXT('J')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Rice, TEXT('R')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Shield, TEXT('N')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Totem, TEXT('T')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Phaser, TEXT('P')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Dirt, TEXT('D')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Attenuator, TEXT('S')),
                std::pair<int, TCHAR>((int)THUnity2D::PropType::Divider, TEXT('F'))
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

            auto gameObjList = pMW->map->GetGameObject();
            int rowAllGrid = THUnity2D::Map::Constant::numOfGridPerCell * pMW->map->Rows;
            int colAllGrid = THUnity2D::Map::Constant::numOfGridPerCell * pMW->map->Cols;
            for each (THUnity2D::GameObject^ gameObj in gameObjList)
            {
                if (gameObj->GetGameObjType() == THUnity2D::GameObject::GameObjType::Character)
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
                else
                {
                    THUnity2D::Obj^ obj = (THUnity2D::Obj^)gameObj;
                    switch (obj->objType)
                    {
                    case THUnity2D::ObjType::Bullet:
                        SelectObject(hdcMem, hbrBullet);
                        break;
                    case THUnity2D::ObjType::Wall:
                        SelectObject(hdcMem, hbrWall);
                        break;
                    case THUnity2D::ObjType::Prop:
                        SelectObject(hdcMem, hbrPropUnpicked);
                        break;
                    case THUnity2D::ObjType::BirthPoint:
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
                        case THUnity2D::GameObject::ShapeType::Circle: PaintFunc = &Ellipse; break;
                        case THUnity2D::GameObject::ShapeType::Sqare: PaintFunc = &Rectangle; break;
                        }
                        RECT rect;
                        rect.left = (y - rad) * (width - appendCx) / colAllGrid;
                        rect.top = (x - rad) * (height - appendCy) / rowAllGrid;
                        rect.right = (y + rad) * (width - appendCx) / colAllGrid;
                        rect.bottom = (x + rad) * (height - appendCy) / rowAllGrid;
                        PaintFunc(hdcMem, rect.left, rect.top, rect.right, rect.bottom);
                        if (obj->objType == THUnity2D::ObjType::Prop && ((THUnity2D::Prop^)obj)->Laid == false)
                        {
                            THUnity2D::Prop^ prop = (THUnity2D::Prop^)obj;
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
