// game.cpp - главный цикл, экраны, уровни, очки, рекорды и тревога
#include "game.h"
#include <cmath>
#include "clamp.h"

const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 540;
const float UI_HEIGHT = 720.0f;      // интерфейс задан для высоты 720, на других экранах масштабируется
const int FONT_LOAD_SIZE = 64;       // шрифт грузится крупным, чтобы не мылился на большом экране
const int LEVEL_COUNT = 3;
const int LEVEL_SCORE = 100;         // очки за пройденный уровень
const int TIME_LIMIT = 600;          // за каждую секунду быстрее - одно очко
const float ALARM_PAUSE = 30.0f;     // тишина между сиренами
const float ALARM_DURATION = 12.0f;
const float ALARM_PULSE = 4.0f;      // как быстро мигает красный свет
const float MESSAGE_TIME = 3.5f;
const float FONT_SIZE = 32.0f;
const int SHOWN_RECORDS = 5;
const float PEACEFUL_SPEED = 3.0f;   // в режиме осмотра герой ходит втрое быстрее         // сколько рекордов видно в меню

// руководство пользователя на экране справки
const int HELP_COUNT = 16;
const string HELP_LINES[HELP_COUNT] = {
    "Ночь, воздушная тревога, света нет. Выберись из университета.",
    "Этаж 1: найди электронный пропуск и пройди через турникет.",
    "Этаж 2 и подвал: найди любой ключ от лестницы и выйди по ней.",
    "Ключ от лестницы корпуса лежит в другом корпусе. Поймают -",
    "уровень заново.",
    "",
    "W A S D  — ходьба            Мышь — обзор",
    "Shift    — бег, тратит выносливость",
    "F        — фонарик, тратит заряд",
    "E        — взять предмет или открыть дверь",
    "Esc      — пауза             F11 — во весь экран",
    "",
    "Этаж 1: монстры слышат бег. Ходи шагом.",
    "Этаж 2: монстры идут на свет. Выключай фонарик.",
    "Подвал: вахтёры видят вблизи. Прячься и убегай.",
    "Во время сирены монстры быстрее."};

game::game() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Escape from SevSU");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    load_font();
    rend = new renderer();
    lvl = NULL;
    hero = NULL;
    level_number = 1;
    total_score = 0;
    play_time = 0.0f;
    quit = false;
    peaceful = false;
    alarm_active = false;
    alarm_time = 0.0f;
    message_time = 0.0f;
    record_count = 0;
    set_state(MENU);
}

game::~game() {
    delete hero;
    delete lvl;
    delete rend;   // текстуры нужно выгрузить до закрытия окна
    UnloadFont(font);
    CloseWindow();
}

// в шрифте raylib нет кириллицы, поэтому грузим свой
void game::load_font() {
    int codepoints[200];
    int n = 0;
    for (int code = 32; code < 127; code++) {   // латиница, цифры, знаки
        codepoints[n] = code;
        n++;
    }
    for (int code = 0x400; code < 0x460; code++) {   // кириллица
        codepoints[n] = code;
        n++;
    }
    codepoints[n] = 0x2014;   // тире
    codepoints[n + 1] = 0xAB;   // кавычки-ёлочки
    codepoints[n + 2] = 0xBB;
    n += 3;
    font = LoadFontEx(TextFormat("%sassets/fonts/DejaVuSansMono.ttf", GetApplicationDirectory()),
                      FONT_LOAD_SIZE, codepoints, n);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);   // сглаживание при уменьшении
}

float game::ui_scale() {
    return GetScreenHeight() / UI_HEIGHT;
}

void game::run() {
    while (!quit && !WindowShouldClose()) {
        update(GetFrameTime());
        draw();
    }
}

void game::start_new_game(bool p) {
    peaceful = p;
    total_score = 0;
    play_time = 0.0f;
    start_level(1);
}

void game::start_level(int number) {
    level_number = number;
    delete hero;
    hero = NULL;
    delete lvl;
    lvl = create_level(number);

    try {
        lvl->load(TextFormat("%sassets/maps/level%d.txt", GetApplicationDirectory(), number));
    }
    catch (const char *msg) {
        error_text = msg;
        set_state(LOAD_ERROR);
        return;
    }

    hero = new player(lvl->get_spawn());
    alarm_active = false;
    alarm_time = 0.0f;
    message = TextFormat("Уровень %d: %s", number, lvl->name().c_str());
    if (peaceful) {
        // осмотр карты: монстров нет, все двери и турникет открываются
        lvl->remove_enemies();
        hero->add_key('a');
        hero->add_key('b');
        hero->add_key('v');
        hero->add_key('c');
        hero->set_speed_scale(PEACEFUL_SPEED);
        message += " (осмотр: 1, 2, 3 - другой уровень)";
    }
    message_time = MESSAGE_TIME;
    set_state(PLAYING);
}

