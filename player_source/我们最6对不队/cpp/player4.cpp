#include "AI.h"
#include "Constants.h"
#include<chrono>

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
static auto msec0 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
long long msec1;
int t1 = 300, t2 = 400, dr1 = 20, dr2 = 200;

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
		if (enemy_i[target]->isDying == false)
		{
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
}

void bornplace_hr(GameApi& g) {
	msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	static int state = 1, astate = 0;
	auto self = g.GetSelfInfo();
	static double x = 1500, y = 1500;
	double x0 = self->x, y0 = self->y;
	double angle, distance;
	static int flag = 0;
	if (self->isDying == true) {
		state = 1;
		astate = 0;
		flag = 0;
	}

	if ((state == -1 || state == 0 || state == 1 || state == 2 || state == 3 || state == 4))//not arriving at the right
	{
		if (self->y < 48500) {
			flag = 1;
		}
		else if (self->x <= 47500 && g.GetCellColor((self->x) / 1000 + 1, (self->y) / 1000) != g.GetSelfTeamColor()) {
			state = 0;
		}
		else if (self->x >= 2500 && g.GetCellColor((self->x) / 1000 - 1, (self->y) / 1000) != g.GetSelfTeamColor()) {
			state = -1;
		}
	}

	/*if (self->bulletNum == 0 && g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor()) {
		if (self->y >= 48500 && msec1 - msec0 > t1) {
			msec1 = msec0;
			if (self->x >= 2500 && g.GetCellColor(self->x / 1000 - 1, self->y / 1000) == g.GetSelfTeamColor()) {
				g.MoveUp(dr1);
			}
			else {
				g.MoveDown(dr1);
			}
		}
		else if (self->x <= 1500 && msec1 - msec0 > t1) {
			msec1 = msec0;
			if (self->y >= 2500 && g.GetCellColor(self->x / 1000, self->y / 1000 - 1) == g.GetSelfTeamColor()) {
				g.MoveLeft(dr1);
			}
			else {
				g.MoveRight(dr1);
			}
		}
	}*/
	if (flag == 1) {
		g.MoveRight(50);
		flag = 0;
	}
	else if (state == -1 && msec1 - msec0 > t2) {
		g.Attack(dr2, 3.1415926);
		state = 3;
		msec1 = msec0;
	}
	else if (state == 0 && msec1 - msec0 > t2) {
		g.Attack(dr2, 0);
		state = 1;
		msec1 = msec0;
	}
	else if (state == 1 && msec1 - msec0 > t1) {
		g.MoveDown(dr1);
		msec1 = msec0;
		state = 2;
		if (self->x >= 48500) {
			state = 3;
		}
	}
	else if (state == 2 && msec1 - msec0 > t2) {
		if (self->bulletNum >= 1) {
			angle = atan((y - y0) / (x - x0));
			if (x - x0 < 0) {
				angle += 3.1415926;
			}
			distance = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
			g.Attack(distance / 12, angle);

			if (astate == 0) {
				if (y <= 45000) {
					y += 3000;
				}
				else {
					astate = 1;
				}
			}
			else if (astate == 1) {
				x += 3000;
				y = 1500;
				astate = 0;
				if (x >= 45000) {
					astate = 2;
				}
			}
		}
		if (astate == 2) {
			astate = 0;
			x = 47500;
			y = 1500;
		}
		msec1 = msec0;
		state = 1;
	}

	else if (state == 3 && msec1 - msec0 > t1) {
		g.MoveUp(dr1);
		msec1 = msec0;
		state = 4;
		if (self->x <= 1500) {
			state = 1;
		}
	}
	else if (state == 4 && msec1 - msec0 > t2) {
	if (self->bulletNum >= 1) {
		angle = atan((y - y0) / (x - x0));
		if (x - x0 < 0) {
			angle += 3.1415926;
		}
		distance = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
		g.Attack(distance / 12, angle);

		if (astate == 0) {
			if (y <= 45000) {
				y += 3000;
			}
			else {
				astate = 1;
			}
		}
		else if (astate == 1) {
			x += 3000;
			y = 1500;
			astate = 0;
			if (x >= 45000) {
				astate = 2;
			}
		}
	}
	if (astate == 2) {
		astate = 0;
		x = 1500;
		y = 1500;
	}
		msec1 = msec0;
		state = 3;
	}
}

void bornplace_hl(GameApi& g) {
	msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	static int state = 0;
	auto self = g.GetSelfInfo();
	static double x = 1500, y = 1500;
	double x0 = self->x, y0 = self->y;
	double angle, distance;
	bool flag = true;

	if (self->isDying == true) {
		state = 0;
	}
	if (g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor() && flag == true && self->bulletNum > 0) {
		g.Attack(1, 0);
		flag = false;
		msec1 = msec0;
		return;
	}

	if (msec1 - msec0 > 500 && self->bulletNum > 1) {
		angle = atan((y - y0) / (x - x0));
		if (x - x0 < 0) {
			angle += 3.1415926;
		}
		distance = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
		g.Attack(distance / 12, angle);

		if (state == 0) {
			if (y <= 45000) {
				y += 3000;
			}
			else {
				state = 1;
			}
		}
		else if (state == 1) {
			x += 3000;
			y = 1500;
			state = 0;
			if (x >= 45000) {
				state = 2;
			}
		}

		msec1 = msec0;
	}

	if (state == 2) {
		state = 0;
		x = 1500;
		y = 1500;
	}
	//if (msec1 - msec0 > 1000) {
	//	flag = true;
	//	msec1 = msec0;
	//}
}

