#include "tank.h"

Tank::Tank() = default;

Tank::Tank(int id, int model, int color, int direction, int x, int y)
{
    this->m_id = id;
    this->m_model = model;
    this->m_color = color;
    this->m_direction = direction;
    this->x = x;
    this->y = y;
}

Tank::~Tank() = default;