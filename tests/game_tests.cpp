// game_tests.cpp - проверки логики без окна: карты, двери, столкновения,
// предметы, очки, рекорды и поведение монстров
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include "door.h"
#include "enemy.h"
#include "item.h"
#include "level.h"
#include "player.h"
#include "record.h"
#include "raymath.h"
using namespace std;

int failures = 0;

// печатает результат проверки и считает ошибки
void check(bool condition, string name) {
    if (condition) {
        cout << "[ OK ] " << name << endl;
    } else {
        cout << "[FAIL] " << name << endl;
        failures++;
    }
}

string map_path(int number) {
    return string(MAPS_DIR) + "level" + TextFormat("%d", number) + ".txt";
}

// загружает карту; при ошибке возвращает false вместо исключения
bool loads(level *l, string path) {
    try {
        l->load(path);
    }
    catch (const char *msg) {
        return false;
    }
    return true;
}

// команды героя: стоит на месте, ничего не нажимает
player_input no_input() {
    player_input in = {{0.0f, 0.0f}, {0.0f, 0.0f}, false, false};
    return in;
}

// шумит как герой: идёт или бежит, не сдвигаясь с места
void make_noise(player &p, level &l, bool running) {
    game_context ctx = {l, p, false};
    player_input in = no_input();
    in.move.y = 1.0f;
    in.run = running;
    p.apply_input(in, 0.0f, ctx);
}

// монстр, ближайший к точке p
enemy* nearest_enemy(level *l, Vector3 p) {
    enemy *best = l->get_enemy(0);
    for (int i = 1; i < l->get_enemy_count(); i++)
        if (flat_distance(l->get_enemy(i)->get_position(), p) < flat_distance(best->get_position(), p))
            best = l->get_enemy(i);
    return best;
}

// центр клетки в cells клетках от точки по прямой без стен; тесты не зависят от формы карты
Vector3 free_spot(level *l, Vector3 from, int cells) {
    cell c = cell_at(from);
    int dx[4] = {1, -1, 0, 0};
    int dz[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; i++) {
        bool ok = true;
        for (int k = 1; k <= cells; k++) {
            cell n = {c.x + dx[i] * k, c.z + dz[i] * k};
            if (l->get_map().is_solid(n))
                ok = false;
        }
        if (ok) {
            cell n = {c.x + dx[i] * cells, c.z + dz[i] * cells};
            return cell_center(n);
        }
    }
    return from;
}

// выключает или включает фонарик героя
void toggle_flashlight(player &p, level &l) {
    game_context ctx = {l, p, false};
    player_input in = no_input();
    in.toggle_flashlight = true;
    p.apply_input(in, 0.0f, ctx);
}

// проверяет карту: состав, доступность объектов и закрытый дверью выход
void test_map(int number) {
    string prefix = string("уровень ") + TextFormat("%d", number) + ": ";
    level *l = create_level(number);
    check(loads(l, map_path(number)), prefix + "карта загружается");
    check(l->get_enemy_count() >= 2, prefix + "несколько монстров");
    check(l->get_door_count() > 0, prefix + "есть дверь");

    grid_map &m = l->get_map();
    cell start = cell_at(l->get_spawn());
    // каждый выход (лестница или улица) закрыт дверью или турникетом
    int exits = 0;
    bool all_closed = true;
    for (int z = 0; z < m.get_height(); z++)
        for (int x = 0; x < m.get_width(); x++) {
            cell c = {x, z};
            if (m.symbol_at(c) == 'X') {
                exits++;
                if (m.is_reachable(start, c))
                    all_closed = false;
            }
        }
    check(exits > 0 && all_closed, prefix + "все выходы закрыты дверями");
    for (int i = 0; i < l->get_item_count(); i++) {
        item *it = l->get_item(i);
        check(m.is_reachable(start, cell_at(it->get_position())), prefix + "предмет достижим: " + it->name());
    }
    for (int i = 0; i < l->get_enemy_count(); i++)
        check(m.is_reachable(start, cell_at(l->get_enemy(i)->get_position())), prefix + "монстр достижим");

    // собираем все предметы и открываем двери, как это сделал бы игрок
    string keys = "";
    while (l->get_item_count() > 0) {
        player collector(l->get_item(0)->get_position());
        l->interact(collector);
        string all = "abvc";
        for (int k = 0; k < 4; k++)
            if (collector.has_key(all[k]))
                keys += all[k];
        game_context ctx = {*l, collector, false};
        l->update(0.0f, ctx);
    }
    for (int i = 0; i < l->get_door_count(); i++) {
        player at_door(l->get_door(i)->get_position());
        for (int k = 0; k < (int)keys.length(); k++)
            at_door.add_key(keys[k]);
        l->interact(at_door);
    }
    check(m.is_reachable(start, l->get_exit()), prefix + "с ключами выход достижим");
    delete l;
}

