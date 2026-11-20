// enemy.cpp - общая логика монстров: состояния, движение, поимка героя
#include "enemy.h"
#include "level.h"
#include "player.h"
#include "renderer.h"
#include "raymath.h"

const float ALERT_TIME = 0.6f;       // замирает, заметив героя
const float FORGET_TIME = 3.0f;      // сколько гонится вслепую
const float LOST_TIME = 1.0f;        // стоит, потеряв след
const float SEARCH_TIME = 6.0f;      // ищет вокруг последней точки
const int SEARCH_RADIUS = 3;         // в скольких клетках от неё ищет
const float TOUCH_DISTANCE = 1.5f;   // вплотную героя замечают все
const float CATCH_DISTANCE = 0.8f;
const float ARRIVE_DISTANCE = 0.05f;
const float ALARM_SPEEDUP = 1.1f;    // во время тревоги монстры быстрее

enemy::enemy(Vector3 p) : game_object(p) {
    state = PATROL;
    state_time = 0.0f;
    unseen_time = 0.0f;
    goal = cell_at(p);
    waypoint = goal;
}

void enemy::update(float dt, game_context &ctx) {
    update_state(dt, ctx);
    walk_to_goal(dt, ctx, current_speed(ctx.alarm));

    // проверка, поймал ли монстр героя
    if (flat_distance(position, ctx.hero.get_position()) < CATCH_DISTANCE)
        ctx.hero.catch_player();
}

bool enemy::notices(game_context &ctx) {
    return flat_distance(position, ctx.hero.get_position()) < TOUCH_DISTANCE;
}

void enemy::update_state(float dt, game_context &ctx) {
    state_time += dt;
    bool noticed = notices(ctx);   // вызовется версия нужного монстра
    grid_map &m = ctx.lvl.get_map();
    bool at_goal = cell_at(position) == goal;

    switch (state) {
    case PATROL:
        if (noticed)
            change_state(ALERTED);
        else if (at_goal)
            goal = m.random_floor_cell();
        break;
    case ALERTED:
        goal = cell_at(ctx.hero.get_position());
        if (state_time >= ALERT_TIME)
            change_state(CHASE);
        break;
    case CHASE:
        if (noticed) {
            unseen_time = 0.0f;
            goal = cell_at(ctx.hero.get_position());
        } else {
            unseen_time += dt;
        }
        if (unseen_time >= FORGET_TIME)
            change_state(LOST);
        break;
    case LOST:
        if (noticed)
            change_state(CHASE);
        else if (state_time >= LOST_TIME)
            change_state(SEARCH);
        break;
    case SEARCH:
        if (noticed)
            change_state(CHASE);
        else if (state_time >= SEARCH_TIME)
            change_state(PATROL);
        else if (at_goal)
            goal = m.random_cell_near(goal, SEARCH_RADIUS);
        break;
    }
}

void enemy::change_state(enemy_state s) {
    state = s;
    state_time = 0.0f;
    unseen_time = 0.0f;
}

float enemy::current_speed(bool alarm) {
    if (state == ALERTED || state == LOST)
        return 0.0f;
    float speed = state == CHASE ? chase_speed() : patrol_speed();
    if (alarm)
        speed *= ALARM_SPEEDUP;
    return speed;
}

void enemy::walk_to_goal(float dt, game_context &ctx, float speed) {
    // в погоне, оказавшись в одной клетке с героем, идём прямо к нему, а не в центр клетки,
    // иначе монстр замрёт рядом с героем, который стоит у края клетки
    Vector3 hero = ctx.hero.get_position();
    if (state == CHASE && cell_at(position) == cell_at(hero)) {
        hero.y = position.y;
        position = Vector3MoveTowards(position, hero, speed * dt);
        return;
    }

    Vector3 target = cell_center(waypoint);
    target.y = position.y;
    if (Vector3Distance(position, target) < ARRIVE_DISTANCE) {
        cell current = cell_at(position);
        waypoint = ctx.lvl.get_map().next_step(current, goal);
        // цель за запертой дверью недостижима: бросаем её, иначе монстр замрёт
        if (waypoint == current)
            goal = current;
        target = cell_center(waypoint);
        target.y = position.y;
    }
    position = Vector3MoveTowards(position, target, speed * dt);
}

void enemy::draw(renderer &r) {
    Vector3 body = {position.x, 0.85f, position.z};
    Vector3 head = {position.x, 1.85f, position.z};
    Vector3 body_size = {0.7f, 1.7f, 0.5f};
    Vector3 head_size = {0.4f, 0.4f, 0.4f};
    r.draw_box(body, body_size, color());
    r.draw_box(head, head_size, color());
}