void game::finish_level() {
    total_score += hero->get_score() + LEVEL_SCORE;
    if (level_number < LEVEL_COUNT) {
        start_level(level_number + 1);
        return;
    }
    // бонус за скорость: чем быстрее, тем больше, но не меньше нуля
    total_score += clamp_value(TIME_LIMIT - (int)play_time, 0, TIME_LIMIT);

    if (!peaceful) {   // осмотр карт в рекорды не идёт
        try {
            add_record(records_path(), record(total_score, play_time));
        }
        catch (const char *msg) {
            records_error = msg;
        }
    }
    set_state(VICTORY);
}

void game::set_state(game_state s) {
    state = s;
    if (s == PLAYING)
        DisableCursor();
    else
        EnableCursor();
    if (s == MENU)
        load_record_table();
}

string game::records_path() {
    return string(GetApplicationDirectory()) + "records.txt";
}

void game::load_record_table() {
    records_error = "";
    try {
        record_count = load_records(records_path(), records);
    }
    catch (const char *msg) {
        record_count = 0;
        records_error = msg;
    }
}

void game::update(float dt) {
    // F11 работает на любом экране: окно без рамки на весь монитор и обратно
    if (IsKeyPressed(KEY_F11))
        ToggleBorderlessWindowed();

    switch (state) {
    case MENU:
        if (IsKeyPressed(KEY_ENTER)) start_new_game(false);
        if (IsKeyPressed(KEY_T)) start_new_game(true);
        if (IsKeyPressed(KEY_H)) set_state(HELP);
        if (IsKeyPressed(KEY_ESCAPE)) quit = true;
        break;
    case HELP:
        if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ESCAPE)) set_state(MENU);
        break;
    case PLAYING:
        if (IsKeyPressed(KEY_ESCAPE))
            set_state(PAUSED);
        else
            update_playing(dt);
        break;
    case PAUSED:
        if (IsKeyPressed(KEY_ESCAPE)) set_state(PLAYING);
        if (IsKeyPressed(KEY_Q)) set_state(MENU);
        break;
    case CAUGHT:
        if (IsKeyPressed(KEY_ENTER)) start_level(level_number);
        break;
    case VICTORY:
        if (IsKeyPressed(KEY_ENTER)) set_state(MENU);
        break;
    case LOAD_ERROR:
        if (IsKeyPressed(KEY_ESCAPE)) quit = true;
        break;
    }
}

void game::update_playing(float dt) {
    // при осмотре карт клавиши 1, 2, 3 переносят на нужный уровень
    if (peaceful) {
        for (int n = 1; n <= LEVEL_COUNT; n++)
            if (IsKeyPressed(KEY_ZERO + n)) {
                start_level(n);
                return;
            }
    }
    play_time += dt;
    update_alarm(dt);
    message_time -= dt;
    if (message_time < 0.0f)
        message_time = 0.0f;

    game_context ctx = {*lvl, *hero, alarm_active};
    hero->update(dt, ctx);
    lvl->update(dt, ctx);

    if (IsKeyPressed(KEY_E)) {
        string text = lvl->interact(*hero);
        if (text != "") {
            message = text;
            message_time = MESSAGE_TIME;
        }
    }

    if (hero->is_caught())
        set_state(CAUGHT);
    else if (lvl->is_exit(hero->get_position()))
        finish_level();
}

void game::update_alarm(float dt) {
    alarm_time += dt;
    float phase_length = alarm_active ? ALARM_DURATION : ALARM_PAUSE;
    if (alarm_time >= phase_length) {
        alarm_active = !alarm_active;
        alarm_time = 0.0f;
    }
}

void game::draw() {
    bool has_world = state == PLAYING || state == PAUSED || state == CAUGHT;
    if (has_world) {
        float alarm = 0.0f;
        if (alarm_active)
            alarm = 0.5f + 0.5f * sin(alarm_time * ALARM_PULSE);
        rend->begin_scene(*hero, alarm);
        lvl->draw(*rend);
        rend->end_scene();
    }

    BeginDrawing();
    ClearBackground(BLACK);
    if (has_world)
        rend->present();

    switch (state) {
    case MENU:
        draw_screen("ПОБЕГ ИЗ СЕВГУ", "Enter — начать, T — осмотр карт, H — управление, Esc — выход");
        draw_records();
        break;
    case HELP:
        draw_help();
        break;
    case PLAYING:
        draw_hud();
        break;
    case PAUSED:
        draw_screen("ПАУЗА", "Esc — продолжить, Q — в меню");
        break;
    case CAUGHT:
        draw_screen("ТЕБЯ ПОЙМАЛИ", "Enter — начать уровень заново");
        break;
    case VICTORY:
        draw_screen("ТЫ ВЫБРАЛСЯ",
                    TextFormat("Очки: %d   Время: %d с   Enter — в меню", total_score, (int)play_time));
        break;
    case LOAD_ERROR:
        draw_screen("ОШИБКА", "Не удалось загрузить карту. Esc — выход");
        draw_centered_text(error_text, GetScreenHeight() * 0.6f, FONT_SIZE * 0.68f * ui_scale(), RED);
        break;
    }
    EndDrawing();
}

