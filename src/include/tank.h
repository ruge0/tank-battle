#ifndef SERVER_TANK_H
#define SERVER_TANK_H

class Tank
{
public:
    int x, y;         //坐标
    int m_direction;  //朝向
    int m_color;      //颜色
    int m_model;      //模型
    int m_id;         //编号

    Tank();
    Tank(int id, int model, int color, int direction, int x, int y);
    ~Tank();
};

#endif //SERVER_TANK_H
