// flashlight.cpp - фонарик
#include "flashlight.h"
#include "clamp.h"

const float BATTERY_DRAIN = 1.0f;   // сколько процентов заряда уходит за секунду

void flashlight::add_charge(float amount) {
    charge = clamp_value(charge + amount, 0.0f, 100.0f);
}

void flashlight::update(float dt) {
    if (light_on())
        charge = clamp_value(charge - BATTERY_DRAIN * dt, 0.0f, 100.0f);
}
