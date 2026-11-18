// item.cpp - эффекты предметов
#include "item.h"
#include "player.h"
#include "renderer.h"

const float BATTERY_CHARGE = 40.0f;
const float DRINK_STAMINA = 100.0f;

void item::draw(renderer &r) {
    Vector3 size = {0.25f, 0.25f, 0.25f};
    r.draw_box(position, size, color());
}

string item::pick_up(player &p) {
    active = false;
    p += points();
    return apply_to(p);
}

string key::apply_to(player &p) {
    p.add_key();
    return "Найден ключ";
}

string battery::apply_to(player &p) {
    p.add_charge(BATTERY_CHARGE);
    return "Фонарик заряжен";
}

string energy_drink::apply_to(player &p) {
    p.add_stamina(DRINK_STAMINA);
    return "Выносливость восстановлена";
}