void bornplace_lr(GameApi& g) {
	msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	static int state = 1, astate = 0;
	auto self = g.GetSelfInfo();
	static double x = 47500, y = 1500;
	double x0 = self->x, y0 = self->y;
	double angle, distance;
	static int flag = 0;
	if (self->isDying == true) {
		state = 1;
		astate = 0;
		flag = 0;
	}

	if ((state == -1 || state == 0 || state == 1 || state == 2 || state == 3 || state == 4))//not arriving at the right
	{
		if (self->y < 48500) {
			flag = 1;
		}
		else if (self->x <= 47500 && g.GetCellColor((self->x) / 1000 + 1, (self->y) / 1000) != g.GetSelfTeamColor()) {
			state = 0;
		}
		else if (self->x >= 2500 && g.GetCellColor((self->x) / 1000 - 1, (self->y) / 1000) != g.GetSelfTeamColor()) {
			state = -1;
		}
	}

	/*if (self->bulletNum == 0 && g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor()) {
		if (self->y >= 48500 && msec1 - msec0 > t1) {
			msec1 = msec0;
			if (self->x >= 2500 && g.GetCellColor(self->x / 1000 - 1, self->y / 1000) == g.GetSelfTeamColor()) {
				g.MoveUp(dr1);
			}
			else {
				g.MoveDown(dr1);
			}
		}
		else if (self->x <= 1500 && msec1 - msec0 > t1) {
			msec1 = msec0;
			if (self->y >= 2500 && g.GetCellColor(self->x / 1000, self->y / 1000 - 1) == g.GetSelfTeamColor()) {
				g.MoveLeft(dr1);
			}
			else {
				g.MoveRight(dr1);
			}
		}
	}*/
	if (flag == 1) {
		g.MoveRight(50);//move up 
		flag = 0;
	}
	else if (state == -1 && msec1 - msec0 > t2) {
		g.Attack(dr2, 3.1415926);
		state = 3;
		msec1 = msec0;
	}
	else if (state == 0 && msec1 - msec0 > t2) {
		g.Attack(dr2, 0);
		state = 1;
		msec1 = msec0;
	}
	else if (state == 1 && msec1 - msec0 > t1) {
		g.MoveUp(dr1);
		msec1 = msec0;
		state = 2;
		if (self->x <= 1500) {
			state = 3;
		}
	}
	else if (state == 2 && msec1 - msec0 > t2) {
		if (self->bulletNum >= 1) {
			angle = atan((y - y0) / (x - x0));
			if (x - x0 < 0) {
				angle += 3.1415926;
			}
			distance = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
			g.Attack(distance / 12, angle);

			if (astate == 0) {
				if (y <= 45000) {
					y += 3000;
				}
				else {
					astate = 1;
				}
			}
			else if (astate == 1) {
				x -= 3000;
				y = 1500;
				astate = 0;
				if (x < 5000) {
					astate = 2;
				}
			}
		}
		if (astate == 2) {
			astate = 0;
			x = 47500;
			y = 1500;
		}
		msec1 = msec0;
		state = 1;
	}

	else if (state == 3 && msec1 - msec0 > t1) {
		g.MoveDown(dr1);
		msec1 = msec0;
		state = 4;
		if (self->x >= 48500) {
			state = 1;
		}
	}
	else if (state == 4 && msec1 - msec0 > t2) {
		if (self->bulletNum >= 1) {
			angle = atan((y - y0) / (x - x0));
			if (x - x0 < 0) {
				angle += 3.1415926;
			}
			distance = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
			g.Attack(distance / 12, angle);

			if (astate == 0) {
				if (y <= 45000) {
					y += 3000;
				}
				else {
					astate = 1;
				}
			}
			else if (astate == 1) {
				x -= 3000;
				y = 1500;
				astate = 0;
				if (x < 5000) {
					astate = 2;
				}
			}
		}
		if (astate == 2) {
			astate = 0;
			x = 47500;
			y = 1500;
		}
		msec1 = msec0;
		state = 3;
	}
}

void bornplace_ll(GameApi& g) {

}

void AI::play(GameApi& g)
{
	auto enemies = g.GetCharacters();
	auto self = g.GetSelfInfo();
	static int const x0 = self->x;
	static int const y0 = self->y;
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i]->teamID == self->teamID) {
			enemies.erase(enemies.begin() + i);
			i--;
		}
	}
	if (enemies.size() > 0 && g.GetCellColor(self->x / 1000, self->y / 1000) == g.GetSelfTeamColor()) {
		newAttack(g, self, enemies);
	}
	else if(enemies.size() > 0 && self->bulletNum > 1) {
		newAttack(g, self, enemies);
	}
	else {
		if (x0 < 6000 && y0 < 6000) {
			bornplace_hl(g);
		}
		else if (x0 < 6000 && y0 > 44000) {
			bornplace_hr(g);
		}
		else if (x0 > 44000 && y0 < 6000) {
			bornplace_ll(g);
		}
		else if (x0 > 44000 && y0 > 44000) {
			bornplace_lr(g);
		}
	}
}