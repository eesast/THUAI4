#include "AI.h"
#include "Constants.h"
#define pi 3.1415926

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */
int step = 0;
int step1 = 0;
extern const THUAI4::JobType playerJob = THUAI4::JobType::Job2; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{std::random_device{}()};
}
void moveplayer(GameApi& g,uint32_t a);
void moveplayer1(GameApi& g, uint32_t a, double b);
void attack(GameApi& g);
void AI::play(GameApi &g)
{
	auto self = g.GetSelfInfo();
	auto player = g.GetCharacters();
	auto wall = g.GetWalls();
	auto color = g.GetSelfTeamColor();
	double dire = 0;
	auto col1 = g.GetCellColor(self->x / 1000, self->y / 1000);
	int ci = 0;
	if (self->x<=(uint32_t)1000*50/3) g.MoveDown(1000);
	if (self->x >= (uint32_t)1000 * 100 / 3) g.MoveUp(1000);
	if(self->y <= (uint32_t)1000 * 50 / 3) g.MoveRight(1000);
	if (self->y >= (uint32_t)1000 * 100 / 3) g.MoveLeft(1000);
	while (col1 == color) 
	{
		ci++;
		moveplayer(g, 3000);
		col1 = g.GetCellColor(self->x / 1000, self->y / 1000);
		if (ci == 2) break;
	}
	g.Attack(5000, direction(e));
	moveplayer(g,2000);
	auto tool = g.GetProps();
	auto numtool = tool.size();
	if (numtool > 0)
	{
		auto tool1 = tool[0];
		auto dis = sqrt((self->x - tool1->x) ^ 2 + (self->y - tool1->y) ^ 2);
		for (int i = 0; i < numtool; i++)
		{
			if (tool[i]->propType == THUAI4::PropType::Rice)
			{
				tool1 = tool[i];
				dis = sqrt((self->x - tool1->x) ^ 2 + (self->y - tool1->y) ^ 2);
				break;
			}

			else if (dis > sqrt((self->x - tool[i]->x) ^ 2 + (self->y - tool[i]->y) ^ 2))
			{
				tool1 = tool[i];
				dis = sqrt((self->x - tool1->x) ^ 2 + (self->y - tool1->y) ^ 2);
			}
		}
		if ((tool1->x/1000) == (self->x/1000))
		{
			if (tool1->y - self->y < 0) dire = -pi / 2;
			if (tool1->y - self->y  > 0) dire = pi / 2;
			if (tool1->y  - self->y == 0) dire = 0;
		}
		if (tool1->x  > self->x )
			dire = atan((tool1->y - self->y) / (tool1->x - self->x));
		if (tool1->x / 1000 <self->x )
			dire = pi - (atan((tool1->y - self->y) / (self->x - tool1->x)));
		double tim = dis / self->moveSpeed;
		moveplayer1(g, (uint32_t)tim, dire);
		self = g.GetSelfInfo();
		if ((tool1->x / 1000) == (self->x / 1000) && (tool1->y / 1000) - (self->y / 1000) == 0)
		{
			g.Pick(tool1->propType);
			g.Use();
		}
	}
	if (self->bulletNum)
	{
		g.Attack(1000, direction(e));
	}
	else
	{
		while (self->bulletNum <= 9)
		{
			auto col = g.GetCellColor(self->x / 1000, self->y / 1000);
			while (col != color) 
			{
				bool aa = false;
				double dir;
				for (int i = -3; i <= 3; i++)
				{
					for (int j = -3; j <= 3; j++)
					{
						if (self->x+i*1000>=0&&self->x+i*1000<50000&& self->y + j * 1000 >= 0 && self->y + j * 1000 < 50000)
						{
							col = g.GetCellColor(self->x / 1000 + i, self->y / 1000 + j);
						if (col == color)
						{
							aa = true;
							if (i == 0)
							{
								if (j < 0) dir = -pi / 2;
								if (j > 0) dir = pi / 2;
								if (j == 0) dir = 0;
							}
							else
							{
								if (i > 0)
									dir = atan((double)j / (double)i);
								if (i < 0)
									dir = pi - (atan((double)j / (double)i));
							}
							double dis = sqrt(i * i + j * j) * 1000;
							double tim = dis / self->moveSpeed;
							moveplayer1(g, (uint32_t)tim, dir);
							col = color;
						}
						}
					}
					if (aa == true) break;
				}
				if (aa == false)
				{
					col= g.GetCellColor(self->x / 1000, self->y / 1000);
					while (col != color) 
					{
						self = g.GetSelfInfo();
						if (self->bulletNum > 0) g.Attack(50, 0);
						else g.MovePlayer(2000, 2 * pi - self->facingDirection);
						self = g.GetSelfInfo();
						col = g.GetCellColor(self->x / 1000, self->y / 1000);
					}
				}
				else
				{
					self = g.GetSelfInfo();
					col = g.GetCellColor(self->x / 1000, self->y / 1000);
				}
			}
			self= g.GetSelfInfo();
		}
	}
	tool = g.GetProps();
	numtool = tool.size();
	if (numtool > 0)
	{
		auto tool1 = tool[0];
		auto dis = sqrt((self->x - tool1->x) ^ 2 + (self->y - tool1->y) ^ 2);
		for (int i = 0; i < numtool; i++)
		{
			if (tool[i]->propType == THUAI4::PropType::Rice)
			{
				tool1 = tool[i];
				dis = sqrt((self->x - tool1->x) ^ 2 + (self->y - tool1->y) ^ 2);
				break;
			}

			else if (dis > sqrt((self->x - tool[i]->x) ^ 2 + (self->y - tool[i]->y) ^ 2))
			{
				tool1 = tool[i];
				dis = sqrt((self->x - tool1->x) ^ 2 + (self->y - tool1->y) ^ 2);
			}
		}
		if (tool1->x == self->x)
		{
			if (tool1->y - self->y < 0) dire = -pi / 2;
			if (tool1->y - self->y > 0) dire = pi / 2;
			if (tool1->y - self->y == 0) dire = 0;
		}
		if (tool1->x > self->x)
			dire = atan((tool1->y - self->y) / (tool1->x - self->x));
		if (tool1->x < self->x)
			dire = pi - (atan((tool1->y - self->y) / (self->x - tool1->x)));
		double tim = dis / self->moveSpeed;
		moveplayer1(g, (uint32_t)tim, dire);
		self = g.GetSelfInfo();
		if ((tool1->x / 1000) == (self->x / 1000) && (tool1->y / 1000) - (self->y / 1000) == 0)
		{
			g.Pick(tool1->propType);
			g.Use();
		}
	}
	attack(g);
}

