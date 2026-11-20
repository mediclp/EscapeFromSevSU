// door.cpp - дверь и турникет
#include "door.h"
#include "grid_map.h"
#include "item.h"
#include "player.h"
#include "renderer.h"

const char PASS = 'c';   // ключ турникета - электронный пропуск

char door_key(char symbol) {
    switch (symbol) {
    case '1':
        return 'a';
    case '2':
        return 'b';
    case '3':
        return 'v';
    default:
        return PASS;
    }
}

void door::draw(renderer &r) {
    if (opened)
        return;
    if (need_key == PASS) {
        // турникет: невысокая металлическая стойка
        Vector3 center = {position.x, 0.5f, position.z};
        Vector3 size = {CELL * 0.8f, 1.0f, CELL * 0.8f};
        Color steel = {120, 125, 130, 255};
        r.draw_box(center, size, steel);
        return;
    }
    Vector3 center = {position.x, WALL_HEIGHT / 2.0f, position.z};
    Vector3 size = {CELL, WALL_HEIGHT, CELL};
    Color brown = {95, 60, 40, 255};
    r.draw_box(center, size, brown);
}

string door::open(player &p) {
    if (!p.has_key(need_key)) {
        if (need_key == PASS)
            return "Турникет закрыт. Нужен " + key_title(need_key);
        return "Заперто. Нужен " + key_title(need_key);
    }
    opened = true;
    if (need_key == PASS)
        return "Турникет открыт";
    return "Дверь на лестницу открыта";
}

string door::action() {
    if (need_key == PASS)
        return "пройти через турникет";
    return "открыть дверь на лестницу";
}
