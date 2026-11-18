// enemy.h - абстрактный монстр с общими состояниями
#pragma once
#include "game_object.h"
#include "grid_map.h"

// патруль -> заметил -> погоня -> потерял -> поиск -> патруль
enum enemy_state { PATROL, ALERTED, CHASE, LOST, SEARCH };

// состояния у всех монстров общие, а как монстр замечает героя,
// с какой скоростью ходит и какого он цвета, решают наследники
class enemy : public game_object {
private:
    enemy_state state;
    float state_time;    // сколько монстр в текущем состоянии
    float unseen_time;   // сколько в погоне не замечает героя
    cell goal;           // куда монстр идёт
    cell waypoint;       // ближайшая клетка на пути к цели

    // выбирает следующее состояние и цель
    void update_state(float dt, game_context &ctx);

    // переходит в новое состояние и сбрасывает таймеры
    void change_state(enemy_state s);

    // идёт по клеткам к цели
    void walk_to_goal(float dt, game_context &ctx, float speed);

    // скорость с учётом состояния и тревоги
    float current_speed(bool alarm);

public:
    enemy(Vector3 p);

    // шаг монстра: смена состояния, движение и проверка, поймал ли он героя.
    // наследники его не переопределяют, они меняют только методы ниже
    void update(float dt, game_context &ctx);

    void draw(renderer &r);

    enemy_state get_state() { return state; }

    // заметил ли монстр героя. здесь общая проверка «герой вплотную»,
    // наследники добавляют к ней своё через enemy::notices(ctx)
    virtual bool notices(game_context &ctx);

    // скорость в патруле и в погоне, м/с
    virtual float patrol_speed() = 0;
    virtual float chase_speed() = 0;

    // цвет монстра
    virtual Color color() = 0;
};
