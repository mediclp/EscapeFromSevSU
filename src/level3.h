// level3.h - уровень 3: вахтёры
#pragma once
#include "level.h"

// самый быстрый монстр: видит героя и слышит шум
class warden_enemy : public enemy {
public:
    warden_enemy(Vector3 p) : enemy(p) {}
    bool notices(game_context &ctx);
    float patrol_speed() { return 1.8f; }
    float chase_speed() { return 4.0f; }
    Color color() { return {130, 70, 65, 255}; }
};

// третий уровень: на нём живут вахтёры
class level3 : public level {
public:
    enemy* create_enemy(Vector3 p) { return new warden_enemy(p); }
};