void game::draw_hud() {
    float k = ui_scale();
    float small_text = FONT_SIZE * 0.75f * k;
    string status = TextFormat("Фонарик %d%%  Выносливость %d%%%s  Очки %d  Ключи:",
                               (int)hero->get_charge(), (int)hero->get_stamina(),
                               hero->is_tired() ? " (выдохся)"
                               : hero->get_boost_time() > 0.0f ? TextFormat(" (энергетик %d с)", (int)hero->get_boost_time() + 1)
                               : "",
                               total_score + hero->get_score());
    Vector2 corner = {16.0f * k, 12.0f * k};
    DrawTextEx(font, status.c_str(), corner, small_text, 1.0f, RAYWHITE);

    // ключи уровня: зелёный - найден, красный - ещё нет.
    // на 1 этаже нужен пропуск, на остальных - ключи от лестниц корпусов
    const int KEY_COUNT = 3;
    char ids[KEY_COUNT] = {'a', 'b', 'v'};
    const char *names[KEY_COUNT] = {"А", "Б", "В"};
    int count = KEY_COUNT;
    if (level_number == 1) {
        ids[0] = 'c';
        names[0] = "пропуск";
        count = 1;
    }
    corner.x += MeasureTextEx(font, status.c_str(), small_text, 1.0f).x;
    for (int i = 0; i < count; i++) {
        string label = string(" ") + names[i];
        DrawTextEx(font, label.c_str(), corner, small_text, 1.0f, hero->has_key(ids[i]) ? GREEN : RED);
        corner.x += MeasureTextEx(font, label.c_str(), small_text, 1.0f).x;
    }

    string hint = lvl->prompt(*hero);
    if (hint != "")
        draw_centered_text(hint, GetScreenHeight() * 0.72f, small_text, RAYWHITE);
    if (message_time > 0.0f)
        draw_centered_text(message, GetScreenHeight() * 0.8f, small_text, GOLD);
    if (alarm_active)
        draw_centered_text("ВОЗДУШНАЯ ТРЕВОГА", (12.0f + FONT_SIZE) * k, small_text, RED);
}

void game::draw_help() {
    float k = ui_scale();
    float text_size = FONT_SIZE * 0.68f * k;
    draw_centered_text("КАК ИГРАТЬ", 40.0f * k, FONT_SIZE * 1.25f * k, RAYWHITE);
    float y = 120.0f * k;
    for (int i = 0; i < HELP_COUNT; i++) {
        Vector2 p = {40.0f * k, y};
        DrawTextEx(font, HELP_LINES[i].c_str(), p, text_size, 1.0f, LIGHTGRAY);
        y += text_size * 1.45f;
    }
    draw_centered_text("Esc — назад", GetScreenHeight() - 50.0f * k, FONT_SIZE * 0.75f * k, RAYWHITE);
}

void game::draw_records() {
    float size = FONT_SIZE * 0.68f * ui_scale();
    float y = GetScreenHeight() * 0.62f;
    if (records_error != "") {
        draw_centered_text(records_error, y, size, RED);
        return;
    }
    if (record_count == 0)
        return;

    draw_centered_text("Рекорды", y, size, RAYWHITE);
    for (int i = 0; i < record_count && i < SHOWN_RECORDS; i++) {
        y += size * 1.3f;
        draw_centered_text(TextFormat("%d. %d очков, %.1f с", i + 1, records[i].get_score(),
                                      records[i].get_seconds()),
                           y, size, LIGHTGRAY);
    }
}

void game::draw_screen(string title, string hint) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
    draw_centered_text(title, GetScreenHeight() * 0.38f, FONT_SIZE * 1.5f * ui_scale(), RAYWHITE);
    draw_centered_text(hint, GetScreenHeight() * 0.52f, FONT_SIZE * 0.75f * ui_scale(), LIGHTGRAY);
}

void game::draw_centered_text(string text, float y, float size, Color color) {
    Vector2 measured = MeasureTextEx(font, text.c_str(), size, 1.0f);
    Vector2 p = {(GetScreenWidth() - measured.x) / 2.0f, y};
    DrawTextEx(font, text.c_str(), p, size, 1.0f, color);
}
