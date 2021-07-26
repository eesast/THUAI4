//贺岁师 player3
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

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业

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
    const int sendmsg_avaliable_cd = 2000;
    long long sendmsg_avaliable_lasttime = 0;
    const int sendmsg_colorblock_cd = 3000;
    long long sendmsg_colorblock_lasttime = 0;
    const int sendmsg_enemy_cd = 50;
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
const int bullet_speed = 12000;
const int player_id = 3;

int deadlock = 0;
bool is_init = 0; //初始化变量
bool game_begin = 0;
bool has_init_avoid_walls_list = 0;
int current_state = 1;
int last_state = 0;
int print_controller = 0;
int Maxbulletnum;         //最大子弹数目
bool issupplying = false; //正在补给flag
bool supplyFlag = false;  //缺弹lag

string buffer;
Location scheduled_destination[] = { // player 3 team 0
    Location(4500,44500),
    Location(10500, 44500),
    Location(10500, 34500),
    Location(16500, 34500),
    Location(19500, 37500),
    Location(21500, 32500),
    Location(44500, 33500),
    Location(44500, 44500),
    Location(39500, 44500),
    Location(39500, 33500),
    Location(31500, 33500),
    Location(31500, 46500),
    Location(19500, 42500),
    Location(17500, 48000),
    Location(10500, 48000)
};
vector<Location> destination_list;
int des_number = 0;
Location last_location;
Location now_location;
Location enemy_location;
Location supply_location;
Location prop_location;
vector<Location> avoid_wall_list;
vector<Location> wall[12];
THUAI4::PropType proptype;
THUAI4::ColorType selfcolor;
int wall_index = 0;
int debug = 1; //0:无输出; 1：简单输出; 2：输出行动方向; 3:输出墙的位置
//函数定义
bool shoot_at(GameApi &g, int des_x, int des_y);
void init(GameApi &g)
{
    auto self = g.GetSelfInfo();
    if (is_init == 1)
        return;
    destination_list.clear();
    avoid_wall_list.clear();
    if (self->teamID == 0)
        des_number = 0;
    else if (self->teamID == 1)
        des_number = 7;
    destination_list.push_back(scheduled_destination[des_number]);
    current_state = NAVIGATING;
    last_state = 0;
    deadlock = 0;
    issupplying = supplyFlag = false;
    has_init_avoid_walls_list = false;
    last_location.x = last_location.y = enemy_location.x = enemy_location.y =
        prop_location.x = prop_location.y = supply_location.x = supply_location.y = -1;
    cout << "init add(" << scheduled_destination[0].x << "," << scheduled_destination[0].y << ")" << endl;
    if (game_begin == 0)
    {
        wall[1].push_back(Location(20250, 10750));
        wall[1].push_back(Location(29750, 10750));
        wall[2].push_back(Location(5250, 10250));
        wall[2].push_back(Location(19750, 10250));
        wall[2].push_back(Location(19750, 12750));
        wall[2].push_back(Location(5250, 12750));
        wall[3].push_back(Location(2250, 21250));
        wall[3].push_back(Location(4750, 21250));
        wall[3].push_back(Location(4750, 26750));
        wall[3].push_back(Location(2250, 26750));
        wall[4].push_back(Location(6250, 24250));
        wall[4].push_back(Location(8750, 24250));
        wall[4].push_back(Location(8750, 29750));
        wall[4].push_back(Location(6250, 29750));
        wall[5].push_back(Location(13250, 35250));
        wall[5].push_back(Location(15750, 35250));
        wall[5].push_back(Location(15750, 47750));
        wall[5].push_back(Location(13250, 47750));
        wall[6].push_back(Location(21250, 37250));
        wall[6].push_back(Location(28750, 37250));
        wall[6].push_back(Location(28750, 40750));
        wall[6].push_back(Location(21250, 40750));
        wall[7].push_back(Location(34250, 35250));
        wall[7].push_back(Location(36750, 35250));
        wall[7].push_back(Location(36750, 47750));
        wall[7].push_back(Location(34250, 47750));
        wall[8].push_back(Location(44250, 26250));
        wall[8].push_back(Location(46750, 26250));
        wall[8].push_back(Location(46750, 31750));
        wall[8].push_back(Location(44250, 31750));
        wall[9].push_back(Location(40250, 23250));
        wall[9].push_back(Location(42750, 23250));
        wall[9].push_back(Location(42750, 28750));
        wall[9].push_back(Location(40250, 28750));
        wall[10].push_back(Location(26250, 19250));
        wall[10].push_back(Location(26250, 21750));
        wall[10].push_back(Location(33750, 21750));
        wall[10].push_back(Location(33750, 14250));
        wall[10].push_back(Location(31250, 14250));
        wall[11].push_back(Location(23750, 27250));
        wall[11].push_back(Location(23750, 29750));
        wall[11].push_back(Location(18750, 34750));
        wall[11].push_back(Location(16250, 34750));
        wall[11].push_back(Location(16250, 27250));
        game_begin = 1;
        Maxbulletnum = self->maxBulletNum;
        selfcolor = g.GetSelfTeamColor();
        for (int i = 0; i < 10; i++)
        {
            shoot_at(g, 8000 + i * 3000, 23000);
        }
    }
    is_init = 1;
}
void move_to(GameApi &g, int des_x, int des_y)
{
    auto self = g.GetSelfInfo();
    int my_x = self->x;
    int my_y = self->y;
    double angle = atan2(des_y - my_y, des_x - my_x);
    angle = angle >= 0 ? angle : 2 * PI + angle;
    int time = sqrt(((long long)des_x - (long long)my_x) * ((long long)des_x - (long long)my_x) + 
    ((long long)des_y - (long long)my_y) * ((long long)des_y - (long long)my_y)) * 1000 / (int)self->moveSpeed;
    time = time >= 50 ? 50 : time;
    cout << "move to (" << des_x << "," << des_y << ")" << endl;
    g.MovePlayer(time, angle);
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}
bool is_neared(int my_x, int my_y, int des_x, int des_y, int distance)
{
    return (abs(des_x - my_x) <= distance && abs(des_y - my_y) <= distance);
}
int index_of_wall(int wallx, int wally)
{
    if (wallx < 1000 || wallx > 49000 || wally < 1000 || wally > 49000)
        return 0;
    if (wallx > 21000 && wallx < 29000 && wally < 10000 && wally > 2000)
        return 1;
    if (wally == 11500)
        return 2;
    if (wallx == 3500)
        return 3;
    if (wallx == 7500 && wally > 25000 && wally < 29000)
        return 4;
    if (wallx == 14500 && wally > 36000 && wally < 47000)
        return 5;
    if (wallx > 22000 && wallx < 28000 && wally > 38000 && wally < 40000)
        return 6;
    if (wallx == 35500)
        return 7;
    if (wallx == 45500)
        return 8;
    if (wallx == 41500)
        return 9;
    if (wally > 15000 && wally < 21000)
        return 10;
    if (wallx > 17000 && wallx < 23000 && wally > 28000 && wally < 34000)
        return 11;
    return 0;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return true;
    }
    else
    {
        cout << "Shoot at (" << des_x << "," << des_y << ") failed, bullet: " << bulletnum - 1 << "/" << Maxbulletnum << endl;
        return false;
    }
}
bool is_accessible(GameApi &g, int x1, int y1, int x2, int y2)
{
    auto self = g.GetSelfInfo();
    auto walls = g.GetWalls();
    auto birth_points = g.GetBirthPoints();
    auto characters = g.GetCharacters();
    int counter = 0;
    if (abs(x1 - x2) > 10000 || abs(y1 - y2) > 10000)
    {
        cout << "is_accessible: Can't judge" << endl;
        return false;
    }
    double distance = sqrt(((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
    double angle = atan2(y2 - y1, x2 - x1);
    angle = angle >= 0 ? angle : 2 * PI + angle;
    int wantx = x1;
    int wanty = y1;
    while (1)
    {

        for (int i = 0; i < walls.size(); i++)
        {
            if (is_neared(wantx, wanty, walls[i]->x, walls[i]->y, 1500))
                return false;
        }
        for (int i = 0; i < birth_points.size(); i++)
        {
            if (is_neared(wantx, wanty, birth_points[i]->x, birth_points[i]->y, 1500))
                return false;
        }
        for (int i = 0; i < characters.size(); i++)
        {
            if (characters[i]->guid == self->guid)
                continue;
            if (is_neared(wantx, wanty, characters[i]->x, characters[i]->y, 1500))
                return false;
        }
        counter++;
        if (debug >= 2)
            cout << "is_accessible: has detected " << counter << " cell" << endl;
        if (counter * 1000 > distance)
            return true;
        else
        {
            wantx = wantx + int(cos(angle) * 1000);
            wanty = wanty + int(sin(angle) * 1000);
        }
    }
}
bool error_area(int x, int y)
{
    if ((x <= 4000 && y <= 4000) || (x <= 4000 && y >= 46000) || (x >= 46000 && y <= 4000) || (x >= 46000 && y >= 46000))
        return true;
    return false;
}
void solve_deadlock(GameApi &g)
{
    bool deadlock_flag = 0;
    auto self = g.GetSelfInfo();
    auto characters = g.GetCharacters();
    auto walls = g.GetWalls();
    auto birthpoints = g.GetBirthPoints();
    now_location.x = self->x;
    now_location.y = self->y;
    double angle;
    bool BLOCK[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int index;
    int nowx = now_location.x;
    int nowy = now_location.y;
    int move_index = nowx % 18;
    auto GetDistance = [nowx, nowy](int x, int y) { return sqrt((nowx - x) * (nowx - x) + (nowy - y) * (nowy - y)); };
    if (now_location.x == last_location.x && now_location.y == last_location.y)
    {
        auto now_color = g.GetCellColor(now_location.x / 1000, now_location.y / 1000);
        if (current_state == SUPPLYING && issupplying && int(selfcolor) == int(now_color))
        {
            deadlock = 0;
        }
        else
            deadlock++;
        if (deadlock >= 30)
        {
            cout << "DEADLOCK!" << endl;
            deadlock_flag = 1;
            current_state = NAVIGATING;
            deadlock -= 20;
        }
        if (debug >= 1)
            cout << "deadlock: " << deadlock << endl;
    }
    else
    {
        deadlock = 0;
    }
    if (deadlock_flag)
    {

        for (int i = 0; i < walls.size(); i++)
        {
            if (GetDistance(int(walls[i]->x), int(walls[i]->y)) <= 2000)
            {
                angle = atan2(int(walls[i]->y) - nowy, int(walls[i]->x) - nowx);
                angle = angle >= 0 ? angle : 2 * PI + angle;
                index = int(angle * 18 / (2 * PI)) % 18;
                BLOCK[index] = BLOCK[(index + 1) % 18] = BLOCK[(index + 17) % 18] = BLOCK[(index + 2) % 18] = BLOCK[(index + 16) % 18] = 1;
                if (debug >= 2)
                    cout << "wall: (" << walls[i]->x << "," << walls[i]->y << "BLOCK: " << (index + 16) % 18 << ","
                         << (index + 17) % 18 << "," << index << "," << (index + 1) % 18 << (index + 2) % 18 << "," << endl;
            }
        }
        for (int i = 0; i < birthpoints.size(); i++)
        {
            if (GetDistance(int(birthpoints[i]->x), int(birthpoints[i]->y)) <= 2000)
            {
                angle = atan2(int(birthpoints[i]->y) - nowy, int(birthpoints[i]->x) - nowx);
                angle = angle >= 0 ? angle : 2 * PI + angle;
                index = int(angle * 18 / (2 * PI)) % 18;
                BLOCK[index] = BLOCK[(index + 1) % 18] = BLOCK[(index + 17) % 18] = BLOCK[(index + 2) % 18] = BLOCK[(index + 16) % 18] = 1;
                if (debug >= 2)
                    cout << "wall: (" << birthpoints[i]->x << "," << birthpoints[i]->y << "BLOCK: " << (index + 16) % 18 << ","
                         << (index + 17) % 18 << "," << index << "," << (index + 1) % 18 << (index + 2) % 18 << "," << endl;
            }
        }
        for (int i = 0; i < characters.size(); i++)
        {
            if (characters[i]->guid == self->guid)
                continue;
            if (characters[i]->teamID == self->teamID && GetDistance(int(characters[i]->x), int(characters[i]->y)) < 1500)
            {
                //可能是撞到队友了
                angle = atan2(int(characters[i]->y) - nowy, int(characters[i]->x) - nowx);
                angle = angle >= 0 ? angle : 2 * PI + angle;
                index = int(angle * 18 / (2 * PI))  % 18;
                for (int j = 0; j < 6; j++)
                {
                    BLOCK[(index + j) % 18] = BLOCK[(index + 18 - j) % 18] = 1;
                }
                BLOCK[(index + 9) % 18] = 1;
            }
            else if (GetDistance(int(characters[i]->x), int(characters[i]->y)) < 2000)
            {
                angle = atan2(int(characters[i]->y) - nowy, int(characters[i]->x) - nowx);
                angle = angle >= 0 ? angle : 2 * PI + angle;
                index = int(angle * 18 / (2 * PI))  % 18;
                BLOCK[index] = BLOCK[(index + 1) % 18] = BLOCK[(index + 17) % 18] = BLOCK[(index + 2) % 18] = BLOCK[(index + 16) % 18] = 1;
                if (debug >= 2)
                    cout << "wall: (" << characters[i]->x << "," << characters[i]->y << "BLOCK: " << (index + 16) % 18 << ","
                         << (index + 17) % 18 << "," << index << "," << (index + 1) % 18 << (index + 2) % 18 << "," << endl;
            }
        }
        for (int i = 0; i < 18; i++)
        {
            if (BLOCK[i] == false)
            {
                move_index = i;
                break;
            }
        }
        cout << "move 1500 at " << move_index * 2 * PI / 18 << endl;
        g.MovePlayer(1500 * 1000 / self->moveSpeed, move_index * 2 * PI / 18);
        std::this_thread::sleep_for(std::chrono::milliseconds(1500 * 1000 / self->moveSpeed));
        deadlock_flag = 0;
    }
}
void sendmsg_avaliable(GameApi &g, int to_player)
{
    int bulletnum = g.GetSelfInfo()->bulletNum;
    int selection = (bulletnum >= 2) ? 8 : 9;
    message *msg = new message(selection, 0, 0, player_id);
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); //获取时间
    if (msec - cd::sendmsg_avaliable_lasttime >= cd::sendmsg_avaliable_cd)
    {
        msg->sendMsg(g, to_player);
        cd::sendmsg_avaliable_lasttime = msec;
    }
    delete msg;
}
void sendmsg_getenemy(GameApi &g, int to_player)
{
    auto self = g.GetSelfInfo();
    auto cha = g.GetCharacters();
    for (int i = 0; i < g.GetCharacters().size(); i++)
    {
        if (cha[i]->teamID != self->teamID && !cha[i]->isDying)
        {
            auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); //获取时间
            if (msec - cd::sendmsg_enemy_lasttime >= cd::sendmsg_enemy_cd)
            {
                cd::sendmsg_enemy_lasttime = msec;
                auto job = cha[i]->jobType;
                int enejob;
                if (job == THUAI4::JobType::OrdinaryJob)
                    enejob = 0;
                else if (job == THUAI4::JobType::HappyMan)
                    enejob = 1;
                else if (job == THUAI4::JobType::LazyGoat)
                    enejob = 2;
                else if (job == THUAI4::JobType::PurpleFish)
                    enejob = 3;
                else if (job == THUAI4::JobType::MonkeyDoctor)
                    enejob = 4;
                else if (job == THUAI4::JobType::EggMan)
                    enejob = 5;
                else
                    enejob = 6;
                message *msg = new message(enejob, cha[i]->x, cha[i]->y, cha[i]->hp);
                msg->sendMsg(g, to_player);
                delete msg;
            }
        }
    }
}
bool find_enemy(GameApi& g) //发现敌人直接开枪
{
    bool enemyflag = false;
    auto self = g.GetSelfInfo();
    auto cha = g.GetCharacters();
    for (int i = 0; i < g.GetCharacters().size(); i++)
    {
        if (cha[i]->teamID != self->teamID && !cha[i]->isDying)
        {
            auto bulletnum = self->bulletNum;
            auto hp = cha[i]->hp;
            auto enemyx = cha[i]->x;
            auto enemyy = cha[i]->y;
            int bulletneeded = hp / 750;
            int count = bulletneeded > bulletnum ? bulletneeded : bulletnum;
            for (int k = 0; k < count; k++)
            {
                shoot_at(g, enemyx, enemyy);
                enemyflag = true;
            }
        }
    }
    if (enemyflag) return true;
    return false;
}
void receive_msg(GameApi &g)
{
    if (g.MessageAvailable())
    {
        while (g.TryGetMessage(buffer))
        {
            message *msg = new message(buffer);
            cout << buffer << endl;
            auto self = g.GetSelfInfo();
            if (msg->selection == 7)
            {
                if (self->bulletNum >= 2)
                {
                    cout << "Shoot at (" << msg->x << "," << msg->y << ")" << endl;
                    shoot_at(g, msg->x, msg->y);
                    if (self->bulletNum-1 <= 1)
                    {
                        cout << "bullet: " << self->bulletNum << "/" << Maxbulletnum << endl;
                        supplyFlag = true;
                    }
                    else
                        supplyFlag = false;
                }
                delete msg;
            }
        }
    }
}
bool find_colorblock(GameApi &g)
{
    static pair<int,int> last_find_location;
    auto self = g.GetSelfInfo();
    if (is_neared(self->x, self->y, last_find_location.first * 1000 + 500, last_find_location.second * 1000 + 500, 1000))
        return false;
    last_find_location = make_pair(self->x / 1000, self->y / 1000);
    auto _wall = g.GetWalls();
    int save_count = 0;
    pair<int, int> save_location = make_pair(100,100);
    int state[9][9];// 1代表墙，2代表己方颜色，3代表非己方颜色，4代表不可见
    for (auto k: _wall)
    {
        int x1 = k->x/1000 - last_find_location.first;
        int y1 = k->y/1000 - last_find_location.second;
        if (abs(x1) <= 4 && abs(y1) <= 4)
        {
            state[x1+4][y1+4] = 1;
        }
    }
    auto _birthpoint = g.GetBirthPoints();
    for (auto k : _birthpoint)
    {
        int x1 = k->x / 1000 - last_find_location.first;
        int y1 = k->y / 1000 - last_find_location.second;
        if (abs(x1) <= 4 && abs(y1) <= 4)
        {
            state[x1 + 4][y1 + 4] = 5;
        }
    }
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (state[i][j] == 1 || state[i][j] == 5)
                continue;
            else
            {
                auto color = g.GetCellColor(last_find_location.first + i - 4, last_find_location.second + j - 4);
                if (int(color) == int(selfcolor))
                    state[i][j] = 2;
                else if (int(color) == 5)
                    state[i][j] = 4;
                else
                    state[i][j] = 3;
            }
        }
    }
    for (int i = -3; i <= 3; i++)
    {
        for (int j = -3; j <= 3; j++)
        {
            int count = 0;
            for (int m = -1; m <= 1; m++)
            {
                for (int n = -1; n <= 1; n++)
                {
                    if (state[i + 4 + m][j + 4 + n] == 3)
                        count++;
                }
            }
            int min_colorblock = 7 - g.GetTeamScore() / 300;
            if (count > max(save_count, max(min_colorblock, 2)))
            {
                for (auto k : _wall)
                {
                    if (k->x / 1000 == last_find_location.first + i && k->y / 1000 == last_find_location.second + j)
                        return false;
                }
                save_count = count;
                save_location = make_pair(last_find_location.first + i, last_find_location.second + j);
            }
        }
    }
    if (save_location.first < 100)
    {
        cout << "find a empty color block at (" << save_location.first << "," << save_location.second << ")" << endl;
        if (self->bulletNum)
        {
            shoot_at(g, save_location.first * 1000 + 500, save_location.second * 1000 + 500);
            if (self->bulletNum - 1 <= 1)
            {
                cout << "bullet: " << self->bulletNum - 1 << "/" << Maxbulletnum << endl;
                supplyFlag = true;
            }
            else
                supplyFlag = false;
            return true;
        }
        /*else if (save_count >= 6)
        {
            auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); //获取时间
            if (msec - cd::sendmsg_colorblock_lasttime >= cd::sendmsg_colorblock_cd)
            {
                cd::sendmsg_colorblock_lasttime = msec;
                message *msg1 = new message(9, 0, 0, player_id);
                msg1->sendMsg(g, 0);
                delete msg1;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                message* msg2 = new message(7,save_location.first*1000+500, save_location.second*1000+500,0);
                msg2->sendMsg(g, 0);
                delete msg2;
            }
        }*/
    }
    return false;
}
void AI::play(GameApi &g)
{
    //初始位置 2500,3500
    auto self = g.GetSelfInfo();
    auto walls = g.GetWalls();
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); //获取时间
    if (self->isDying)
    {
        if (debug >= 1)
            cout << "Dying" << endl;
        while (g.TryGetMessage(buffer))
        {
        }
        is_init = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return;
    }
    init(g);
    now_location.x = self->x;
    now_location.y = self->y; //当前位置
    //信息处理
    /*if (find_colorblock(g))
    {
        while (g.TryGetMessage(buffer))
        {
        }
        return;
    }*/
    //receive_msg(g);
    //sendmsg_avaliable(g, 0);
    if (find_enemy(g))
    {
        sendmsg_getenemy(g, 0);
        return; //如果发现敌人向敌人连发然后双手离开键盘
    }
    find_colorblock(g);

    //捡道具
    if (current_state == NAVIGATING)
    {
        auto props = g.GetProps();
        for (int i = 0; i < props.size(); i++)
        {
            if (is_accessible(g, now_location.x, now_location.y, props[i]->x, props[i]->y))
            {
                if (error_area(props[i]->x, props[i]->y))
                    continue;
                prop_location.x = props[i]->x;
                prop_location.y = props[i]->y;
                proptype = props[i]->propType;
                last_state = current_state;
                current_state = PICKING;
                cout << "NAVIGATING -> PICKING" << endl;
                break;
            }
        }
        if (current_state != PICKING)
            prop_location.x = prop_location.y = -1;
    }
    //补弹逻辑
    if (supplyFlag && current_state == NAVIGATING)
    {
        if (self->bulletNum > 0)
            current_state = SUPPLYING;
        else
        {
            for (int i = -2; i < 3; i++) //找染色区
            {
                if (now_location.x / 1000 + i < 0 || now_location.x / 1000 + i > 49)
                    continue;
                for (int j = -2; j < 3; j++) //搜索一个5x5的区域
                {
                    if (now_location.y / 1000 + j < 0 || now_location.y / 1000 + j > 49)
                        continue;
                    auto areacolor = g.GetCellColor(now_location.x / 1000 + i, now_location.y / 1000 + j);
                    if (int(selfcolor) == int(areacolor) && is_accessible(g, now_location.x, now_location.y,
                                                                          (now_location.x / 1000 + i) * 1000 + 500, (now_location.y / 1000 + j) * 1000 + 500)) //匹配成功
                    {
                        supply_location.x = (now_location.x / 1000 + i) * 1000 + 500;
                        supply_location.y = (now_location.y / 1000 + j) * 1000 + 500;
                        break;
                    }
                }
            }
            if (!error_area(supply_location.x, supply_location.y))
                current_state = SUPPLYING;
            else //没找到染色区而且没子弹
            {
                supply_location.x = supply_location.y = -1;
            }
        }
    }
    if (current_state == NAVIGATING)
    {
        cout << "NAVIGATING" << endl;
        auto next_location = destination_list.back();
        if (is_neared(now_location.x, now_location.y, next_location.x, next_location.y, self->moveSpeed / 30))
        {
            destination_list.pop_back();
            if (destination_list.size() == 0)
            {
                current_state = NAVIGATING;
                des_number = (des_number + 1) % (sizeof(scheduled_destination) / sizeof(scheduled_destination[0]));
                destination_list.push_back(scheduled_destination[des_number]);
            }
        }
        else
        {
            move_to(g, next_location.x, next_location.y);
            self = g.GetSelfInfo();
            now_location.x = self->x;
            now_location.y = self->y;
            if (last_location == now_location)
            {
                for (int i = 0; i < walls.size(); i++)
                {
                    int r = walls[i]->radius;
                    if (is_neared(now_location.x, now_location.y, walls[i]->x, walls[i]->y, 3 * r))
                    {
                        current_state = AVOIDWALL;
                        last_state = NAVIGATING;
                        wall_index = index_of_wall(walls[i]->x, walls[i]->y);
                        if (debug >= 1)
                        {
                            if (wall_index > 0)
                                cout << "Meeting NO." << wall_index << " walls" << endl;
                            else
                                cout << "Meeting the bounds" << endl;
                        }
                        break;
                    }
                }
            }
        }
    }
    else if (current_state == PICKING)
    {
        auto props = g.GetProps();
        if (error_area(prop_location.x, prop_location.y) || props.size() == 0)
        {
            current_state = last_state;
            prop_location.x = prop_location.y = -1;
            cout << "Error prop location. PICKING -> " << last_state << endl;
        }
        else
        {
            cout << "PICKING proplocation: (" << prop_location.x << "," << prop_location.y << ")" << endl;
            if (is_neared(now_location.x, now_location.y, prop_location.x, prop_location.y, self->moveSpeed / 30))
            {
                g.Pick(proptype);
                if (int(g.GetSelfInfo()->propType) == 0)
                {
                    prop_location.x = prop_location.y = -1;
                    current_state = last_state;
                    cout << "No prop picked. PICKING -> " << last_state << endl;
                }
                else
                {
                    g.Use();
                    cout << "Use a prop" << endl;
                    prop_location.x = prop_location.y = -1;
                    current_state = last_state;
                    cout << "PICKING -> " << last_state << endl;
                }
            }
            else
            {
                move_to(g, prop_location.x, prop_location.y);
            }
        }
    }
    else if (current_state == AVOIDWALL)
    {
        if (wall_index == 0)
            current_state = last_state;
        else
        {
            cout << "AVOIDWALL" << endl;
            Location next_location;
            if (last_state == NAVIGATING)
                next_location = destination_list.back();
            else if (last_state == SUPPLYING)
                next_location = supply_location;
            if (!has_init_avoid_walls_list)
            {
                if (wall_index == 1)
                {
                    int begin, end;
                    if (debug >= 2)
                        cout << "next location: (" << next_location.x << " " << next_location.y << ")" << endl;
                    if (now_location.x <= 25000)
                        begin = 0;
                    else
                        begin = 1;
                    if (next_location.x <= 25000)
                        end = 0;
                    else
                        end = 1;
                    if (debug >= 2)
                        cout << "begin: " << begin << " end: " << end << endl;
                    if (begin == end)
                        avoid_wall_list.push_back(wall[1][begin]);
                    else
                    {
                        avoid_wall_list.push_back(wall[1][end]);
                        avoid_wall_list.push_back(wall[1][begin]);
                    }
                }
                else if (wall_index >= 2 && wall_index <= 9)
                {
                    int begin, end;
                    int ave_x = (wall[wall_index][0].x + wall[wall_index][2].x) / 2;
                    int ave_y = (wall[wall_index][0].y + wall[wall_index][2].y) / 2;
                    if (now_location.y <= ave_y)
                        begin = now_location.x <= ave_x ? 0 : 1;
                    else
                        begin = now_location.x <= ave_x ? 3 : 2;
                    if (next_location.y <= ave_y)
                        end = next_location.x <= ave_x ? 0 : 1;
                    else
                        end = next_location.x <= ave_x ? 3 : 2;
                    if (debug >= 2)
                        cout << "begin: " << begin << " end: " << end << endl;
                    if (begin == end)
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    else if (abs(begin - end) == 2)
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][(end + 1) % 4]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    else
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    if (debug >= 2)
                        for (int i = 0; i < avoid_wall_list.size(); i++)
                        {
                            cout << avoid_wall_list[i].x << "," << avoid_wall_list[i].y << endl;
                        }
                }
                else if (wall_index == 10)
                {
                    int begin = 0, end = 0;
                    if (now_location.y <= 15000)
                        begin = now_location.x <= 32500 ? 4 : 3;
                    else if (now_location.x >= 33000)
                        begin = now_location.y <= 18000 ? 3 : 2;
                    else if (now_location.y >= 21000)
                        begin = now_location.x <= 30000 ? 1 : 2;
                    else if (now_location.x <= 27000)
                        begin = now_location.y <= 20500 ? 0 : 1;
                    else
                        begin = now_location.y <= 19000 ? 4 : 0;
                    if (next_location.y <= 15000)
                        end = next_location.x <= 32500 ? 4 : 3;
                    else if (next_location.x >= 33000)
                        end = next_location.y <= 18000 ? 3 : 2;
                    else if (next_location.y >= 21000)
                        end = next_location.x <= 30000 ? 1 : 2;
                    else if (next_location.x <= 27000)
                        end = next_location.y <= 20500 ? 0 : 1;
                    else
                        end = next_location.y <= 19000 ? 4 : 0;
                    if (debug >= 2)
                        cout << "begin: " << begin << " end: " << end << endl;
                    if (begin == end)
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    else if (abs(begin - end) == 1 || abs(begin - end) == 4)
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    else if (abs(begin - end) == 2)
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][(min(begin, end) + 1) % 5]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    else
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][(max(begin, end) + 1) % 5]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    if (debug >= 2)
                        for (int i = 0; i < avoid_wall_list.size(); i++)
                        {
                            cout << avoid_wall_list[i].x << "," << avoid_wall_list[i].y << endl;
                        }
                }
                else if (wall_index == 11)
                {
                    int begin = 0, end = 0;
                    if (now_location.y <= 28000)
                        begin = now_location.x <= 20000 ? 4 : 0;
                    else if (now_location.x >= 23000)
                        begin = now_location.y <= 28500 ? 0 : 1;
                    else if (now_location.y >= 34000)
                        begin = now_location.x <= 17500 ? 3 : 2;
                    else if (now_location.x <= 17000)
                        begin = now_location.y >= 31000 ? 4 : 3;
                    else
                        begin = now_location.y <= 30000 ? 1 : 2;
                    if (next_location.y <= 28000)
                        end = next_location.x <= 20000 ? 4 : 0;
                    else if (next_location.x >= 23000)
                        end = next_location.y <= 28500 ? 0 : 1;
                    else if (next_location.y >= 34000)
                        end = next_location.x <= 17500 ? 3 : 2;
                    else if (next_location.x <= 17000)
                        end = next_location.y >= 31000 ? 4 : 3;
                    else
                        end = next_location.y <= 30000 ? 1 : 2;
                    if (debug >= 2)
                        cout << "begin: " << begin << " end: " << end << endl;
                    if (begin == end)
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    else if (abs(begin - end) == 1 || abs(begin - end) == 4)
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    else if (abs(begin - end) == 2)
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][(min(begin, end) + 1) % 5]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    else
                    {
                        avoid_wall_list.push_back(wall[wall_index][end]);
                        avoid_wall_list.push_back(wall[wall_index][(max(begin, end) + 1) % 5]);
                        avoid_wall_list.push_back(wall[wall_index][begin]);
                    }
                    if (debug >= 2)
                        for (int i = 0; i < avoid_wall_list.size(); i++)
                        {
                            cout << avoid_wall_list[i].x << "," << avoid_wall_list[i].y << endl;
                        }
                }
                if (!avoid_wall_list.empty())
                    has_init_avoid_walls_list = true;
            }
            else
            {
                if (avoid_wall_list.empty())
                {
                    current_state = last_state;
                    has_init_avoid_walls_list = false;
                    cout << "Sucess avoid NO." << wall_index << " walls" << endl;
                }
                else
                {
                    if (is_neared(now_location.x, now_location.y, avoid_wall_list.back().x, avoid_wall_list.back().y, self->moveSpeed / 40))
                        avoid_wall_list.pop_back();
                    else
                    {
                        move_to(g, avoid_wall_list.back().x, avoid_wall_list.back().y);
                        if (self->isDying)
                        {
                            cout << "Die while avoiding walls" << endl;
                            last_state = current_state = NAVIGATING;
                            has_init_avoid_walls_list = false;
                        }
                    }
                }
            }
        }
    }
    else if (current_state == SUPPLYING)
    {
        cout << "SUPPLYING" << endl;
        auto now_color = g.GetCellColor(now_location.x / 1000, now_location.y / 1000);
        int bulletnum = self->bulletNum;
        if (int(selfcolor) == int(now_color)) //已经开始补弹
        {
            cout << "Issupplying, bullet: " << bulletnum << "/" << Maxbulletnum << endl;
            issupplying = true;
            if (bulletnum >= 0.95 * Maxbulletnum)
            {
                supplyFlag = false;
                current_state = NAVIGATING;
            }
            supply_location.x = supply_location.y = -1;
        }
        else if (!error_area(supply_location.x, supply_location.y))
        {
            move_to(g, supply_location.x, supply_location.y);
            self = g.GetSelfInfo();
            now_location.x = self->x;
            now_location.y = self->y;
            if (is_neared(now_location.x, now_location.y, supply_location.x, supply_location.y, 200) && int(selfcolor) != int(now_color))
            {
                supply_location.x = supply_location.y = -1;
                current_state = NAVIGATING;
                cout << "Colorarea disappeared! SUPPLYING -> NAVIGATING" << endl;
            }
        }
        else
        {
            cout << "Going to supply, bullet: " << bulletnum << "/" << Maxbulletnum << endl;
            supply_location.x = supply_location.y = -1;
            for (int i = -2; i < 3; i++) //找染色区
            {
                if (now_location.x / 1000 + i < 0 || now_location.x / 1000 + i > 49)
                    continue;
                for (int j = -2; j < 3; j++) //搜索一个5x5的区域
                {
                    if (now_location.y / 1000 + j < 0 || now_location.y / 1000 + j > 49)
                        continue;
                    auto areacolor = g.GetCellColor(now_location.x / 1000 + i, now_location.y / 1000 + j);
                    if (int(selfcolor) == int(areacolor) && is_accessible(g, now_location.x, now_location.y,
                                                                          (now_location.x / 1000 + i) * 1000 + 500, (now_location.y / 1000 + j) * 1000 + 500)) //匹配成功
                    {
                        supply_location.x = (now_location.x / 1000 + i) * 1000 + 500;
                        supply_location.y = (now_location.y / 1000 + j) * 1000 + 500;
                        cout << "find a colorarea at (" << supply_location.x << "," << supply_location.y << ")" << endl;
                        break;
                    }
                }
            }
            if (error_area(supply_location.x, supply_location.y)) //没找到染色区
            {
                if (bulletnum >= 1)
                {
                    supply_location.x = now_location.x / 1000 * 1000 + 500;
                    supply_location.y = now_location.y / 1000 * 1000 + 500;
                    shoot_at(g, supply_location.x, supply_location.y);
                }
                else
                {
                    cout << "Can't find or create a colorarea" << endl;
                    current_state = NAVIGATING;
                }
            }
        }
    }
    if (debug >= 1)
        cout << "location: (" << now_location.x << "," << now_location.y << ") " << endl;
    if (debug >= 2)
        cout << "facing direction: " << self->facingDirection << endl;
    if (debug >= 3)
    {
        cout << "walls: " << walls.size() << ' ';
        for (int i = 0; i < walls.size(); i++)
        {
            cout << "(" << walls[i]->x << "," << walls[i]->y << ") ";
        }
        cout << endl;
    }
    solve_deadlock(g);
    last_location = now_location;
}
