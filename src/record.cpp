// record.cpp - таблица рекордов
#include "record.h"
#include <fstream>
#include <iomanip>

bool record::operator<(record &ob) {
    if (score != ob.score)
        return score < ob.score;
    return seconds > ob.seconds;
}

// строка таблицы: очки в 6 позициях, время в 10 позициях с одним знаком после точки
ostream& operator<<(ostream &out, record &ob) {
    out << setw(6) << ob.score << setw(10) << fixed << setprecision(1) << ob.seconds;
    out.unsetf(ios::fixed);
    return out;
}

istream& operator>>(istream &in, record &ob) {
    in >> ob.score >> ob.seconds;
    return in;
}

int load_records(string path, record *list) {
    ifstream in(path.c_str());
    if (!in.is_open())
        return 0;   // файла ещё нет: никто не проходил игру

    string header;
    getline(in, header);   // первая строка - заголовок таблицы
    int n = 0;
    while (n < MAX_RECORDS && in >> list[n])
        n++;
    // поток остановился не в конце файла: значит, там не числа
    if (in.fail() && !in.eof())
        throw "Файл рекордов испорчен";
    in.close();
    return n;
}

void add_record(string path, record r) {
    record table[MAX_RECORDS + 1];
    int n = load_records(path, table);
    table[n] = r;
    n++;

    // сортировка пузырьком: лучшие результаты наверху
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (table[j] < table[j + 1]) {
                record t = table[j];
                table[j] = table[j + 1];
                table[j + 1] = t;
            }
        }
    }
    if (n > MAX_RECORDS)
        n = MAX_RECORDS;

    ofstream out(path.c_str());
    if (!out.is_open())
        throw "Не удалось сохранить рекорды";
    out << "  Очки     Время" << endl;
    for (int i = 0; i < n; i++)
        out << table[i] << endl;
    out.close();
}
