// clamp.h - шаблон, который держит значение в заданных пределах
#pragma once

// возвращает x, но не меньше low и не больше high.
// работает с float (выносливость, заряд, взгляд) и с int (цвет, бонус за время)
template <class T>
T clamp_value(T x, T low, T high) {
    if (x < low)
        return low;
    if (x > high)
        return high;
    return x;
}
