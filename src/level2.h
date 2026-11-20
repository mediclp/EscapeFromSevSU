// level2.h - уровень 2: охотники на свет
#pragma once
#include "level.h"

// быстрый монстр, который идёт на свет фонарика
class light_hunter_enemy : public enemy {
public:
    light_hunter_enemy(Vector3 p) : enemy(p) {}
    bool notices(game_context &ctx);
    float patrol_speed() { return 1.4f; }
    float chase_speed() { return 3.0f; }
    Color color() { return {90, 105, 120, 255}; }
};

// второй уровень: второй этаж главного корпуса, на нём живут охотники на свет
class level2 : public level {
public:
    enemy* create_enemy(Vector3 p) { return new light_hunter_enemy(p); }
    string name() { return "Главный корпус, 2 этаж"; }
};
