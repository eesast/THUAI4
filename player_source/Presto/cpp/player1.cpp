#include "AI.h"
#include "Constants.h"
#include <thread>
#define pi 3.14159

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::PurpleFish; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * pi);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

static int x00, y00;//出生点坐标
static int calledcount = 0;//调用次数
static int myid;//我方teamID
static int enmid;//敌方teamID
static int indanger;//是否被瞄准
static int seekingprp;//是否正在寻找道具
static int beingchased;//是否遇见敌人
static int blocked;//是否卡住
static int a[9][9] = { 0 };

int distance(uint32_t x, uint32_t y, GameApi& g) {//坐标(x,y)与我间距
	return (int)sqrt(((int)x - (int)g.GetSelfInfo()->x) * ((int)x - (int)g.GetSelfInfo()->x) + ((int)y - (int)g.GetSelfInfo()->y) * ((int)y - (int)g.GetSelfInfo()->y));
}
std::string space(int k)
{
	std::string s;
	for (int i = 0; i < k; i++)
		s = s + " ";
	return s;
}
std::string infobox(int x, int y, bool isDying, int typeofmessage)
{
	std::string sx = std::to_string(x);
	std::string sy = std::to_string(y);
	std::string sisDying = std::to_string(isDying);
	std::string stypeofmessage = std::to_string(typeofmessage);
	std::string buffer = sx + space(8 - sx.size()) + sy + space(8 - sy.size()) + sisDying + space(1) + stypeofmessage + space(1);//注意长度限制
	return buffer;
}//写入信息，末尾的信息类型计划是用来辨别信息是关于敌还是友的，但若如此需再加一条信息，即队友的编号
struct MessageGroup
{
	int x;
	int y;
	bool isDying;
	int typeofmessage;
};
MessageGroup extractinfo(std::string buffer)
{
	char* cbuffer = new char[buffer.size()];
	cbuffer = buffer.data();
	int ox = atoi(&cbuffer[0]);
	int oy = atoi(&cbuffer[8]);
	bool oisDying = atoi(&cbuffer[16]);
	int otypeofmessage = atoi(&cbuffer[18]);
	MessageGroup r = { ox,oy,oisDying,otypeofmessage };
	return r;
}

