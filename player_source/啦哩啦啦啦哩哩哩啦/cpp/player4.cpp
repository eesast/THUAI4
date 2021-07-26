#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job5; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

//以下为提交的代码
static int step = 0;   //计算movecool调用次数 调整染色频率
static long long int times = 0;//计算player调用次数，类似于计时器
static int ex = 1;			//记录上一次所在的x位置，防止卡住
static int wai = 1;			//记录上一次所在的y位置，防止卡住
static int blocktime = 0;	//记录不动的次数，防止卡住
static int swich = 1;	//出生地获取函数开关，确保只调用一次
static int birth;     //出生地位置记录，1在上面，2在下面
static int specialblocked = 0; //防止特殊卡墙

int birthplace(GameApi& g)	//获取出生地
{
	int birthplace;
	int a, b;
	auto p = g.GetBirthPoints();
	if (p.size())
	{
		a = p[0]->x;
		b = p[0]->y;
		if (p[0]->x < 25000)
		{
			swich = 0;
			return 1;	//出生点在上方
		}
		else if (p[0]->x > 25000)
		{
			swich = 0;
			return 2;	//出生点在下方
		}
	}


}

bool blocked(GameApi& g)	//移动监测，卡住了则返回true，否则false
{
	int a = g.GetSelfInfo()->x;
	int b = g.GetSelfInfo()->y;
	if ((g.GetSelfInfo()->x - ex) / 1000 == 0 && (g.GetSelfInfo()->y - wai) / 1000 == 0) blocktime++;
	if (blocktime > 500)
	{
		ex = g.GetSelfInfo()->x;
		wai = g.GetSelfInfo()->y;
		blocktime = 0;
		return true;
	}
	else
	{
		ex = g.GetSelfInfo()->x;
		wai = g.GetSelfInfo()->y;
		return false;
	}

}

bool ahead_wall(GameApi& g, int distance_require, int moveDir)
{
	auto wallinfo = g.GetWalls();
	int b = wallinfo.size();
	auto selfx = g.GetSelfInfo()->x, selfy = g.GetSelfInfo()->y;
	if (moveDir == 0 || moveDir == 2)
	{
		if (b)
			for (int k = 0; k < b; k += 1)
				if (selfy / 1000 == wallinfo[k]->y / 1000 && (selfx / 1000 - wallinfo[k]->x / 1000) == distance_require * (moveDir - 1)) return false;
	}
	else if (moveDir == 1 || moveDir == 3)
	{
		if (b)
			for (int k = 0; k < b; k += 1)
				if (selfx / 1000 == wallinfo[k]->x / 1000 && (selfy / 1000 - wallinfo[k]->y / 1000) == distance_require * (moveDir - 2)) return false;
	}
	return true;
}

