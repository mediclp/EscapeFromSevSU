// door.h - дверь с замком
#pragma once
#include <string>
#include "game_object.h"
using namespace std;

// дверь открывается, когда собраны все ключи уровня
class door : public game_object {
private:
    int need_keys;   // сколько ключей нужно
    bool opened;

public:
    door(Vector3 p, int k) : game_object(p) {
        need_keys = k;
        opened = false;
    }

    void draw(renderer &r);

    // пытается открыть дверь, возвращает текст для экрана
    string open(player &p);

    bool is_open() { return opened; }
};
