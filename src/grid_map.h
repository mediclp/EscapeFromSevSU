// grid_map.h - сетка уровня: стены, столкновения, видимость и поиск пути
#pragma once
#include <string>
#include "raylib.h"
using namespace std;

const float CELL = 1.0f;          // размер клетки в метрах: один символ карты - один метр
const float WALL_HEIGHT = 2.6f;   // высота стен в метрах
const int MAX_SIZE = 80;          // наибольшая ширина и высота карты в клетках

// клетка карты: столбец x и строка z
struct cell {
    int x, z;

    // одна и та же ли это клетка
    bool operator==(cell ob) { return x == ob.x && z == ob.z; }
};

// переводят клетку в координаты мира и обратно
Vector3 cell_center(cell c);
cell cell_at(Vector3 p);

// расстояние между точками по полу, без учёта высоты
float flat_distance(Vector3 a, Vector3 b);

// символ закрытой двери: 1, 2, 3 - лестницы корпусов А, Б, В; T - турникет
bool is_door_symbol(char s);

// хранит символы карты и отвечает на вопросы про стены и пути.
// про объекты уровня не знает, ими занимается level
class grid_map {
private:
    string rows[MAX_SIZE];
    int width, height;

public:
    grid_map() {
        width = 0;
        height = 0;
    }

    // добавляет строку карты; бросает исключение, если карта слишком большая
    void add_row(string row);

    // символ клетки; за границей карты считается стена
    char symbol_at(cell c);

    // превращает клетку в пол, например после открытия двери
    void clear_cell(cell c);

    // стена, лифт или закрытая дверь
    bool is_solid(cell c);

    // задевает ли круг радиуса radius стену
    bool collides(float x, float z, float radius);

    // нет ли стен на прямой между двумя точками
    bool line_of_sight(Vector3 from, Vector3 to);

    // следующая клетка кратчайшего пути; если пути нет, возвращает from
    cell next_step(cell from, cell to);

    // можно ли дойти из одной клетки в другую
    bool is_reachable(cell from, cell to);

    // случайная клетка пола на всей карте или рядом с center
    cell random_floor_cell();
    cell random_cell_near(cell center, int radius);

    int get_width() { return width; }
    int get_height() { return height; }
};
