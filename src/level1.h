// level1.h - уровень 1: монстры-слухачи
#pragma once
#include "level.h"

// медленный монстр, который слышит шаги и бег
class listener_enemy : public enemy {
public:
    listener_enemy(Vector3 p) : enemy(p) {}
    bool notices(game_context &ctx);
    float patrol_speed() { return 1.2f; }
    float chase_speed() { return 2.2f; }
    Color color() { return {120, 110, 95, 255}; }
};

// первый уровень: на нём живут слухачи
class level1 : public level {
public:
    enemy* create_enemy(Vector3 p) { return new listener_enemy(p); }
};
