// door.h - дверь, которую открывает свой ключ
#pragma once
#include <string>
#include "game_object.h"
using namespace std;

// дверь на лестницу корпуса открывается ключом от этой лестницы,
// турникет на выходе - электронным пропуском
class door : public game_object {
private:
    char need_key;   // какой ключ нужен: a, b, v - лестницы корпусов А, Б, В; c - пропуск
    bool opened;

public:
    door(Vector3 p, char k) : game_object(p) {
        need_key = k;
        opened = false;
    }

    void draw(renderer &r);

    // пытается открыть дверь, возвращает текст для экрана
    string open(player &p);

    // что сделать по кнопке E: открыть дверь или пройти через турникет
    string action();

    bool is_open() { return opened; }
};

// какой ключ открывает дверь с этим символом карты: 1 -> a, 2 -> b, 3 -> v, T -> c
char door_key(char symbol);