void moveplayer(GameApi& g, uint32_t a)
{
	auto self = g.GetSelfInfo();
	auto pBirthPoints = g.GetBirthPoints();
	auto pWall = g.GetWalls();
	auto count_BirthPoints = pBirthPoints.size();
	auto count_Walls = pWall.size();
	double distance_Walls = 5000;
	double distance_BirthPoints = 5000;
	g.MovePlayer(500, self->facingDirection);
	for (uint32_t ii = 1; ii * 1000 + 500< a; ii++)
	{
		pBirthPoints = g.GetBirthPoints();
		pWall = g.GetWalls();
		count_BirthPoints = pBirthPoints.size();
		count_Walls = pWall.size();
		distance_Walls = 5000;
		distance_BirthPoints = 5000;
		double angl1=0, angl2=pi/2;
		if (count_Walls == 0 && count_BirthPoints == 0)
		{
			g.MovePlayer(1000, self->facingDirection);
		}
		if (count_Walls > 0 || count_BirthPoints > 0)
		{
			if (count_Walls > 0)
			{
				for (int i = 0; i < count_Walls; i++)      //判断最近墙体距离
				{
					if (sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y))) < distance_Walls)
					{
						distance_Walls = sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y)));
						if (pWall[i]->x == self->x) 
						{ 
							if (pWall[i]->y >= self->y) angl1 = pi / 2;
							if (pWall[i]->y < self->y) angl1 = 3 * pi / 2;
						}
						if (pWall[i]->x > self->x) angl1 = atan((pWall[i]->y - self->y) / (pWall[i]->x - self->x));
						if (pWall[i]->x <self->x) angl1 = atan((pWall[i]->y - self->y) / (pWall[i]->x - self->x))+pi;
					}
				}
			}
			if (count_BirthPoints > 0)
			{
				for (int i = 0; i < count_BirthPoints; i++)      //判断最近出生点距离
				{
					if (sqrt(((self->x) - (pBirthPoints[i]->x)) * ((self->x) - (pBirthPoints[i]->x)) + ((self->y) - (pBirthPoints[i]->y)) * ((self->y) - (pBirthPoints[i]->y))) < distance_BirthPoints)
					{
						distance_BirthPoints = sqrt(((self->x) - (pBirthPoints[i]->x)) * ((self->x) - (pBirthPoints[i]->x)) + ((self->y) - (pBirthPoints[i]->y)) * ((self->y) - (pBirthPoints[i]->y)));
						if (pBirthPoints[i]->x == self->x)
						{
							if (pBirthPoints[i]->y >= self->y) angl2 = pi / 2;
							if (pBirthPoints[i]->y < self->y) angl2 = 3 * pi / 2;
						}
						if (pBirthPoints[i]->x > self->x) angl2 = atan((pBirthPoints[i]->y - self->y) / (pBirthPoints[i]->x - self->x));
						if (pBirthPoints[i]->x < self->x) angl2= atan((pBirthPoints[i]->y - self->y) / (pBirthPoints[i]->x - self->x)) + pi;
					}
				}
			}
			self = g.GetSelfInfo();
			if ((ii + 1) * 1000 + 500 > a) 
			{
				if (count_Walls > 0)
				{
					double Tim = (distance_Walls) / self->moveSpeed;
					g.Attack((uint32_t)Tim, angl1);
				}
				if (count_BirthPoints > 0)
				{
					double Tim = (distance_BirthPoints) / self->moveSpeed;
					g.Attack((uint32_t)Tim, angl2);
				}
			}
			if ((distance_Walls < 2001 || distance_BirthPoints < 2001) && step > 0)
			{
				if ((rand() % 100) < 51)
				{
					g.MovePlayer(2000, self->facingDirection + 3.1415926 / 2);
				}
				else 
				{
					g.MovePlayer(2000, self->facingDirection - 3.1415926 / 2);
				}
			}
			if (distance_Walls > 2000 && distance_BirthPoints > 1000)
				g.MovePlayer(2000, self->facingDirection);
		}
	}
	if (step == 0)
	{
		g.MoveRight(600);
		g.MoveDown(600);
		step++;
	}
}

