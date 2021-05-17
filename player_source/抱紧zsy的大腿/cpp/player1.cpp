//猴博士 Player0
#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>
#include <thread>
#include <vector>
#include <ctime>
#include <cmath>
#include <chrono>
#include <string>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job4; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{std::random_device{}()};
}
using namespace std;
//定义数据结构
class Location
{
public:
    int x;
    int y;
    Location(int x = -1, int y = -1) : x(x), y(y) {}
    bool operator==(const Location &A) const
    {
        if (abs(x - A.x) < 5 && abs(y - A.y) < 5)
            return true;
        else
            return false;
    }
};
#include <iostream>
#include <string>
using namespace std;

class message
{
public:
    string msgString; //消息字符串
    int selection;    // 0-6代表敌人职业，7代表涂色目标，8代表子弹充足
    int x, y, hp;

public:
    message(int selection = 0, int X = 0, int Y = 0, int HP = 0) : selection(selection), x(X), y(Y), hp(HP) //以角色信息初始化
    {
        info2str();
    }
    message(string str) :msgString(str)  //以通信字符串初始化
	{
		str2info();
	}
	void info2str()  //产生通讯字符串
	{
		msgString = "000000000000000000000000000000000000000000000000000000000000000";
		msgString[2] = (char)(selection + '0');
		itos(x, 7);
		itos(y, 12);
		itos(hp, 17);
	}
	void str2info()  //从通讯字符串获取信息
	{
		string type = msgString.substr(0, 2);
		if (msgString.size() != 63)
		{
			selection = -1;
			x = y = hp = 0;
			return;
		}
		if (type == "00")
		{
			selection = (int)(msgString[2] - '0');
			x = stoi(msgString.substr(3, 5));
			y = stoi(msgString.substr(8, 5));
			hp = stoi(msgString.substr(13, 5));
		}
	}
	void sendMsg(GameApi &g,int toPlayer)
	{
		info2str();
		g.Send(toPlayer, msgString);
	}
	bool receiveMsg(GameApi &g)
	{
		if (g.MessageAvailable())
		{
			g.TryGetMessage(msgString);
			str2info();
			return 1;
		}
		else return 0;
	}
private:
	void itos(int i,int tailPos)
	{
		while (i != 0)
		{
			msgString[tailPos] = (char)(i % 10 + '0');
			i /= 10;
			tailPos--;
		}
	}
};
namespace cd // ms
{
    const int attack_cd = 500;
    long long attack_lasttime = 0;
    const int sendmsg_empty_color_cd = 2000;
    long long sendmsg_empty_color_lasttime = 0;
    const int sendmsg_enemy_cd = 1000;
    long long sendmsg_enemy_lasttime = 0;
}

//定义常量及变量
const double PI = 3.1415926;
const int NAVIGATING = 1;
const int REPLENISHING = 2;
const int SHOOTING = 3;
const int PICKING = 4;
const int AVOIDENEMY = 5;
const int AVOIDBULLET = 6;
const int AVOIDWALL = 7;
const int SUPPLYING = 8;
const int print_controller_parameter = 50;
const int bullet_speed = 18000;
const int player_num = 4;
int deadlock = 0;
bool is_init = 0; //初始化变量
bool game_begin = 0;
bool has_init_avoid_walls_list = 0;
int current_state = 1;
int last_state = 0;
int print_controller = 0;
bool shoot_flag = 0;
int Maxbulletnum;         //最大子弹数目
bool issupplying = false; //正在补给flag
bool supplyFlag = false;  //缺弹lag
bool avaliable_player[4];

string buffer;
vector<Location> destination_list;
int des_number = 0;
Location last_location;
Location now_location;
Location enemy_location;
Location supply_location;
Location prop_location;
vector<Location> color_block_list;
vector<Location> avoid_wall_list;
vector<Location> wall[12];
THUAI4::PropType proptype;
THUAI4::ColorType selfcolor;
int wall_index = 0;
int debug = 1; //0:无输出; 1：简单输出; 2：输出行动方向; 3:输出墙的位置
//函数定义
void init(GameApi &g)
{
    auto self = g.GetSelfInfo();
    if (is_init == 1)
        return;
    while(g.TryGetMessage(buffer));
    is_init = 1;
}
bool shoot_at(GameApi &g, int des_x, int des_y)
{
    auto self = g.GetSelfInfo();
    int bulletnum = self->bulletNum;
    if (bulletnum)
    {
        cout << "Shoot at (" << des_x << "," << des_y << "), bullet: " << bulletnum - 1 << "/" << Maxbulletnum << endl;
        long long my_x = self->x;
        long long my_y = self->y;
        double distance = sqrt(((my_x - (long long)des_x) * (my_x - (long long)des_x) + (my_y - (long long)des_y) * (my_y - (long long)des_y)));
        double angle = atan2(des_y - my_y, des_x - my_x);
        angle = angle >= 0 ? angle : 2 * PI + angle;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        g.Attack(int(distance * 1000 / bullet_speed), angle);
        return true;
    }
    else
    {
        cout << "Shoot at (" << des_x << "," << des_y << ") failed, bullet: " << bulletnum  << "/" << Maxbulletnum << endl;
        return false;
    }
}
void receive_msg(GameApi &g)
{
    if (g.MessageAvailable())
    {
        while (g.TryGetMessage(buffer))
        {
            message *msg = new message(buffer);
            cout << buffer <<endl;
            auto self = g.GetSelfInfo();
            int bulletnum = self->bulletNum;
            cout << buffer << endl;
            if (msg->selection <= 6)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (bulletnum >= 1)
                    {
                        shoot_at(g, msg->x, msg->y);
                        bulletnum--;
                        if (bulletnum <= 1)
                            supplyFlag = true;
                        else
                            supplyFlag = false;
                    }
                    else
                    {
                        supplyFlag = true;
                        break;
                    }
                    msg->hp -= 3000;
                    if (msg->hp <= 0)
                        break;
                }
            }
            delete msg;
        }
    }
}
void get_enemy(GameApi &g)
{
    auto self = g.GetSelfInfo();
    auto cha = g.GetCharacters();
    for (int i = 0; i < g.GetCharacters().size(); i++)
    {
        if (cha[i]->teamID != self->teamID && !cha[i]->isDying)
        {
            shoot_at(g, cha[i]->x, cha[i]->y);
        }
    }
}
void AI::play(GameApi &g)
{
    //初始位置 2500,3500
    auto self = g.GetSelfInfo();
    if (self->isDying)
    {
            cout << "Dying" << endl;
        while (g.TryGetMessage(buffer))
        {
        }
        is_init = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return;
    }
    init(g);
    receive_msg(g);
    get_enemy(g);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
