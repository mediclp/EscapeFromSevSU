// record.h - таблица рекордов: результат одной игры и работа с файлом
#pragma once
#include <iostream>
#include <string>
using namespace std;

const int MAX_RECORDS = 10;   // сколько лучших результатов хранится

// результат пройденной игры: очки и время
class record {
private:
    int score;
    float seconds;

public:
    record() {
        score = 0;
        seconds = 0.0f;
    }
    record(int s, float t) {
        score = s;
        seconds = t;
    }

    int get_score() { return score; }
    float get_seconds() { return seconds; }

    // хуже ли этот результат: меньше очков, а при равных очках больше время
    bool operator<(record &ob);

    // запись и чтение строки таблицы, друзьям нужны закрытые поля
    friend ostream& operator<<(ostream &out, record &ob);
    friend istream& operator>>(istream &in, record &ob);
};

// читает таблицу из файла в массив list, возвращает число записей.
// если файла ещё нет, возвращает 0, а если файл испорчен, бросает исключение
int load_records(string path, record *list);

// добавляет результат, сортирует таблицу и оставляет MAX_RECORDS лучших
void add_record(string path, record r);
