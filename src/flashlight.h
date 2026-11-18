// flashlight.h - фонарик: заряд, включение и разряд
#pragma once

// фонарик героя. вторая база класса player
class flashlight {
protected:
    float charge;   // заряд в процентах
    bool on;

public:
    flashlight() {
        charge = 100.0f;
        on = true;
    }

    // включает или выключает фонарик
    void toggle() { on = !on; }

    // светит, только если включён и не разряжен
    bool light_on() { return on && charge > 0.0f; }

    // заряжает батарею, но не больше 100
    void add_charge(float amount);

    // разряжает батарею, пока фонарик горит
    void update(float dt);

    float get_charge() { return charge; }
};
