// player.cpp - герой
#include "player.h"
#include <cmath>
#include "level.h"
#include "clamp.h"
#include "raymath.h"

const float EYE_HEIGHT = 1.6f;
const float WALK_SPEED = 2.6f;   // м/с
const float RUN_SPEED = 4.6f;
const float MOUSE_SENSITIVITY = 0.0025f;
const float MAX_PITCH = 1.4f;    // насколько можно задрать или опустить голову
const float STAMINA_DRAIN = 20.0f;   // бег без остановки - 5 секунд
const float STAMINA_REGEN = 15.0f;
const float TIRED_RECOVER = 30.0f;   // после усталости бег возвращается с 30% выносливости
const float WALK_NOISE = 1.0f;   // радиус шума в метрах: шаги слышно только вплотную
const float RUN_NOISE = 11.0f;

// собирает команды героя с клавиатуры и мыши
player_input read_input() {
    player_input in;
    in.look = GetMouseDelta();
    in.move.x = 0.0f;
    in.move.y = 0.0f;
    if (IsKeyDown(KEY_W)) in.move.y += 1.0f;
    if (IsKeyDown(KEY_S)) in.move.y -= 1.0f;
    if (IsKeyDown(KEY_D)) in.move.x += 1.0f;
    if (IsKeyDown(KEY_A)) in.move.x -= 1.0f;
    in.run = IsKeyDown(KEY_LEFT_SHIFT);
    in.toggle_flashlight = IsKeyPressed(KEY_F);
    return in;
}

player::player(Vector3 p) : game_object(p), flashlight() {
    yaw = PI;   // смотрит вверх по карте: старт на всех уровнях внизу

    pitch = 0.0f;
    stamina = 100.0f;
    keys = "";
    score = 0;
    moving = false;
    running = false;
    tired = false;
    boost_time = 0.0f;
    speed_scale = 1.0f;
    caught = false;
}

void player::update(float dt, game_context &ctx) {
    player_input in = read_input();
    apply_input(in, dt, ctx);
}

void player::apply_input(player_input &in, float dt, game_context &ctx) {
    look(in.look);
    walk(in, dt, ctx);
    if (in.toggle_flashlight)
        toggle();
    // update есть в обеих базах, поэтому явно указываем, чей вызываем
    flashlight::update(dt);
}

void player::look(Vector2 delta) {
    yaw -= delta.x * MOUSE_SENSITIVITY;
    pitch = clamp_value(pitch - delta.y * MOUSE_SENSITIVITY, -MAX_PITCH, MAX_PITCH);
}

void player::walk(player_input &in, float dt, game_context &ctx) {
    Vector3 front = {sin(yaw), 0.0f, cos(yaw)};
    Vector3 side = {-front.z, 0.0f, front.x};
    Vector3 direction = Vector3Add(Vector3Scale(front, in.move.y), Vector3Scale(side, in.move.x));

    moving = Vector3LengthSqr(direction) > 0.0f;
    running = moving && in.run && !tired;

    float change = running ? -STAMINA_DRAIN : STAMINA_REGEN;
    stamina = clamp_value(stamina + change * dt, 0.0f, 100.0f);
    // после энергетика выносливость держится на 100%
    if (boost_time > 0.0f) {
        boost_time = clamp_value(boost_time - dt, 0.0f, boost_time);
        stamina = 100.0f;
    }
    // выносливость кончилась - герой выдохся и идёт шагом, пока не отдышится до 30%
    if (stamina <= 0.0f)
        tired = true;
    else if (stamina >= TIRED_RECOVER)
        tired = false;
    if (!moving)
        return;

    float speed = (running ? RUN_SPEED : WALK_SPEED) * speed_scale;
    Vector3 step = Vector3Scale(Vector3Normalize(direction), speed * dt);

    // оси проверяем отдельно, чтобы герой скользил вдоль стен
    grid_map &m = ctx.lvl.get_map();
    if (!m.collides(position.x + step.x, position.z, PLAYER_RADIUS))
        position.x += step.x;
    if (!m.collides(position.x, position.z + step.z, PLAYER_RADIUS))
        position.z += step.z;
}

Vector3 player::eye() {
    Vector3 e = {position.x, EYE_HEIGHT, position.z};
    return e;
}

Vector3 player::forward() {
    Vector3 f = {cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw)};
    return f;
}

float player::noise_radius() {
    if (running)
        return RUN_NOISE;
    if (moving)
        return WALK_NOISE;
    return 0.0f;
}

bool player::has_key(char id) {
    for (int i = 0; i < (int)keys.length(); i++)
        if (keys[i] == id)
            return true;
    return false;
}

void player::drink_energy(float seconds) {
    stamina = 100.0f;
    tired = false;
    boost_time = seconds;
}
