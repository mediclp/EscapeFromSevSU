// level1.cpp - как слухач замечает героя
#include "level1.h"
#include "player.h"

const float ALARM_HEARING = 1.3f;   // во время сирены слух острее

bool listener_enemy::notices(game_context &ctx) {
    float d = flat_distance(position, ctx.hero.get_position());
    float hearing = ctx.hero.noise_radius();
    if (ctx.alarm)
        hearing *= ALARM_HEARING;
    // сначала общая проверка из базового класса
    return enemy::notices(ctx) || d < hearing;
}
