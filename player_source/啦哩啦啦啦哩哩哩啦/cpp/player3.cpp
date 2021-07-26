#include "AI.h"
#include "Constants.h"
#include <chrono>
#include<thread>


//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job3; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}



static double pi = 3.1415926535898;
static int a = 0, n = 0, d = 0;
static int pick=0,stay=0,wait=0,M=0;
static int A = 1,judging=0;
static int formerX = 0, formerY = 0; 

bool judgemove(GameApi& g)
{
    if (g.GetSelfInfo()->x== formerX && g.GetSelfInfo()->y== formerY) { return false; }
    else { return true; }
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

void picktool(GameApi& g)    //捡道具函数（随捡随用）
{
    
        auto prop = g.GetProps();
        if (prop[0]->x / 1000 == g.GetSelfInfo()->x / 1000 && prop[0]->y / 1000 == g.GetSelfInfo()->y / 1000)
        {
            g.Pick(prop[0]->propType);
            g.Use();
            pick = 0;
        }
        double beta = angleCalculate(g, prop[0]->x, prop[0]->y);     
        double d = distanceCalculate(g, prop[0]->x, prop[0]->y);
        g.MovePlayer(d/ g.GetSelfInfo()->moveSpeed, beta);
        
        pick += 1;

        
    
}

void AI::play(GameApi& g)
{   
        n += 1;
        if ((n % 16) < 8)d = 1; else d = -1;
        if (g.GetSelfInfo()->bulletNum < 3&&stay>=0)
        {
            g.Attack(1, a * pi / 2);
            do
            {
                stay += 1;

            } while (g.GetSelfInfo()->bulletNum < 3&&stay<500);
            if (g.GetSelfInfo()->bulletNum < 3) { stay = -1; }

        }
        else
        {
            if (g.GetProps().size() && pick < 200)
            {
                A = 0;
                picktool(g);
            }
            else if (A)
            {

                g.Attack(1, (a % 4) * pi / 2);
                if(g.GetSelfInfo()->bulletNum>1)g.Attack(1, (a % 4) * pi / 2 + d * pi / 4);

                A = 0;
                do
                {
                    wait += 1;
                } while (g.GetBullets().size());
                M = wait;
                do
                {
                    wait += 1;
                } while (wait < 2 * M);
                M = 0; wait = 0;
                g.MovePlayer(10 * 1000 * 1000 / g.GetSelfInfo()->moveSpeed, (a % 4) * pi / 2);

            }
            else
            {
                if (g.GetProps().size() == 0)pick = 0;
                if (!judging)
                {
                    formerX = g.GetSelfInfo()->x; formerY = g.GetSelfInfo()->y;
                    judging = 1;
                    g.MovePlayer(1000 * 1000 / g.GetSelfInfo()->moveSpeed, (a % 4) * pi / 2);
                }
                else
                {
                    if (!judgemove(g))
                    {
                        a += d;
                        judging = 0;
                    }
                    else
                    {
                        A = 1; judging = 0;
                    }
                }

            }


        }
    
}


