// level2.cpp - как охотник на свет замечает героя
#include "level2.h"
#include "player.h"

const float LIGHT_RANGE = 12.0f;   // с какого расстояния виден фонарик

bool light_hunter_enemy::notices(game_context &ctx) {
    if (enemy::notices(ctx))
        return true;
    float d = flat_distance(position, ctx.hero.get_position());
    return ctx.hero.light_on() && d < LIGHT_RANGE &&
           ctx.lvl.get_map().line_of_sight(position, ctx.hero.get_position());
}
