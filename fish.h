#pragma once

const int N=40;//地图横坐标范围1-N
const int M=40;//地图纵坐标范围1-M
const int MAX_PLAYER=40;//最多玩家数量，ID从1-MAX_PLAYER
const int MAX_FOOD=50;//刷新食物数量
const int FOOD_ROUND=5;//刷新食物回合数
const int EMPTY=0;//格子为空
const int FOOD=-1;//食物
const int LEVEL_POINT=3;//升级能获得的点数

class fish{
private:
    void setID(int){}//设置ID，只可被调用一次
public:
    fish(){}
    ~fish(){}
    bool move(int,int){}//移动到某个位置
    bool attack(int,int){}//攻击某个位置
    int getPoint(){}//返回当前玩家剩余点数
    int getLevel(){}//返回当前玩家等级
    int getExp(){}//返回当前玩家经验
    int getX(){}//返回当前玩家横坐标
    int getY(){}//返回当前玩家纵坐标
    int getHP(){}//返回当前玩家生命值
    int getMaxHP(){}//返回当前玩家生命上限
    int getAtt(){}//返回当前玩家攻击力
    int getSp(){}//返回当前玩家速度
    int getID(){}//获取玩家ID
    int askWhat(int,int){}//返回地图某个位置内容（玩家ID或FOOD或EMPTY）
    int askHP(int){}//询问某个玩家ID当前血量
    bool increaseHealth(){}//增加health属性1
    bool increaseStrength(){}//增加Strength属性1
    bool increaseSpeed(){}//增加Speed属性1

    virtual void init(){}//初始化函数，每一局重新开始将调用
    virtual void play(){}//行动函数，每回合行动将调用
    virtual void revive(int&,int&){}//复活时调用，参数传回选择复活位置（若不合法则随机）
};

//可自行增加所需函数及变量，但需保证上面每个函数的完整与可调用

//不要使用全局变量!!!
