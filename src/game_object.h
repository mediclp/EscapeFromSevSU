// game_object.h - базовый класс всех объектов игры
#pragma once
#include "raylib.h"

class level;
class player;
class renderer;

// всё, что объект видит в мире на одном шаге игры
struct game_context {
    level &lvl;
    player &hero;
    bool alarm;   // идёт ли воздушная тревога
};

// абстрактный объект игры: у него есть позиция, он обновляется и рисуется
class game_object {
protected:
    Vector3 position;
    bool active;   // неактивный объект убирается с уровня

public:
    game_object(Vector3 p) {
        position = p;
        active = true;
    }

    // виртуальный деструктор: при delete через указатель на базу удалится весь наследник
    virtual ~game_object() {}

    // шаг объекта на dt секунд, по умолчанию объект стоит на месте
    virtual void update(float dt, game_context &ctx) {}

    // рисует объект, у каждого наследника по-своему
    virtual void draw(renderer &r) = 0;

    Vector3 get_position() { return position; }
    bool is_active() { return active; }
};