double distanceCalculate(GameApi& g, int64_t x, int64_t y)
{
	int64_t deltaX = x - g.GetSelfInfo()->x, deltaY = y - g.GetSelfInfo()->y;
	uint32_t time_in_mili;
	double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
	return distance;
}
double angleCalculate(GameApi& g, int64_t x, int64_t y)
{
	int64_t deltaX = x - g.GetSelfInfo()->x, deltaY = y - g.GetSelfInfo()->y;
	if (deltaX == 0 && deltaY == 0) return 0;
	double angle = atan2(deltaY, deltaX);
	return angle;
}
bool enemy_around(GameApi& g, std::shared_ptr<const THUAI4::Character>& target)
{
	auto characterinfo = g.GetCharacters();
	auto self = g.GetSelfInfo()->teamID;
	int num = characterinfo.size();
	for (int i = 0; i < num; i++)
	{
		if (num && self != characterinfo[i]->teamID && !characterinfo[i]->isDying)
		{
			target = characterinfo[i];
			return true;
		}
	}
	return false;
}
void movecool(GameApi& g, int a, int b)		//定点移动函数
{
	if (g.GetSelfInfo()->x / 1000 < a && ahead_wall(g, 1, 0)) g.MoveDown((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	if (g.GetSelfInfo()->y / 1000 < b && ahead_wall(g, 1, 1)) g.MoveRight((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	if (g.GetSelfInfo()->x / 1000 > a && ahead_wall(g, 1, 2)) g.MoveUp((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	if (g.GetSelfInfo()->y / 1000 > b && ahead_wall(g, 1, 3)) g.MoveLeft((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	step++;
}



void scan(GameApi& g) //巡航函数
{
	if (birth == 1)
		switch ((times / 500) % 3)
		{
		case 0:movecool(g, 25, 25); break;
		case 1:movecool(g, 44, 3); break;
		case 2:movecool(g, 44, 45); break;
	
		}
	else
	{
		switch ((times / 500) % 3)
		{
		case 0:movecool(g, 25, 25); break;
		case 1:movecool(g, 6, 3); break;
		case 2:movecool(g, 6, 45); break;

		}
	}
}
void noblock(GameApi& g) //防堵函数
{
	int i;
	if (blocked(g))
	{
		for (i = 0; i <= 200; i++)
			movecool(g, 25, 25);
	}
	specialblocked++;

}
void nonoblock(GameApi& g) //进一步防堵函数
{
	if (specialblocked % 5 == 0) g.MoveRight((10000 * 1000) / g.GetSelfInfo()->moveSpeed);
}



void chasecool(GameApi& g, std::shared_ptr<const THUAI4::Character>& target)   //追逐函数（优先调用）
{
	double beta = angleCalculate(g, target->x, target->y);
	if (distanceCalculate(g, target->x, target->y) <= 2000) g.Attack(1, beta);
	else g.MovePlayer((3000 * 1000) / g.GetSelfInfo()->moveSpeed, beta);

}
void recovercool(GameApi& g)   //恢复子弹
{
	int bug = 0;
	if (g.GetCellColor(g.GetSelfInfo()->x / 1000, g.GetSelfInfo()->y / 1000) == g.GetSelfTeamColor())
		while (g.GetSelfInfo()->bulletNum < 4 && bug < 30)
		{
			bug++;
		}
	else scan(g);
}
void flipbullet(GameApi& g)	//随缘躲子弹函数
{
	if (g.GetBullets().size()) g.MovePlayer((3000 * 1000) / g.GetSelfInfo()->moveSpeed, 3.1415926 / 4.0);
}

void pickcool(GameApi& g)    //捡道具函数（随捡随用）
{
	auto prop = g.GetProps();
	if (prop.size())
	{
		if ((15 * 1000 < prop[0]->x < 35 * 1000) && (15 * 1000 < prop[0]->y < 35 * 1000))
		{
			double beta = angleCalculate(g, prop[0]->x, prop[0]->y);
			g.MovePlayer((1000 * 1000) / g.GetSelfInfo()->moveSpeed, beta);
			if (prop[0]->x / 1000 == g.GetSelfInfo()->x / 1000 && prop[0]->y / 1000 == g.GetSelfInfo()->y / 1000)
			{
				g.Pick(prop[0]->propType);
				g.Use();
			}
		}
		else scan(g);
	}
}

void color(GameApi& g, int freq) //上色函数
{
	if (step > freq && g.GetCellColor(g.GetSelfInfo()->x / 1000, g.GetSelfInfo()->y / 1000) != g.GetSelfTeamColor())
	{
		g.Attack(1, 0);
		step = 0;
	}
}

void AI::play(GameApi& g)
{
	times++;
	std::shared_ptr<const THUAI4::Character> target;
	if (swich == 1) birth = birthplace(g);	//开局时获取出生地
	if (enemy_around(g, target) && g.GetSelfInfo()->bulletNum && times > 100) chasecool(g, target);//优先攻击敌人
	else if (g.GetProps().size()) pickcool(g);						//其次拾取道具
	else if (g.GetSelfInfo()->bulletNum <= 2) recovercool(g);		//子弹数目小于等于2回复子弹
	else scan(g);					//一般巡航
	flipbullet(g);					//时刻躲子弹
	noblock(g);						//时刻防堵
	nonoblock(g);
	color(g, 50);					//偶尔上色
}