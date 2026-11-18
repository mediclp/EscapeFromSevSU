// level3.cpp - как вахтёр замечает героя
#include "level3.h"
#include "player.h"

const float SIGHT_RANGE = 10.0f;   // с какого расстояния вахтёр видит героя

bool warden_enemy::notices(game_context &ctx) {
    float d = flat_distance(position, ctx.hero.get_position());
    if (enemy::notices(ctx) || d < ctx.hero.noise_radius())
        return true;
    return d < SIGHT_RANGE && ctx.lvl.get_map().line_of_sight(position, ctx.hero.get_position());
}