void test_broken_maps() {
    level *missing = create_level(1);
    check(!loads(missing, "no_such_file.txt"), "несуществующая карта не загружается");
    delete missing;

    const char *path = "test_no_start.txt";
    ofstream f(path);
    f << "#####\n#..X#\n#####\n";
    f.close();
    level *no_start = create_level(1);
    check(!loads(no_start, path), "карта без старта не загружается");
    delete no_start;
    remove(path);
}

void test_door_needs_its_key() {
    Vector3 p = {1.0f, 0.0f, 1.0f};
    door d(p, 'a');
    player hero(p);
    hero.add_key('b');
    d.open(hero);
    check(!d.is_open(), "дверь на лестницу А не открывается ключом от Б");
    hero.add_key('a');
    d.open(hero);
    check(d.is_open(), "дверь на лестницу А открывается своим ключом");

    door gate(p, door_key('T'));
    player guest(p);
    gate.open(guest);
    check(!gate.is_open(), "турникет не пускает без пропуска");
    guest.add_key('c');
    gate.open(guest);
    check(gate.is_open(), "турникет пускает с пропуском");
}

// лифты - декорация: они непроходимы и кнопкой E не открываются
void test_lifts_are_decoration() {
    for (int n = 1; n <= 3; n++) {
        level *l = create_level(n);
        loads(l, map_path(n));
        grid_map &m = l->get_map();
        int lifts = 0;
        for (int z = 0; z < m.get_height(); z++)
            for (int x = 0; x < m.get_width(); x++) {
                cell c = {x, z};
                if (m.symbol_at(c) == 'L') {
                    lifts++;
                    // герой со всеми ключами жмёт E рядом с лифтом
                    Vector3 lift_pos = cell_center(c);
                    lift_pos.x -= 1.5f;
                    player near_lift(lift_pos);
                    near_lift.add_key('a');
                    near_lift.add_key('b');
                    near_lift.add_key('v');
                    near_lift.add_key('c');
                    l->interact(near_lift);
                    check(m.is_solid(c), string("уровень ") + TextFormat("%d", n) + ": лифт не открывается");
                }
            }
        check(lifts == 2, string("уровень ") + TextFormat("%d", n) + ": два лифта");
        delete l;
    }
}

void test_collision() {
    level *l = create_level(1);
    loads(l, map_path(1));
    Vector3 spawn = l->get_spawn();
    check(!l->get_map().collides(spawn.x, spawn.z, PLAYER_RADIUS), "старт свободен");
    check(l->get_map().collides(0.5f, 0.5f, PLAYER_RADIUS), "стена блокирует героя");
    delete l;
}

void test_items() {
    Vector3 zero = {0.0f, 0.0f, 0.0f};
    player hero(zero);
    hero.add_charge(50.0f);
    check(hero.get_charge() == 100.0f, "заряд не превышает 100");

    key k(zero, 'a');
    k.pick_up(hero);
    check(hero.get_keys() == 1 && hero.get_score() == 50, "ключ считается и даёт очки");
    check(!k.is_active(), "подобранный предмет исчезает");
}

// выдохшийся герой не бежит, пока не отдышится
void test_tired_player_walks() {
    level *l = create_level(1);
    loads(l, map_path(1));
    player hero(l->get_spawn());
    game_context ctx = {*l, hero, false};
    player_input in = no_input();
    in.move.y = 1.0f;
    in.run = true;
    for (int i = 0; i < 50; i++)
        hero.apply_input(in, 0.1f, ctx);     // 5 секунд бега: выносливость кончилась
    check(hero.is_tired(), "без выносливости герой выдыхается");
    hero.apply_input(in, 0.1f, ctx);
    check(hero.noise_radius() < 2.0f, "выдохшийся герой идёт шагом, а не бежит");
    delete l;
}

