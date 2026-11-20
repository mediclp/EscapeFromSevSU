// level3.h - уровень 3: вахтёры
#pragma once
#include "level.h"

// вахтёр: видит героя недалеко и слышит бег; от бегущего героя отстаёт
class warden_enemy : public enemy {
public:
    warden_enemy(Vector3 p) : enemy(p) {}
    bool notices(game_context &ctx);
    float patrol_speed() { return 1.2f; }
    float chase_speed() { return 2.8f; }
    Color color() { return {130, 70, 65, 255}; }
};

// третий уровень: подвал главного корпуса, на нём живут вахтёры
class level3 : public level {
public:
    enemy* create_enemy(Vector3 p) { return new warden_enemy(p); }
    string name() { return "Подвал"; }
};
