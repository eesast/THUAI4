#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>
#include <vector>  
#include <stack>
#include <cmath>
#include <algorithm>
#include <chrono>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

//程序开始的时候记录时间，ms1是为了记录捡道具的时间
extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
static auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
static auto ms1 = ms;
static auto ms2 = ms;
static auto ms3 = ms;
static auto ms4 = ms;
static auto ms5 = ms;

static int flag = 0;
static int state = 0;
static THUAI4::ColorType selfteamcolor;
const static int movedis = 200;
const static uint32_t at = 200;
const static uint32_t at1 = 83;//攻击一格
static uint32_t x11, x22;
static uint32_t y11, y22;
static auto ms6 = ms;
static auto ms7 = ms;
static auto ms0 = ms;

static double pi = 3.1415;
//目前出生点在左上方的已经写好了，在左下方的准备写；
/*void ready(GameApi& g, int state)
{
	if (state == 1)
	{
		std::cout << 1 << std::endl;
		if ((int)g.GetSelfInfo()->y / 1000 != 13)
		{
			g.MoveRight(movedis);
			std::cout << 1 << std::endl;
		}
		else if ((int)g.GetSelfInfo()->y / 1000 == 13 && (int)g.GetSelfInfo()->x / 1000 != 43)
			g.MoveDown(movedis);
		else if ((int)g.GetSelfInfo()->x / 1000 == 43 && (int)g.GetSelfInfo()->y / 1000 == 13)
		{
			flag = 2;
			std::cout << flag << std::endl;
			return;
		}

	}
	else if (state == 2)
	{
	}
	else return;
}*/
//攻击敌人，同时向敌人小步移动
void newAttack(GameApi& g, std::shared_ptr<const THUAI4::Character>s_i, std::vector<std::shared_ptr<const THUAI4::Character>>enemy_i)
{	//如果没有敌人，就不要打了
	int a = (int)enemy_i.size();
	if (a == 0)
		return;
	//如果有敌人的话就挑选小紫鱼的打，其次就是打最近的
	else
	{
		int target = -1;
		for (int i = 0; i < a; i++)
		{
			if (enemy_i[i]->jobType == THUAI4::JobType::PurpleFish)
			{
				target = i;
				break;
			}
		}
		//target还是-1就说明敌人中没有小紫鱼，就打最近的
		if (target == -1)
		{
			target = 0;
			std::uint32_t* dis = new std::uint32_t[a];
			for (int i = 0; i < a; i++)
			{
				dis[i] = (enemy_i[i]->y - s_i->y) * (enemy_i[i]->y - s_i->y) + (enemy_i[i]->x - s_i->x) * (enemy_i[i]->x - s_i->x);//此处有警告，什么数据会溢出，搞不懂
			}

			for (int i = 0; i < a - 1; i++)
				if (dis[target] > dis[i + 1])
				{
					target = i + 1;
					i = target;
				}
			delete[]dis;
		}
		if (enemy_i[target]->isMoving == true)
		{
			int ms;
			double angle, maybeuseful_angle = 0, maybeuseful_angle2 = 0, maybeuseful_distance = 0;//中间计算需要的临时的变量，不管它们
			double s_x = (double)s_i->x, s_y = (double)s_i->y, e_x = (double)enemy_i[target]->x, e_y = (double)enemy_i[target]->y;
			maybeuseful_distance = sqrt((e_y - s_y) * (e_y - s_y) + (e_x - s_x) * (e_x - s_x));
			if (e_x - s_x > 30)
			{
				maybeuseful_angle2 = atan((e_y - s_y) / (e_x - s_x));
			}
			else if ((e_x - s_x) < -30)
			{
				maybeuseful_angle2 = atan((e_y - s_y) / (e_x - s_x)) + 3.1415926;
			}
			else
			{
				if ((e_y - s_y) > 0) maybeuseful_angle2 = 1.570796;
				else if ((e_y - s_y) < 0)  maybeuseful_angle2 = -1.570796;
			}
			maybeuseful_angle = asin((double)(enemy_i[target]->moveSpeed) / (Constants::BulletMoveSpeed::bullet1)*sin(enemy_i[target]->facingDirection - maybeuseful_angle2));
			angle = maybeuseful_angle + maybeuseful_angle2;
			ms = 1000 * abs(maybeuseful_distance * sin(maybeuseful_angle2) / (Constants::BulletMoveSpeed::bullet1 * sin(angle) - enemy_i[target]->moveSpeed * sin(enemy_i[target]->facingDirection)));
			g.Attack(ms, angle);
		}
		else
		{
			int ms;
			double angle;
			double s_x = (double)s_i->x, s_y = (double)s_i->y, e_x = (double)enemy_i[target]->x, e_y = (double)enemy_i[target]->y;
			ms = sqrt((e_y - s_y) * (e_y - s_y) + (e_x - s_x) * (e_x - s_x));
			if (e_x - s_x > 30)
			{
				angle = atan((e_y - s_y) / (e_x - s_x));
			}
			else if ((e_x - s_x) < -30)
			{
				angle = atan((e_y - s_y) / (e_x - s_x)) + 3.1415926;
			}
			else
			{
				if ((e_y - s_y) > 0) angle = 1.570796;
				else if ((e_y - s_y) < 0)  angle = -1.570796;
			}
			ms = 1000 * ms / Constants::BulletMoveSpeed::bullet1;
			g.Attack(ms, angle);
		}
	}
}
static int movestate = 1;
void Newcolor(GameApi& g, THUAI4::ColorType selfteamcolor, std::shared_ptr<const THUAI4::Character>self, std::vector < std::shared_ptr < const THUAI4::Wall>>wall)
{
	int x = self->x / 1000, y = self->y / 1000;
	int i = -2, j = -2;
	for (i = -2; i <= 2; i++)
		for (j = -2; j <= 2; j++)
		{
			/*if (abs(i) == 1 && abs(j) == 1)
				continue;
			if (abs(i) == 1 && abs(j) == 0)
				continue;
			if (abs(i) == 0 && abs(j) == 1)
				continue;*/
			if (abs(i) == 0 && abs(j) == 0)
				continue;
			if ((x + i > 0) && (x + i < 49) && (y + j > 0) && (y + j < 49))
			{
				int flag2 = 0;
				for (int i = 0; i < wall.size() && flag2 == 0; i++)
				{
					if (wall[i]->x / 1000 == x + i && wall[i]->y / 1000 == y + i)
						flag2 = 1;
				}
				if (g.GetCellColor(x + i, y + j) != selfteamcolor && flag2 == 0)
				{
					double ms, angle;
					ms = sqrt(double(i * i + j * j)) * 1000 / 12;
					if (i > 0)
					{
						angle = atan(j / i);
					}
					else if (i < 0)
					{
						angle = atan(j / i) + 3.1416;
					}
					else
					{
						if (j > 0) angle = 1.5708;
						else angle = -1.5708;
					}
					g.Attack((int)ms, angle);
					return;
				}
			}
			else continue;
		}
}
static int l = 1;
void color(GameApi& g, std::shared_ptr<const THUAI4::Character>self, std::vector < std::shared_ptr < const THUAI4::Wall>>wall)
{
	if (state == 1)
	{
		if (movestate == 1)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			/*if ((g.GetCellColor(self->x / 1000 + 3, self->y / 1000) != g.GetSelfTeamColor() || g.GetCellColor(self->x / 1000 - 3, self->y / 1000) != g.GetSelfTeamColor()) && self->bulletNum >= 5)
				{
					g.Attack(1.5 * at, 0);
					g.Attack(1.5 * at, 3.1415926);
				}*/
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->y / 1000 != 11)
				g.MovePlayer(movedis, 0.45);
			else movestate = 2;
		}
		else if (movestate == 2)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			/*if ((g.GetCellColor(self->x / 1000, self->y / 1000 - 2) != g.GetSelfTeamColor() || g.GetCellColor(self->x / 1000, self->y / 1000 + 3) != g.GetSelfTeamColor()) && self->bulletNum >= 5)
			{
				g.Attack(at, -1.570796);
				g.Attack(1.5 * at, 1.570796);
			}*/
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->x / 1000 != 36)
				g.MovePlayer(movedis, 0);
			else movestate = 3;
		}
		else if (movestate == 3)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			/*if ((g.GetCellColor(self->x / 1000 + 3, self->y / 1000) != g.GetSelfTeamColor() || g.GetCellColor(self->x / 1000 - 3, self->y / 1000) != g.GetSelfTeamColor()) && self->bulletNum >= 5)
			{

				g.Attack(1.5 * at, 0);
				g.Attack(1.5 * at, 3.1415926);
			}*/
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->x / 1000 != 44)
				g.MovePlayer(movedis, 0.7854);
			else movestate = 4;
		}
		else if (movestate == 4)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 500 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			/*if ((g.GetCellColor(self->x / 1000, self->y / 1000 - 3) != g.GetSelfTeamColor() || g.GetCellColor(self->x / 1000, self->y / 1000 + 2) != g.GetSelfTeamColor()) && self->bulletNum >= 5)
			{
				g.Attack(1.5 * at, -1.570796);
				g.Attack(at, 1.570796);

			}*/
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->x / 1000 != 30)
				g.MovePlayer(movedis, 2.8);
			else movestate = 5;
		}
		/*else if (movestate == 5)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 2)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			if (self->bulletNum == 1)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->x / 1000 != 31)
				g.MovePlayer(movedis, 3.1416);
			else movestate = 6;
		}
		else if (movestate == 6)
		{
		ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (ms3 - ms2 > 300 && self->bulletNum >= 2)
		{
			Newcolor(g, selfteamcolor, self, wall);
			//g.Attack(at, -1.570796);
			ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}
		if (self->bulletNum == 1)
		{
			g.Attack(0, 0);
			flag = 3;
		}
		if ((int)self->y / 1000 != 26)
			g.MovePlayer(movedis, -1.5708);
		else movestate = 7;
		}*/
		else if (movestate == 5)
		{
			std::cout << "5" << std::endl;
			if (self->bulletNum >= 9)
			{
				g.Attack(at1 * l, 0);
				g.Attack(at1 * l, pi / 4);
				g.Attack(at1 * l, pi / 2);
				g.Attack(at1 * l, pi);
				g.Attack(at1 * l, pi / 4 * 5);
				g.Attack(at1 * l, pi / 2 * 3);
				g.Attack(at1 * l, pi / 4 * 7);
				g.Attack(at1 * l, pi / 4 * 3);
				l = (l + 3) % 25;
				pi = pi + 0.5;
			}
			if (g.GetCellColor(self->x / 1000, self->y / 1000) != selfteamcolor)
			{
				g.Attack(1, 0);
			}
			movestate = 6;

		}
		else if (movestate == 6)
		{

			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 47 - self->x;
				int j = 47 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 47 - self->x;
				int j = 3 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			movestate = 5;
		}
		return;
	}
	if (state == 2)
	{
		if (movestate == 1)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			/*if ((g.GetCellColor(self->x / 1000 + 3, self->y / 1000) != g.GetSelfTeamColor() || g.GetCellColor(self->x / 1000 - 3, self->y / 1000) != g.GetSelfTeamColor()) && self->bulletNum >= 5)
				{
					g.Attack(1.5 * at, 0);
					g.Attack(1.5 * at, 3.1415926);
				}*/
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->y / 1000 != 17)
				g.MovePlayer(movedis, -1.1);
			else movestate = 2;
		}
		else if (movestate == 2)
		{
			std::cout << "5" << std::endl;
			if (self->bulletNum >= 9)
			{
				g.Attack(at1 * l, 0);
				g.Attack(at1 * l, pi / 4);
				g.Attack(at1 * l, pi / 2);
				g.Attack(at1 * l, pi);
				g.Attack(at1 * l, pi / 4 * 5);
				g.Attack(at1 * l, pi / 2 * 3);
				g.Attack(at1 * l, pi / 4 * 7);
				g.Attack(at1 * l, pi / 4 * 3);
				g.Attack(1, 0);
				l = (l + 4) % 30;
				pi = pi + 0.5;
			}
			if (g.GetCellColor(self->x / 1000, self->y / 1000) != selfteamcolor)
			{
				g.Attack(1, 0);
				flag = 3;
			}
			movestate = 3;
		}
		else if (movestate == 3)
		{

			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 47 - self->x;
				int j = 47 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 47 - self->x;
				int j = 3 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			movestate = 2;
		}
		return;
	}
	if (state == 3)
	{
		if (movestate == 1)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->x / 1000 != 12)
				g.MovePlayer(movedis, 2.62);
			else movestate = 2;
		}

		else if (movestate == 2)
		{
			std::cout << "5" << std::endl;
			if (self->bulletNum >= 9)
			{
				g.Attack(at1 * l, 0);
				g.Attack(at1 * l, pi / 4);
				g.Attack(at1 * l, pi / 2);
				g.Attack(at1 * l, pi);
				g.Attack(at1 * l, pi / 4 * 5);
				g.Attack(at1 * l, pi / 2 * 3);
				g.Attack(at1 * l, pi / 4 * 7);
				g.Attack(at1 * l, pi / 4 * 3);
				g.Attack(1, 0);
				l = (l + 3) % 25;
				pi = pi + 0.5;
			}
			if (g.GetCellColor(self->x / 1000, self->y / 1000) != selfteamcolor)
			{
				g.Attack(1, 0);
				flag = 3;
			}
			movestate = 3;
		}
		else if (movestate == 3)
		{

			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 3 - self->x;
				int j = 3 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 47 - self->x;
				int j = 3 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			movestate = 2;
		}
		return;
	}
	if (state == 4)
	{
		if (movestate == 1)
		{
			ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (ms3 - ms2 > 300 && self->bulletNum >= 3)
			{
				Newcolor(g, selfteamcolor, self, wall);
				//g.Attack(at, -1.570796);
				ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			if (self->bulletNum == 1 || self->bulletNum == 2)
			{
				g.Attack(0, 0);
				flag = 3;
			}
			if ((int)self->x / 1000 != 33)
				g.MovePlayer(movedis, -2.1);
			else movestate = 2;
		}

		else if (movestate == 2)
		{
			std::cout << "5" << std::endl;
			if (self->bulletNum >= 9)
			{
				g.Attack(at1 * l, 0);
				g.Attack(at1 * l, pi / 4);
				g.Attack(at1 * l, pi / 2);
				g.Attack(at1 * l, pi);
				g.Attack(at1 * l, pi / 4 * 5);
				g.Attack(at1 * l, pi / 2 * 3);
				g.Attack(at1 * l, pi / 4 * 7);
				g.Attack(at1 * l, pi / 4 * 3);
				g.Attack(1, 0);
				l = (l + 3) % 25;
				pi = pi + 0.5;
			}
			if (g.GetCellColor(self->x / 1000, self->y / 1000) != selfteamcolor)
			{
				g.Attack(1, 0);
				flag = 3;
			}
			movestate = 3;
		}
		else if (movestate == 3)
		{

			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 3 - self->x;
				int j = 47 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			if (self->bulletNum >= 5)
			{
				double ms, angle;
				int i = 47 - self->x;
				int j = 3 - self->y;
				ms = sqrt(double(i * i + j * j)) * 1000 / 12;
				if (i > 0)
				{
					angle = atan(j / i);
				}
				else if (i < 0)
				{
					angle = atan(j / i) + 3.1416;
				}
				else
				{
					if (j > 0) angle = 1.5708;
					else angle = -1.5708;
				}
				g.Attack((int)ms, angle);
				g.Attack((int)ms, angle);
			}
			movestate = 2;
		}
	}
}
void AI::play(GameApi& g)
{
	//全局只进行一次，确定出生位置，分配不同代码
	if (flag == 0)
	{
		const static int birthX = (int)g.GetSelfInfo()->x / 1000;
		const static int birthY = (int)g.GetSelfInfo()->y / 1000;
		selfteamcolor = g.GetSelfTeamColor();
		if (birthX < 5 && birthY < 5) state = 1;
		else if (birthX < 5 && birthY > 45)state = 2;
		else if (birthX > 45 && birthY < 5)state = 3;
		else  state = 4;
		flag = 1;
	}
	auto self = g.GetSelfInfo();;
	auto enemy = g.GetCharacters();
	//删除自己队员的信息
	for (int i = 0; i < enemy.size(); i++)
	{
		if (enemy[i]->teamID == self->teamID)
		{
			enemy.erase(enemy.begin() + i);
			i--;
		}
	}
	if (self->isDying == true)
	{
		movestate = 1;
	}
	if (flag == 1)
	{
		ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (ms1 - ms > 300)
		{
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

			auto wall = g.GetWalls();

			if (enemy.size() > 0 && self->bulletNum > 1)
				newAttack(g, self, enemy);
			else
			{
				color(g, self, wall);
			}

		}
	}
	if (flag == 3)
	{
		ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (ms1 - ms > 4000)
		{
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			flag = 1;
		}
		if (enemy.size() > 0 && self->bulletNum > 1)
			newAttack(g, self, enemy);
	}
	x22 = self->x / 1000;
	y22 = self->y / 1000;
	if (x22 == x11 && y22 == y11)
	{
		ms7 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (ms7 - ms6 > 200000)
		{
			if (state == 1)
				movestate = 5;
			else movestate = 2;
		}
	}
	else
	{
		x11 = self->x / 1000;
		y11 = self->y / 1000;
		ms6 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
}