void test_energy_drink() {
    level *l = create_level(1);
    loads(l, map_path(1));
    player hero(l->get_spawn());
    game_context ctx = {*l, hero, false};
    player_input in = no_input();
    in.move.y = 1.0f;
    in.run = true;
    hero.apply_input(in, 1.0f, ctx);
    check(hero.get_stamina() < 100.0f, "бег тратит выносливость");

    Vector3 zero = {0.0f, 0.0f, 0.0f};
    energy_drink drink(zero);
    drink.pick_up(hero);
    check(hero.get_stamina() == 100.0f, "энергетик восстанавливает выносливость");
    for (int i = 0; i < 100; i++)
        hero.apply_input(in, 0.1f, ctx);      // 10 секунд бега под энергетиком
    check(hero.get_stamina() == 100.0f && !hero.is_tired(), "под энергетиком выносливость не тратится");
    for (int i = 0; i < 60; i++)
        hero.apply_input(in, 0.1f, ctx);      // ещё 6 секунд: энергетик закончился
    check(hero.get_stamina() < 100.0f, "через 15 секунд энергетик заканчивается");
    delete l;
}

// обновляет монстра seconds секунд или пока он не поймает героя
void simulate(enemy &e, game_context &ctx, float seconds) {
    float dt = 1.0f / 60.0f;
    for (float t = 0.0f; t < seconds && !ctx.hero.is_caught(); t += dt)
        e.update(dt, ctx);
}

void test_listener_hears_noise() {
    level *l = create_level(1);
    loads(l, map_path(1));
    enemy *e = nearest_enemy(l, l->get_spawn());
    Vector3 close = free_spot(l, e->get_position(), 3);

    player quiet(close);
    game_context quiet_ctx = {*l, quiet, false};
    simulate(*e, quiet_ctx, 0.5f);
    check(e->get_state() == PATROL, "слухач не замечает стоящего героя");

    level *loud_level = create_level(1);
    loads(loud_level, map_path(1));
    enemy *hunter = nearest_enemy(loud_level, loud_level->get_spawn());
    player loud(close);
    make_noise(loud, *loud_level, true);
    game_context loud_ctx = {*loud_level, loud, false};
    simulate(*hunter, loud_ctx, 0.1f);
    check(hunter->get_state() == ALERTED, "слухач замечает бегущего героя");
    simulate(*hunter, loud_ctx, 0.7f);
    check(hunter->get_state() == CHASE, "после паузы начинается погоня");
    simulate(*hunter, loud_ctx, 20.0f);
    check(loud.is_caught(), "монстр догоняет шумного героя");
    delete l;
    delete loud_level;
}

void test_chase_ends_in_search_and_patrol() {
    level *l = create_level(1);
    loads(l, map_path(1));
    enemy *e = nearest_enemy(l, l->get_spawn());
    player hero(l->get_spawn());
    make_noise(hero, *l, true);
    game_context ctx = {*l, hero, false};
    simulate(*e, ctx, 1.0f);
    check(e->get_state() == CHASE, "погоня началась");

    // герой убежал в дальнюю комнату и затих
    player escaped(cell_center(l->get_exit()));
    game_context escaped_ctx = {*l, escaped, false};
    simulate(*e, escaped_ctx, 3.5f);
    check(e->get_state() == LOST, "герой затих: монстр теряет след");
    simulate(*e, escaped_ctx, 1.2f);
    check(e->get_state() == SEARCH, "затем ищет");
    simulate(*e, escaped_ctx, 6.5f);
    check(e->get_state() == PATROL, "затем возвращается в патруль");
    delete l;
}

