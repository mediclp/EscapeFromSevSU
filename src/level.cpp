// level.cpp - загрузка уровня и работа с его объектами
#include "level.h"
#include <fstream>
#include "level1.h"
#include "level2.h"
#include "level3.h"
#include "player.h"
#include "renderer.h"

const float INTERACT_DISTANCE = 1.5f;   // с какого расстояния берутся предметы
const float DOOR_DISTANCE = 1.6f;       // с какого расстояния открываются двери: только вплотную
const float ITEM_HEIGHT = 0.3f;         // высота предмета над полом
const int RANDOM_BONUS_COUNT = 2;       // сколько бонусов появляется случайно

const int NOTE_COUNT = 3;
const string NOTE_TEXTS[NOTE_COUNT] = {
    "«Пересдача в 23:40. Не опаздывать.»",
    "«Если слышишь шаги, не беги. Оно идёт на звук.»",
    "«Ключи от лестницы у вахтёра. Вахтёра больше нет.»"};

level* create_level(int number) {
    switch (number) {
    case 1:
        return new level1();
    case 2:
        return new level2();
    default:
        return new level3();
    }
}

level::level() {
    enemy_count = 0;
    item_count = 0;
    door_count = 0;
    spawn.x = 0.0f;
    spawn.y = 0.0f;
    spawn.z = 0.0f;
    exit_cell.x = 0;
    exit_cell.z = 0;
    has_spawn = false;
    has_exit = false;
}

level::~level() {
    for (int i = 0; i < enemy_count; i++)
        delete enemies[i];
    for (int i = 0; i < item_count; i++)
        delete items[i];
    for (int i = 0; i < door_count; i++)
        delete doors[i];
}

void level::load(string path) {
    ifstream file(path.c_str());
    if (!file.is_open())
        throw "Не найден файл карты";

    string row;
    while (getline(file, row)) {
        // файл мог быть сохранён в Windows с \r в конце строки
        if (row.length() > 0 && row[row.length() - 1] == '\r')
            row.erase(row.length() - 1);
        if (row.length() > 0)
            grid.add_row(row);
    }
    file.close();

    create_objects();
    if (!has_spawn)
        throw "На карте нет старта P";
    if (!has_exit)
        throw "На карте нет выхода X";

    add_random_bonuses();
}

void level::create_objects() {
    int next_note = 0;
    for (int z = 0; z < grid.get_height(); z++) {
        for (int x = 0; x < grid.get_width(); x++) {
            cell c = {x, z};
            Vector3 center = cell_center(c);
            Vector3 on_floor = {center.x, ITEM_HEIGHT, center.z};
            char s = grid.symbol_at(c);
            switch (s) {
            case 'P':
                spawn = center;
                has_spawn = true;
                break;
            case 'X':
                if (!has_exit) {   // выходов может быть несколько, запоминаем первый
                    exit_cell = c;
                    has_exit = true;
                }
                break;
            case 'E':
                add_enemy(create_enemy(center));   // монстра выбирает наследник
                break;
            case 'a':
            case 'b':
            case 'v':
            case 'c':
                add_item(new key(on_floor, s));   // ключ от лестницы или пропуск
                break;
            case 'B':
                add_item(new battery(on_floor));
                break;
            case 'N':
                add_item(new note(on_floor, NOTE_TEXTS[next_note % NOTE_COUNT]));
                next_note++;
                break;
            case '1':
            case '2':
            case '3':
            case 'T':
                add_door(new door(center, door_key(s)));   // дверь на лестницу или турникет
                break;
            }
        }
    }
}

void level::add_random_bonuses() {
    cell start = cell_at(spawn);
    for (int i = 0; i < RANDOM_BONUS_COUNT; i++) {
        cell c = grid.random_floor_cell();
        while (!grid.is_reachable(c, start))
            c = grid.random_floor_cell();

        Vector3 center = cell_center(c);
        Vector3 on_floor = {center.x, ITEM_HEIGHT, center.z};
        if (GetRandomValue(0, 1) == 0)
            add_item(new battery(on_floor));
        else
            add_item(new energy_drink(on_floor));
    }
}

