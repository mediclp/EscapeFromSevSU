// item.h - предметы, которые герой подбирает на уровне
#pragma once
#include <string>
#include "game_object.h"
using namespace std;

// абстрактный предмет: подбор общий, а эффект у каждого наследника свой
class item : public game_object {
public:
    item(Vector3 p) : game_object(p) {}

    void draw(renderer &r);

    // применяет предмет к герою, даёт очки и убирает предмет.
    // возвращает текст для экрана
    string pick_up(player &p);

    // название в винительном падеже для подсказки «взять ...»
    virtual string name() = 0;

    // эффект предмета, возвращает текст для экрана
    virtual string apply_to(player &p) = 0;

    // сколько очков даёт предмет
    virtual int points() = 0;

    // цвет кубика на уровне
    virtual Color color() = 0;
};

// название ключа по его букве: a, b, v - ключи от лестниц корпусов А, Б, В; c - электронный пропуск
string key_title(char id);

// ключ от одной двери: от лестницы своего корпуса или пропуск для турникета
class key : public item {
private:
    char id;   // какую дверь открывает

public:
    key(Vector3 p, char k) : item(p) { id = k; }
    string name() { return key_title(id); }
    string apply_to(player &p);
    int points() { return 50; }
    Color color();
};

// батарейка: заряжает фонарик
class battery : public item {
public:
    battery(Vector3 p) : item(p) {}
    string name() { return "батарейку"; }
    string apply_to(player &p);
    int points() { return 10; }
    Color color() { return LIME; }
};

// энергетик: восстанавливает выносливость
class energy_drink : public item {
public:
    energy_drink(Vector3 p) : item(p) {}
    string name() { return "энергетик"; }
    string apply_to(player &p);
    int points() { return 10; }
    Color color() { return SKYBLUE; }
};

// записка: рассказывает часть сюжета
class note : public item {
private:
    string text;

public:
    note(Vector3 p, string t) : item(p) { text = t; }
    string name() { return "записку"; }
    string apply_to(player &p) { return text; }
    int points() { return 20; }
    Color color() { return RAYWHITE; }
};