// герой стоит у края клетки: монстр должен подойти вплотную и поймать, а не встать в центре клетки
void test_catches_player_off_center() {
    level *l = create_level(1);
    loads(l, map_path(1));
    enemy *e = nearest_enemy(l, l->get_spawn());
    Vector3 near = free_spot(l, e->get_position(), 2);
    near.x += CELL * 0.35f;
    near.z += CELL * 0.35f;
    player hero(near);
    make_noise(hero, *l, true);
    game_context ctx = {*l, hero, false};
    simulate(*e, ctx, 1.0f);
    check(e->get_state() == CHASE, "монстр погнался за героем у края клетки");

    player still(near);
    game_context still_ctx = {*l, still, false};
    simulate(*e, still_ctx, 10.0f);
    check(still.is_caught(), "монстр ловит героя, который замер у края клетки");
    delete l;
}

void test_walking_is_quiet() {
    level *l = create_level(1);
    loads(l, map_path(1));
    enemy *e = nearest_enemy(l, l->get_spawn());
    Vector3 close = free_spot(l, e->get_position(), 2);
    player hero(close);
    make_noise(hero, *l, false);
    game_context ctx = {*l, hero, false};
    simulate(*e, ctx, 0.5f);
    check(e->get_state() == PATROL, "слухач не слышит шаги героя в 4 м");
    delete l;
}

// монстр не должен замирать, выбрав цель за запертой дверью
void test_patrol_never_freezes() {
    level *l = create_level(1);
    loads(l, map_path(1));
    enemy *e = l->get_enemy(0);
    player hidden(cell_center(l->get_exit()));
    game_context ctx = {*l, hidden, false};
    simulate(*e, ctx, 600.0f);
    Vector3 before = e->get_position();
    simulate(*e, ctx, 30.0f);
    check(Vector3Distance(before, e->get_position()) > 0.0f, "монстр не замирает в патруле");
    delete l;
}

void test_light_hunter_ignores_dark_player() {
    level *l = create_level(2);
    loads(l, map_path(2));
    // берём монстра, рядом с которым есть 3 свободные клетки по прямой
    enemy *e = l->get_enemy(0);
    Vector3 close = free_spot(l, e->get_position(), 3);
    for (int i = 1; i < l->get_enemy_count() && flat_distance(close, e->get_position()) < 1.0f; i++) {
        e = l->get_enemy(i);
        close = free_spot(l, e->get_position(), 3);
    }
    player hero(close);   // фонарик в начале выключен
    game_context ctx = {*l, hero, false};
    simulate(*e, ctx, 0.05f);
    check(e->get_state() == PATROL, "без фонарика охотник не реагирует");

    toggle_flashlight(hero, *l);
    simulate(*e, ctx, 0.05f);
    check(e->get_state() == ALERTED, "на свет фонарика реагирует");
    delete l;
}

void test_records() {
    const char *path = "test_records.txt";
    remove(path);
    record list[MAX_RECORDS];
    check(load_records(path, list) == 0, "без файла рекордов таблица пустая");

    add_record(path, record(300, 400.0f));
    add_record(path, record(500, 450.5f));
    add_record(path, record(500, 420.0f));
    int n = load_records(path, list);
    check(n == 3, "рекорды сохраняются в файл");
    check(list[0].get_score() == 500 && list[0].get_seconds() == 420.0f,
          "лучший результат наверху, при равных очках быстрее лучше");
    check(list[2].get_score() == 300, "худший результат внизу");

    for (int i = 0; i < MAX_RECORDS + 2; i++)
        add_record(path, record(i, 100.0f));
    check(load_records(path, list) == MAX_RECORDS, "хранится не больше 10 рекордов");

    ofstream f(path);
    f << "  Очки     Время\nабв где\n";
    f.close();
    bool thrown = false;
    try {
        load_records(path, list);
    }
    catch (const char *msg) {
        thrown = true;
    }
    check(thrown, "испорченный файл рекордов даёт исключение");
    remove(path);
}

int main() {
    for (int number = 1; number <= 3; number++)
        test_map(number);
    test_broken_maps();
    test_door_needs_its_key();
    test_lifts_are_decoration();
    test_collision();
    test_items();
    test_energy_drink();
    test_tired_player_walks();
    test_listener_hears_noise();
    test_chase_ends_in_search_and_patrol();
    test_patrol_never_freezes();
    test_catches_player_off_center();
    test_walking_is_quiet();
    test_light_hunter_ignores_dark_player();
    test_records();
    cout << endl << "Ошибок: " << failures << endl;
    if (failures == 0)
        return 0;
    return 1;
}
