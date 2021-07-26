#include "AI.h"
#include "Constants.h"

//峈樑寀play()覃蚚潔蚔牁袨怓載陔郯ㄛ峈淩寀硐悵痐絞蚔牁袨怓祥頗掩袨怓載陔滲杅迵GameApi腔源楊肮奀溼恀
extern const bool asynchronous = false;
#include <vector>
#include <random>
#include <iostream>
#include <math.h>
#include <string>
#include <chrono>
/* 衾 VS2019 砐醴扽俶笢羲 C++17 梓袧ㄩ/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job3; //恁忒眥珛ㄛ恁忒 !!斛剕!! 隅砱森曹講懂恁寁眥珛

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
#define int long long
#define PI 3.1415926
#define start_time  1618993950538ll
#define degree PI/120
GameApi* temp;

static int isWalls[50][50];
static int walls[50][50];
int Minimum(int a, int b, int c, int d);
void attack();
void run(double a[80][4], int b);
int pick();
void hide(double a[4][3]);
void addWalls();
void message();
void getMes();
inline int getTime();
void AI::play(GameApi& g)
{
	int det = 0;
	double P[4][3], B[80][4]; int E = 0, j = 0, b = 0;
	temp = &g;
	std::vector <std::shared_ptr<const THUAI4::Prop>> vp = g.GetProps();
	std::vector <std::shared_ptr<const THUAI4::Character>> ve = g.GetCharacters();
	std::vector <std::shared_ptr<const THUAI4::Wall>> vw = g.GetWalls();
	std::vector <std::shared_ptr<const THUAI4::Bullet>> vb = g.GetBullets();
	auto self = g.GetSelfInfo();
	for (int i = 0; i < (int)vw.size(); i++)
	{
		if (isWalls[(vw[i]->x) / 1000][(vw[i]->y) / 1000] != 1)
		{
			walls[(vw[i]->x) / 1000][(vw[i]->y) / 1000] = 1;
		}
	}
	for (int i = 0; i < (int)ve.size(); i++)
	{
		if (ve[i]->teamID != self->teamID)
		{
			P[j][0] = (double)((unsigned char)(ve[i]->jobType));
			P[j][1] = atan2((ve[i]->y - self->y), (ve[i]->x - self->x));
			if (P[j][1] < 0)
				P[j][1] += PI * 2;
			P[j][2] = sqrt(pow((ve[i]->y - self->y), 2) + pow((ve[i]->x - self->x), 2));
			E++;
		}
	}
	for (int i = 0; i < (int)vb.size(); i++)
	{
		if (vb[i]->teamID != self->teamID)
		{
			if (sqrt(pow(vb[i]->x - self->x, 2) + pow(vb[i]->y - self->y, 2)) <= 2400)
			{
				B[b][0] = vb[i]->moveSpeed;
				B[b][1] = vb[i]->x;
				B[b][2] = vb[i]->y;
				B[b][3] = vb[i]->facingDirection;
				b++;
			}
		}
	}
	for (int i = b; i < 80; i++)
	{
		B[i][0] = 0;
		B[i][1] = 0;
		B[i][2] = 0;
		B[i][3] = 0;
	}
	getMes();
	attack();
	if (self->hp == 0 && (int)((unsigned char)(self->propType)) == 6)
	{
		g.Use();
	}
	//message();
	addWalls();
	g.MovePlayer(100, direction(e));
}
void attack()
{
	auto self = temp->GetSelfInfo();
	int a, b; a = (self->x) / 1000; b = (self->y) / 1000;
	if ((a > 1 && a < 49) && (b > 1 && b < 49))
	{
		if (temp->GetCellColor(a, b - 1) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI * 3 / 2);
		}
		if (temp->GetCellColor(a, b + 1) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI / 2);
		}
		if (temp->GetCellColor(a - 1, b) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI);
		}
		if (temp->GetCellColor(a + 1, b) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, 0);
		}
		if (temp->GetCellColor(a + 2, b + 2) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI / 4);
		}
		if (temp->GetCellColor(a - 2, b + 2) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI * 3 / 4);
		}
		if (temp->GetCellColor(a - 2, b - 2) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI * 5 / 4);
		}
		if (temp->GetCellColor(a + 2, b - 2) != temp->GetSelfTeamColor())
		{
			temp->Attack(100, PI * 7 / 4);
		}
	}
}
void run(double a[80][4], int b)
{
	int lu = 0, ru = 0, ld = 0, rd = 0, f = 0, ba = 0, bu = 0, bd = 0, t; double r, r2; //鍔狟砃
	std::vector <std::shared_ptr<const THUAI4::Wall>> vw = temp->GetWalls(); auto self = temp->GetSelfInfo();
	for (int i = 0; i < b; i++)
	{
		r = atan2(a[i][2] - self->y, a[i][1] - self->x);
		if (a[i][0] == 0)
		{
			break;
		}
		if (a[i][1] >= self->x && atan2(a[i][3], r) <= PI / 2)
		{
			bd++;
		}
		else if (a[i][1] < self->x && atan2(a[i][3], r) <= PI / 2)
		{
			bu++;
		}
	} //150
	if (b != 0)
	{
		for (int i = 0; i < b; i++)
		{
			r2 = sqrt(pow((a[i][1] + a[i][0] * cos(a[i][3]) * 0.05) - self->x, 2) + pow((a[i][2] + a[i][0] * sin(a[i][3]) * 0.05) - self->y, 2));
			if (r2 == (self->moveSpeed) * 0.05)
			{
				t = 45;
				break;
			}
			else
			{
				t = 45;
			}
		}
		if (bu > bd)
		{
			temp->MovePlayer(t, PI * 5 / 4);
		}
		else if (bd > bu)
		{
			temp->MovePlayer(t, PI / 4);
		}
		else
		{
			temp->MovePlayer(t, PI * 3 / 2);
		}
	}
	else
	{
		if (bu > bd)
		{
			temp->MovePlayer(50, PI * 5 / 4);
		}
		else if (bd > bu)
		{
			temp->MovePlayer(50, PI / 4);
		}
		else
		{
			temp->MovePlayer(50, PI * 3 / 2);
		}
	}
}
int pick()
{
	int Mo = 1; std::vector <std::shared_ptr<const THUAI4::Prop>> vp = temp->GetProps(); auto self = temp->GetSelfInfo();
	for (int i = 0; i < (int)vp.size(); i++)
	{
		if ((int)((unsigned char)vp[i]->propType) == 4)
		{
			if ((vp[i]->x == self->x) && (vp[i]->y == self->y))
			{
				temp->Throw(1000, 0);
				temp->Pick(vp[i]->propType);
				if (self->hp < 2500)
				{
					temp->Use();
				}
				Mo = 1;
			}
			else
			{ //250
				double angle = atan2((double)((vp[i]->y) - (self->y)), (double)((vp[i]->x) - (self->x)));
				if (angle < 0)
					angle += PI * 2;
				temp->MovePlayer(50, angle);
				Mo = 0;
			}
			break;
		}
		else if ((int)((unsigned char)vp[i]->propType) == 1 && (int)((unsigned char)self->propType) != 4)
		{
			if ((vp[i]->x == self->x) && (vp[i]->y == self->y))
			{
				temp->Throw(1000, 0);
				temp->Pick(vp[i]->propType);
				temp->Use();
				Mo = 1;
			}
			else
			{
				double ang = atan2((vp[i]->x) - (self->x), (vp[i]->y) - (self->y));
				if (ang < 0)
					ang += PI * 2;
				temp->MovePlayer(50, ang);
				Mo = 0;
			}
			break;
		}
		else if ((int)((unsigned char)vp[i]->propType) == 5 && ((int)((unsigned char)self->propType) != 4 && (int)((unsigned char)self->propType) != 1))
		{
			if ((vp[i]->x == self->x) && (vp[i]->y == self->y))
			{
				temp->Throw(1000, 0);
				temp->Pick(vp[i]->propType);
				temp->Use();
				Mo = 1;
			}
			else
			{
				double ang = atan2((vp[i]->x) - (self->x), (vp[i]->y) - (self->y));
				if (ang < 0)
					ang += 2 * PI;
				temp->MovePlayer(50, ang);
				Mo = 0;
			}
			break;
		}
		else if (((int)((unsigned char)vp[i]->propType) == 6 && (int)((unsigned char)self->propType) != 4) && ((int)((unsigned char)self->propType) != 1 && (int)((unsigned char)self->propType) != 5))
		{
			if ((vp[i]->x == self->x) && (vp[i]->y == self->y))
			{
				temp->Throw(1000, 0);
				temp->Pick(vp[i]->propType);
				Mo = 1;
			}
			else
			{
				double ang = atan2((vp[i]->x) - (self->x), (vp[i]->y) - (self->y));
				if (ang < 0)
					ang += 2 * PI;
				temp->MovePlayer(50, ang);
				Mo = 0;
			}
			break;
		}
		else if ((int)((unsigned char)vp[i]->propType) == 3 && (((int)((unsigned char)self->propType) != 4 && (int)((unsigned char)self->propType) != 1) && ((int)((unsigned char)self->propType) != 5 && (int)((unsigned char)self->propType) != 6)))
		{
			if ((vp[i]->x == self->x) && (vp[i]->y == self->y))
			{
				temp->Throw(1000, 0); //300
				temp->Pick(vp[i]->propType);
				temp->Use();
				Mo = 1;
			}
			else
			{
				double an = atan2((vp[i]->x) - (self->x), (vp[i]->y) - (self->y));
				if (an < 0)
					an += PI * 2;
				temp->MovePlayer(50, an);
				Mo = 0;
			}
			break;
		}
		else if ((vp[i]->x == self->x) && (vp[i]->y == self->y))
		{
			temp->Pick(vp[i]->propType);
			temp->Use();
			Mo = 1;
		}
		else
		{
			Mo = 1;
		}
	}
	return Mo;
}
void hide(double a[4][3])
{
	std::vector <std::shared_ptr<const THUAI4::BirthPoint>> vbp = temp->GetBirthPoints(); auto self = temp->GetSelfInfo();
	int BP[4][2]; double R[4];
	int e1 = 0, e2 = 0, e3 = 0, e4 = 0, n = 0, m, bp = 0, r1, ri, ms, an;
	for (int i = 0; i < 4; i++)
	{
		if (a[i][0] >= 0 && a[i][0] <= 6)
		{
			n++;
		}
	}
	if (n == 1)
	{
		if (a[1][0] != 4 && a[1][0] != 5)
		{
			temp->Attack(100, a[1][1]);
			temp->MovePlayer(50, a[1][1] + PI);//350
		}
		else
		{
			if (a[1][1] >= PI / 2)
			{
				e4 += 10000;
			}
			else if (a[1][1] < PI / 2 && a[1][1] >= 0)
			{
				e1 += 10000;
			}
			else if (a[1][1] < -PI / 2)
			{
				e3 += 10000;
			}
			else
			{
				e2 += 10000;
			}
			for (int i = 0; i < 1000; i++)
			{
				for (int j = 0; j < 1000; j++)
				{
					if (isWalls[i][j] == 1 && (((i * 1000) + 500) > self->x && ((j * 1000) + 500) > self->y))
					{
						e1++;//400
					}
					if (isWalls[i][j] == 1 && (((i * 1000) + 500) > self->x && ((j * 1000) + 500) < self->y))
					{
						e2++;
					}
					if (isWalls[i][j] == 1 && (((i * 1000) + 500) < self->x && ((j * 1000) + 500) < self->y))
					{
						e3++;
					}
					if (isWalls[i][j] == 1 && (((i * 1000) + 500) < self->x && ((j * 1000) + 500) > self->y))
					{
						e4++;
					}
				}
			}
			m = Minimum(e1, e2, e3, e4);
			if (m == e1)
			{
				temp->Attack(100, a[1][1]);
				temp->MovePlayer(50, PI / 4);
			}
			else if (m == e2)
			{
				temp->Attack(100, a[1][1]);
				temp->MovePlayer(50, PI * 7 / 4);
			}
			else if (m == e3)
			{
				temp->Attack(100, a[1][1]);
				temp->MovePlayer(50, PI * 5 / 4);
			}
			else
			{
				temp->Attack(100, a[1][1]);
				temp->MovePlayer(50, PI * 3 / 4);
			}
		}
	}
	else
	{
		for (int i = 0; i < n; i++)
		{
			if (a[i][1] >= PI / 2)
			{
				e4 += 10000;
			}
			else if (a[i][1] < PI / 2 && a[i][1] >= 0)
			{
				e1 += 10000;
			}
			else if (a[i][1] < -PI / 2)
			{
				e3 += 10000;
			}
			else
			{
				e2 += 10000;
			}
		}
		for (int i = 0; i < 1000; i++)
		{
			for (int j = 0; j < 1000; j++)
			{
				if (isWalls[i][j] == 1 && (((i * 1000) + 500) > self->x && ((j * 1000) + 500) > self->y))
				{
					e1++;
				}
				if (isWalls[i][j] == 1 && (((i * 1000) + 500) > self->x && ((j * 1000) + 500) < self->y))
				{
					e2++;
				}
				if (isWalls[i][j] == 1 && (((i * 1000) + 500) < self->x && ((j * 1000) + 500) < self->y))
				{
					e3++;
				}//500
				if (isWalls[i][j] == 1 && (((i * 1000) + 500) < self->x && ((j * 1000) + 500) > self->y))
				{
					e4++;
				}
			}
		}
		m = Minimum(e1, e2, e3, e4);
		if (m == e1)
		{
			temp->Attack(100, a[1][1]);
			temp->MovePlayer(50, PI / 4);
		}
		else if (m == e2)
		{
			temp->Attack(100, a[1][1]);
			temp->MovePlayer(50, PI * 7 / 2);
		}
		else if (m == e3)
		{
			temp->Attack(100, a[1][1]);
			temp->MovePlayer(50, PI * 5 / 4);
		}
		else
		{
			temp->Attack(100, a[1][1]);
			temp->MovePlayer(50, PI * 3 / 4);
		}
	}
}
int Minimum(int a, int b, int c, int d)
{
	int m;
	m = a;
	if (m > b)
	{
		m = b;
	}
	if (m > c)
	{
		m = c;
	}
	if (m > d)
	{
		m = d;
	}
	return m;
}
void addWalls()
{
	int n = 0;//550
	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			if (walls[i][j] == 1 && n < 4)
			{
				isWalls[i][j] = walls[i][j];
				walls[i][j] = 0;
				n++;
			}
			else
			{
				break;
			}
		}
		if (n >= 4)
		{
			break;
		}
	}
}
inline int getTime() {
	return (1ll * std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count()) - start_time;
}
void message()
{
	auto self = temp->GetSelfInfo(); std::vector <std::shared_ptr<const THUAI4::Character>> ve = temp->GetCharacters();
	char sn; int a = 0, b = 0, d, t; char s[2][3], el[4][4], ej[4], ed[4][2], ev[4][2], wl[4][2], ti[3], c = '0';
	std::string ms, E = "MAYDAY";
	s[0][0] = (int)((self->x) / 65536);
	s[0][1] = (int)(((self->x) % 65536) / 256);
	s[0][2] = (int)((self->x) % 256);
	s[1][0] = (int)((self->y) / 65536);
	s[1][1] = (int)(((self->y) % 65536) / 256);
	s[1][2] = (int)((self->y) % 256);
	for (int i = 0; i < (int)ve.size(); i++)
	{
		if (ve[i]->teamID != self->teamID)
		{
			ej[a] = (int)((unsigned)ve[i]->jobType);
			el[a][1] = (int)((ve[i]->x) - (self->x) % 256) - 128;
			el[a][0] = (int)((ve[i]->x) - (self->x) / 256) - 128;
			el[a][2] = (int)((ve[i]->y) - (self->y) / 256) - 128;
			el[a][3] = (int)((ve[i]->y) - (self->y) % 256) - 128;
			ed[a][0] = (int)(ve[i]->facingDirection) / degree;
			ed[a][1] = (int)((ve[i]->facingDirection) - (int)(ve[i]->facingDirection) / degree) / 256 - 128;
			ev[a][0] = (int)(ve[i]->moveSpeed) / 256 - 128;
			ev[a][1] = (int)(ve[i]->moveSpeed) % 256 - 128;//600
			a++;
		}
	}
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 1000; i++)
		{
			while (b < 4)
			{
				if (walls[i][j] == 1)
				{
					wl[b][0] = (int)(i / 1000);
					wl[b][1] = (int)(j / 1000);
					b++;
				}
			}
		}
		if (b >= 4)
		{
			break;
		}
	}
	for (int i = a; i < 4; i++)
	{
		ej[a] = 0;
		for (int j = 0; j < 4; j++)
		{
			el[i][j] = 0;
		}
		for (int j = 0; j < 2; j++)
		{
			ed[i][j] = 0;
			ev[i][j] = 0;
		}
	}
	for (int i = b; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			wl[i][j] = 0;
		}
	}
	t = getTime(); sn = (int)((unsigned char)self->jobType);
	ti[0] = (int)(t / 65536); ti[1] = (int)((t % 65536) / 256); ti[2] = (int)(t % 256);
	ms = sn;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ms.append(1, s[i][j]);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		ms.append(1, ej[i]);
		for (int j = 0; j < 4; j++)
		{
			ms.append(1, el[i][j]);
		}
		for (int j = 0; j < 2; j++)
		{
			ms.append(1, ed[i][j]);
		}
		for (int j = 0; j < 2; j++)
		{
			ms.append(1, ev[i][j]);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		ms.append(1, ti[i]);
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			ms.append(1, wl[i][j]);
		}
	}
	if (self->hp == 0)
	{
		for (int i = 0; i < 6; i++)
		{
			ms.append(1, c);
		}
	}
	else if (a > 1)
	{
		ms.append(E);
	}
	ms.append("\0");
	for (int i = 0; i < 4; i++)
	{
		temp->Send(i, ms);
	}
}
void getMes()
{
	auto self = temp->GetSelfInfo();
	int a, b, c, d, f, g; double e;
	for (int i = 0; i < 4; i++)//700
	{
		std::string msg;
		if (temp->TryGetMessage(msg))
		{
			if (msg[0] == ((int)self->jobType))
			{
				continue;
			}
			else
			{
				a = msg[1] * 65536 + msg[2] * 256 + msg[3];
				b = msg[4] * 65536 + msg[5] * 256 + msg[6];
				c = a - self->x; d = b - self->y;
				e = atan2(d, c);
				if (e < 0)
					e += PI * 2;
				temp->Attack(100, e);
				for (int j = 47; j < 55; j + 2)
				{
					f = (int)msg[j]; g = (int)msg[j + 1];
					if (isWalls[f][g] != 1)
					{
						isWalls[f][g] = 1;
						walls[f][g] = 0;
					}
				}
			}
		}
		else
		{
			break;
		}
	}
}