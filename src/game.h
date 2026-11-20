// game.h - главный класс игры
#pragma once
#include <string>
#include "level.h"
#include "player.h"
#include "renderer.h"
#include "record.h"
using namespace std;

// экраны игры
enum game_state { MENU, HELP, PLAYING, PAUSED, CAUGHT, VICTORY, LOAD_ERROR };

// главный цикл, меню, переключение уровней, очки, рекорды и воздушная тревога
class game {
private:
    renderer *rend;
    level *lvl;
    player *hero;
    Font font;
    game_state state;
    int level_number;
    int total_score;       // очки за пройденные уровни
    float play_time;       // сколько секунд идёт игра
    bool quit;
    bool peaceful;         // осмотр карт без монстров
    bool alarm_active;
    float alarm_time;      // сколько длится текущая фаза тревоги
    string message;        // сообщение на экране
    float message_time;    // сколько его ещё показывать
    string error_text;     // что не так с картой
    record records[MAX_RECORDS];
    int record_count;
    string records_error;  // что не так с файлом рекордов

    // загружает шрифт с кириллицей
    void load_font();

    // во сколько раз увеличить интерфейс: 1 для высоты 720, 1.5 для 1080
    float ui_scale();

    // сбрасывает счёт и начинает игру с первого уровня;
    // peaceful - осмотр карт: без монстров и со всеми ключами
    void start_new_game(bool p);

    // загружает уровень и создаёт героя
    void start_level(int number);

    // начисляет очки за уровень и переходит к следующему или к победе
    void finish_level();

    // меняет экран и показывает или прячет курсор
    void set_state(game_state s);

    // где лежит файл рекордов и как его прочитать
    string records_path();
    void load_record_table();

    // логика одного кадра
    void update(float dt);
    void update_playing(float dt);
    void update_alarm(float dt);

    // отрисовка одного кадра
    void draw();
    void draw_hud();
    void draw_help();
    void draw_records();
    void draw_screen(string title, string hint);
    void draw_centered_text(string text, float y, float size, Color color);

public:
    // открывает окно и загружает ресурсы
    game();

    // освобождает ресурсы и закрывает окно
    ~game();

    // крутит главный цикл до выхода из игры
    void run();
};
