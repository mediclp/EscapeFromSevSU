// level.h - уровень: карта и все объекты на ней
#pragma once
#include <string>
#include "grid_map.h"
#include "enemy.h"
#include "item.h"
#include "door.h"
using namespace std;

const int MAX_ENEMIES = 10;
const int MAX_ITEMS = 40;
const int MAX_DOORS = 5;

// абстрактный уровень: загружает карту из файла и хранит объекты.
// каких монстров создать, решает наследник: level1, level2 или level3
class level {
private:
    grid_map grid;
    enemy *enemies[MAX_ENEMIES];
    int enemy_count;
    item *items[MAX_ITEMS];
    int item_count;
    door *doors[MAX_DOORS];
    int door_count;
    Vector3 spawn;
    cell exit_cell;
    bool has_spawn, has_exit;

    // копировать уровень нельзя: две копии удалили бы одни и те же объекты
    level(level &ob);

    // создаёт объекты по символам карты
    void create_objects();

    // кладёт случайные бонусы туда, куда герой может дойти
    void add_random_bonuses();

    // кладут объект в массив; если места нет, бросают исключение
    void add_enemy(enemy *e);
    void add_item(item *it);
    void add_door(door *d);

    // ближайшие к герою предмет и дверь или NULL
    item* nearest_item(player &p);
    door* nearest_door(player &p);

public:
    level();

    // удаляет все объекты уровня
    virtual ~level();

    // создаёт монстра, который живёт на этом уровне
    virtual enemy* create_enemy(Vector3 p) = 0;

    // читает карту из файла; если файла нет или карта неправильная, бросает исключение
    void load(string path);

    // обновляет монстров и убирает подобранные предметы
    void update(float dt, game_context &ctx);

    // рисует клетки рядом с камерой и все объекты
    void draw(renderer &r);

    // стоит ли герой на клетке выхода
    bool is_exit(Vector3 p);

    // действие E: берёт ближайший предмет или открывает дверь.
    // возвращает текст для экрана
    string interact(player &p);

    // подсказка, что можно сделать рядом с героем
    string prompt(player &p);

    grid_map& get_map() { return grid; }
    Vector3 get_spawn() { return spawn; }
    cell get_exit() { return exit_cell; }
    int get_enemy_count() { return enemy_count; }
    enemy* get_enemy(int i) { return enemies[i]; }
    int get_item_count() { return item_count; }
    item* get_item(int i) { return items[i]; }
    int get_door_count() { return door_count; }
    door* get_door(int i) { return doors[i]; }
};

// создаёт уровень по номеру 1, 2 или 3
level* create_level(int number);