void AI::play(GameApi& g)
{
	calledcount++;
	auto guid = g.GetPlayerGUIDs();

	if (calledcount == 1)
	{//初始化
		myid = g.GetSelfInfo()->teamID;
		enmid = 1 ^ myid;
		x00 = (int)g.GetSelfInfo()->x;
		y00 = (int)g.GetSelfInfo()->y;
		if (myid == 0)
		{
			g.MoveDown(200);
			g.Attack(1000, direction(e) / 4);
		}//出生点在地图上半部
		else {
			g.MoveUp(200);
			g.Attack(1000, pi / 2 + direction(e) / 4);
		}//出生地点在地图下半部
	}

	{special://特殊状态
		{//敌人检测与跑路
			auto oth = g.GetCharacters();
			for (int i = 0; i < oth.size(); i++)
			{
				if (oth[i]->teamID != myid&& oth[i]->isDying==0)
				{
					g.Send(2, infobox((int)oth[i]->x, (int)oth[i]->y, oth[i]->isDying, 2));
					g.Send(1, infobox((int)oth[i]->x, (int)oth[i]->y, oth[i]->isDying, 2));
					g.Send(3, infobox((int)oth[i]->x, (int)oth[i]->y, oth[i]->isDying, 2));
					//作为playerid=0
					g.MovePlayer(50, -atan2((int)oth[i]->y - (int)g.GetSelfInfo()->y, (int)oth[i]->x - (int)g.GetSelfInfo()->x));
					beingchased = 1;
				}
			}
			beingchased = 0;
		}
		if (beingchased) goto special;

		{//子弹检测与闪躲
			auto blt = g.GetBullets();
			for (int i = 0; i < blt.size(); i++) {
				double angle_blt = atan2((int64_t)g.GetSelfInfo()->y - (int64_t)blt[i]->y, (int64_t)g.GetSelfInfo()->x - (int64_t)blt[i]->x);//子弹对我方的角度
				if ((blt[i]->teamID != myid) && (abs(angle_blt - blt[i]->facingDirection) <= pi / 5)) {//有敌方子弹瞄准自己所在区域
					switch (rand() % 2)
					{
					case 0:
						g.MovePlayer(50, blt[i]->facingDirection + pi / 2 + direction(e) / 8);//direction(e)/8可视作一个微小扰动
						break;
					case 1:
						g.MovePlayer(50, blt[i]->facingDirection - pi / 2 + direction(e) / 8);
						break;
					default:
						break;
					}
					indanger = 1;
				}
			}
			indanger = 0;
		}
		if (indanger) goto special;

		{//道具拾取
			auto prp = g.GetProps();
			for (int i = 0; i < prp.size(); i++) {
				int dst = distance(prp[i]->x, prp[i]->y, g);
				if (dst < 400)
				{
					g.Pick(prp[i]->propType);
				}
				else
				{
					g.MovePlayer(50, atan2((int)prp[i]->y - (int)g.GetSelfInfo()->y, (int)prp[i]->x - (int)g.GetSelfInfo()->x));
					seekingprp = 1;
				}
				if (g.GetSelfInfo()->propType != (THUAI4::PropType)0)
				{
					g.Use();
				}
			}
			seekingprp = 0;
		}
		if (seekingprp) goto special;
	}

	{//非特殊情况

		if (g.GetSelfInfo()->bulletNum != 0)
		{
			double angle = 0;
			if (g.GetCellColor((int)g.GetSelfInfo()->x / 1000, (int)g.GetSelfInfo()->y / 1000) != g.GetSelfTeamColor())
			{
				if ((int)g.GetSelfInfo()->x <= 25000 && (int)g.GetSelfInfo()->y <= 25000) angle = rand() % 4 * pi / 8;
				else if ((int)g.GetSelfInfo()->x > 25000 && (int)g.GetSelfInfo()->y <= 25000) angle = pi / 2 + rand() % 4 * pi / 8;
				else if ((int)g.GetSelfInfo()->x > 25000 && (int)g.GetSelfInfo()->y > 25000) angle = pi + rand() % 4 * pi / 8;
				else angle = 3 * pi / 2 + rand() % 4 * pi / 8;
				g.Attack(1000, angle);//需要完善的地方：如何确定angle
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				g.MovePlayer(150, angle);//沿着打子弹的方向走
			}
			else
			{
				int xn = (int)g.GetSelfInfo()->x / 1000, yn = (int)g.GetSelfInfo()->y / 1000;
				int i1 = 0, j1 = 0;
				for (int i = -4; i <= 4; i++)
				{
					for (int j = -4; j <= 4; j++)
					{
						int flag = 0;
						if (!(xn + i > 0 && xn + i < 49 && yn + j > 0 && yn + j < 49)) continue;//在地图外getcellcolor()是会出错的！！
						if (g.GetCellColor(xn + i, yn + j) != g.GetSelfTeamColor() && g.GetCellColor(xn + i, yn + j) != THUAI4::ColorType::Invisible)
						{
							a[i + 4][j + 4] = 1;
							if (i * i + j * j >= i1 * i1 + j1 * j1)
							{
								auto nowwalls = g.GetWalls();
								for (int k = 0; k < nowwalls.size(); k++)
								{
									if ((((int)g.GetSelfInfo()->x <= (int)nowwalls[k]->x&& (int)nowwalls[k]->x <= (xn + i + 1) * 1000) || ((int)g.GetSelfInfo()->x >= (int)nowwalls[k]->x&& (int)nowwalls[k]->x >= (xn + i - 1) * 1000)) && abs(atan2(j, i) - atan2((int)nowwalls[k]->y - (int)g.GetSelfInfo()->y, (int)nowwalls[k]->x - (int)g.GetSelfInfo()->x)) <= pi / 8)
									{
										flag = 1;
										break;
									}
								}
								if (!flag)  i1 = i, j1 = j;
							}
						}
						else
						{
							a[i + 4][j + 4] = 0;
						}
					}
				}//寻找视野内最远的非自己颜色的格子,且确认击打过程无障碍物
				if (i1 != 0 && j1 != 0)
				{
					angle = atan2(j1, i1);
					g.Attack(100, angle);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					g.MovePlayer(150, angle);//沿着打子弹的方向走
				}
				else;
			}
		}

		if (g.GetSelfInfo()->bulletNum == 0)
		{
			int xn = (int)g.GetSelfInfo()->x / 1000, yn = (int)g.GetSelfInfo()->y / 1000;
			if (g.GetCellColor(xn, yn) == g.GetSelfTeamColor()) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			else
			{
				int i1 = 5, j1 = 5;
				for (int i = -4; i <= 4; i++)
				{
					for (int j = -4; j <= 4; j++)
					{
						if (!(xn + i >= 0 && xn + i < 50 && yn + j >= 0 && yn + j < 50)) continue;
						if (g.GetCellColor(xn + i, yn + j) == g.GetSelfTeamColor())
						{
							a[i + 4][j + 4] = 1;
							if (i * i + j * j <= i1 * i1 + j1 * j1)
							{
								i1 = i, j1 = j;
							}
						}
						else
						{
							a[i + 4][j + 4] = 0;
						}
					}
				}//再一次更新地图颜色信息,顺便寻找距离最近的自己颜色的格子
				if (i1 != 5 && j1 != 5)
				{
					double angle = atan2(j1, i1);
					g.MovePlayer(100, angle);
				}
				else g.MovePlayer(100, rand() % 8 * pi / 4);//不然就随机走
			}
		}//没子弹时到自己的区域恢复子弹，视野内找不到就随机走着找
		else//有子弹时乱走
		{
			double angle;
			if ((int)g.GetSelfInfo()->x <= 25000 && (int)g.GetSelfInfo()->y <= 25000) angle = rand() % 4 * pi / 8;
			else if ((int)g.GetSelfInfo()->x > 25000 && (int)g.GetSelfInfo()->y <= 25000) angle = pi / 2 + rand() % 4 * pi / 8;
			else if ((int)g.GetSelfInfo()->x > 25000 && (int)g.GetSelfInfo()->y > 25000) angle = pi + rand() % 4 * pi / 8;
			else angle = 3 * pi / 2 + rand() % 4 * pi / 8;
			g.MovePlayer(500, angle);
		}

		{
			g.MovePlayer(250, rand() % 8 * pi / 4);
		}

		int xnow = (int)g.GetSelfInfo()->x;
		int ynow = (int)g.GetSelfInfo()->y;//用xnow,ynow记录先前位置，比较确定是否已走出blocked状态

		if (!g.GetSelfInfo()->bulletNum)
			if (g.GetCellColor(xnow / 1000, ynow / 1000) == g.GetSelfTeamColor()) g.Wait();
		//g.wait()函数:进行50ms的休眠，减少占用

		if (distance(xnow, ynow, g) <= 100) blocked += 1;
		else blocked = 0;
		while (blocked) {
			xnow = (int)g.GetSelfInfo()->x;
			ynow = (int)g.GetSelfInfo()->y;
			g.MovePlayer(500, g.GetSelfInfo()->facingDirection + (double)blocked * pi / 4);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			if (distance(xnow, ynow, g) == 0) blocked += 1;
			else blocked = 0;
		}//走出blocked状态
	}


	if (g.GetCounterOfFrames() == 200) //200帧时休眠
	{
		g.Wait();
	}
}