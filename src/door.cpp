// door.cpp - дверь
#include "door.h"
#include "grid_map.h"
#include "player.h"
#include "renderer.h"

void door::draw(renderer &r) {
    if (opened)
        return;
    Vector3 center = {position.x, WALL_HEIGHT / 2.0f, position.z};
    Vector3 size = {CELL, WALL_HEIGHT, CELL};
    Color brown = {95, 60, 40, 255};
    r.draw_box(center, size, brown);
}

string door::open(player &p) {
    if (p.get_keys() < need_keys)
        return TextFormat("Заперто. Нужно ключей: %d", need_keys);
    opened = true;
    return "Дверь открыта";
}