void moveplayer1(GameApi& g, uint32_t a,double b)
{
	auto self = g.GetSelfInfo();
	auto pBirthPoints = g.GetBirthPoints();
	auto pWall = g.GetWalls();
	auto count_BirthPoints = pBirthPoints.size();
	auto count_Walls = pWall.size();
	double distance_Walls = 5000;
	double distance_BirthPoints = 5000;
	g.MovePlayer(1000, b);
	for (uint32_t ii = 1; ii * 1500 + 1000< a; ii++)
	{
		pBirthPoints = g.GetBirthPoints();
		pWall = g.GetWalls();
		count_BirthPoints = pBirthPoints.size();
		count_Walls = pWall.size();
		distance_Walls = 5000;
		distance_BirthPoints = 5000;
		double angl1=0, angl2=pi/2;
		if (count_Walls == 0 && count_BirthPoints == 0)
		{
			g.MovePlayer(1000, self->facingDirection);
		}
		if (count_Walls > 0 || count_BirthPoints > 0)
		{
			if (count_Walls > 0)
			{
				for (int i = 0; i < count_Walls; i++)      //判断最近墙体距离
				{
					if (sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y))) < distance_Walls)
					{
						distance_Walls = sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y)));
						if (pWall[i]->x == self->x)
						{
							if (pWall[i]->y >= self->y) angl1 = pi / 2;
							if (pWall[i]->y < self->y) angl1 = 3 * pi / 2;
						}
						if (pWall[i]->x > self->x) angl1 = atan((pWall[i]->y - self->y) / (pWall[i]->x - self->x));
						if (pWall[i]->x < self->x) angl1 = atan((pWall[i]->y - self->y) / (pWall[i]->x - self->x)) + pi;
					}
				}
			}
			if (count_BirthPoints > 0)
			{
				for (int i = 0; i < count_BirthPoints; i++)      //判断最近出生点距离
				{
					if (sqrt(((self->x) - (pBirthPoints[i]->x)) * ((self->x) - (pBirthPoints[i]->x)) + ((self->y) - (pBirthPoints[i]->y)) * ((self->y) - (pBirthPoints[i]->y))) < distance_BirthPoints)
					{
						distance_BirthPoints = sqrt(((self->x) - (pBirthPoints[i]->x)) * ((self->x) - (pBirthPoints[i]->x)) + ((self->y) - (pBirthPoints[i]->y)) * ((self->y) - (pBirthPoints[i]->y)));
						if (pBirthPoints[i]->x == self->x)
						{
							if (pBirthPoints[i]->y >= self->y) angl2 = pi / 2;
							if (pBirthPoints[i]->y < self->y) angl2 = 3 * pi / 2;
						}
						if (pBirthPoints[i]->x > self->x) angl2 = atan((pBirthPoints[i]->y - self->y) / (pBirthPoints[i]->x - self->x));
						if (pBirthPoints[i]->x < self->x) angl2 = atan((pBirthPoints[i]->y - self->y) / (pBirthPoints[i]->x - self->x)) + pi;
					}
				}
			}
			self = g.GetSelfInfo();
			if ((ii + 1) * 1000 + 500 > a)
			{
				if (count_Walls > 0)
				{
					double Tim = (distance_Walls) / self->moveSpeed;
					g.Attack((uint32_t)Tim, angl1);
				}
				if (count_BirthPoints > 0)
				{
					double Tim = (distance_BirthPoints) / self->moveSpeed;
					g.Attack((uint32_t)Tim, angl2);
				}
			}
			if ((distance_Walls < 1501 || distance_BirthPoints < 1501) && step1 > 0) 
			{
				if ((rand() % 100) < 51)
					g.MovePlayer(1500, self->facingDirection + 3.1415926 / 2);
				else
					g.MovePlayer(1500, self->facingDirection - 3.1415926 / 2);
			}
			if (distance_Walls > 1500 && distance_BirthPoints > 1500)
				g.MovePlayer(1500, self->facingDirection);
		}
	}
	if (step1 == 0)
	{
		g.MovePlayer(1500,b);
		step1++;
	}
}

