// player.h - герой: движение, выносливость, ключи и очки
#pragma once
#include "game_object.h"
#include "flashlight.h"

const float PLAYER_RADIUS = 0.3f;   // радиус столкновений героя в метрах

// команды героя на один шаг. ввод отделён от логики,
// чтобы героя можно было проверять тестами без клавиатуры
struct player_input {
    Vector2 look;   // смещение мыши
    Vector2 move;   // x - вправо, y - вперёд
    bool run;
    bool toggle_flashlight;
};

// герой - объект игры и одновременно фонарик (множественное наследование)
class player : public game_object, public flashlight {
private:
    float yaw, pitch;   // поворот взгляда влево-вправо и вверх-вниз
    float stamina;
    int keys;
    int score;
    bool moving, running, caught;

    // поворачивает взгляд на смещение мыши
    void look(Vector2 delta);

    // двигает героя, тратит или восстанавливает выносливость
    void walk(player_input &in, float dt, game_context &ctx);

public:
    player(Vector3 p);

    // читает клавиатуру и мышь и выполняет команды
    void update(float dt, game_context &ctx);

    // выполняет команды героя за шаг dt секунд
    void apply_input(player_input &in, float dt, game_context &ctx);

    // героя не рисуем: игра от первого лица
    void draw(renderer &r) {}

    // где глаза героя и куда он смотрит, нужно камере
    Vector3 eye();
    Vector3 forward();

    // на каком расстоянии слышно героя: зависит от того, идёт он или бежит
    float noise_radius();

    // восстанавливает выносливость, но не больше 100
    void add_stamina(float amount);

    // начисляет очки: hero += 50
    player& operator+=(int points) {
        score += points;
        return *this;
    }

    void add_key() { keys++; }

    // монстр догнал героя
    void catch_player() { caught = true; }

    float get_stamina() { return stamina; }
    int get_keys() { return keys; }
    int get_score() { return score; }
    bool is_caught() { return caught; }
};