void level::add_enemy(enemy *e) {
    if (enemy_count >= MAX_ENEMIES) {
        delete e;
        throw "На карте слишком много монстров";
    }
    enemies[enemy_count] = e;
    enemy_count++;
}

void level::add_item(item *it) {
    if (item_count >= MAX_ITEMS) {
        delete it;
        throw "На карте слишком много предметов";
    }
    items[item_count] = it;
    item_count++;
}

void level::add_door(door *d) {
    if (door_count >= MAX_DOORS) {
        delete d;
        throw "На карте слишком много дверей";
    }
    doors[door_count] = d;
    door_count++;
}

void level::remove_enemies() {
    for (int i = 0; i < enemy_count; i++)
        delete enemies[i];
    enemy_count = 0;
}

void level::update(float dt, game_context &ctx) {
    // полиморфизм: у каждого монстра своя версия notices внутри update
    for (int i = 0; i < enemy_count; i++)
        enemies[i]->update(dt, ctx);

    // подобранные предметы удаляем, остальные сдвигаем к началу массива
    int n = 0;
    for (int i = 0; i < item_count; i++) {
        if (items[i]->is_active()) {
            items[n] = items[i];
            n++;
        } else {
            delete items[i];
        }
    }
    item_count = n;
}

void level::draw(renderer &r) {
    for (int z = 0; z < grid.get_height(); z++) {
        for (int x = 0; x < grid.get_width(); x++) {
            cell c = {x, z};
            Vector3 center = cell_center(c);
            if (!r.is_near_camera(center))
                continue;
            char s = grid.symbol_at(c);
            if (s == '#') {
                r.draw_wall(center);
            } else if (s == 'L') {
                // лифт - декорация: света нет, лифты не работают
                Vector3 lift = {center.x, WALL_HEIGHT / 2.0f, center.z};
                Vector3 lift_size = {CELL, WALL_HEIGHT, CELL};
                Color steel = {105, 110, 118, 255};
                r.draw_box(lift, lift_size, steel);
            } else {
                r.draw_floor_and_ceiling(center);
            }
            if (s == 'X') {
                Vector3 mark = {center.x, 0.05f, center.z};
                Vector3 mark_size = {CELL * 0.8f, 0.05f, CELL * 0.8f};
                Color green = {50, 170, 80, 255};
                r.draw_box(mark, mark_size, green);
            }
        }
    }
    for (int i = 0; i < door_count; i++)
        doors[i]->draw(r);
    for (int i = 0; i < item_count; i++)
        items[i]->draw(r);
    for (int i = 0; i < enemy_count; i++)
        enemies[i]->draw(r);
}

bool level::is_exit(Vector3 p) {
    return grid.symbol_at(cell_at(p)) == 'X';
}

item* level::nearest_item(player &p) {
    item *nearest = NULL;
    float best = INTERACT_DISTANCE;
    for (int i = 0; i < item_count; i++) {
        float d = flat_distance(items[i]->get_position(), p.get_position());
        // предмет за стеной не берём: клетки маленькие, стена бывает тоньше расстояния
        if (d < best && grid.line_of_sight(items[i]->get_position(), p.get_position())) {
            best = d;
            nearest = items[i];
        }
    }
    return nearest;
}

door* level::nearest_door(player &p) {
    for (int i = 0; i < door_count; i++) {
        if (!doors[i]->is_open() && flat_distance(doors[i]->get_position(), p.get_position()) < DOOR_DISTANCE)
            return doors[i];
    }
    return NULL;
}

string level::interact(player &p) {
    item *it = nearest_item(p);
    if (it != NULL)
        return it->pick_up(p);

    door *d = nearest_door(p);
    if (d == NULL)
        return "";

    string message = d->open(p);
    if (d->is_open())
        grid.clear_cell(cell_at(d->get_position()));
    return message;
}

string level::prompt(player &p) {
    item *it = nearest_item(p);
    if (it != NULL)
        return "E — взять " + it->name();
    door *d = nearest_door(p);
    if (d != NULL)
        return "E — " + d->action();
    return "";
}