void attack(GameApi& g)
{
	auto self = g.GetSelfInfo();
	auto enemy = g.GetCharacters();
	auto num = enemy.size();
	double distance_Enemies = 0;
	for (int i = 0; i < num; i++)
	{
		if (enemy[i]->teamID != self->teamID)
		{
			distance_Enemies = sqrt(((self->x) - (enemy[i]->x)) ^ 2 + ((self->y) - (enemy[i]->x)) ^ 2);
			if (distance_Enemies < 1000)
			{
				double angle=0;
				double a;
				a = ((enemy[i]->y) - (self->y)) / ((enemy[i]->x) - (self->x));
				if (((enemy[i]->x) - (self->x)) > 0)
					angle = atan(a);
				if (((enemy[i]->x) - (self->x)) < 0)
					angle = atan(a) + pi;
				if (((enemy[i]->x) - (self->x)) == 0)
				{
					if (((enemy[i]->y) - (self->y)) > 0) angle = pi / 2;
					if (((enemy[i]->y) - (self->y)) < 0) angle = -pi / 2;
					if(((enemy[i]->y) - (self->y)) == 0) angle = 0;
				}
				double time1 = distance_Enemies / self->moveSpeed;
				g.Attack((uint32_t)time1, angle);
			}
		}
	}
}