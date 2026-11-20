// grid_map.cpp - сетка уровня
#include "grid_map.h"
#include <cmath>
#include "raymath.h"

const float SIGHT_STEP = 0.25f;   // шаг проверки линии видимости в метрах

Vector3 cell_center(cell c) {
    Vector3 p = {(c.x + 0.5f) * CELL, 0.0f, (c.z + 0.5f) * CELL};
    return p;
}

cell cell_at(Vector3 p) {
    cell c = {(int)floor(p.x / CELL), (int)floor(p.z / CELL)};
    return c;
}

float flat_distance(Vector3 a, Vector3 b) {
    Vector2 a2 = {a.x, a.z};
    Vector2 b2 = {b.x, b.z};
    return Vector2Distance(a2, b2);
}

void grid_map::add_row(string row) {
    if (height >= MAX_SIZE)
        throw "На карте больше 96 строк";
    if ((int)row.length() > MAX_SIZE)
        throw "Строка карты длиннее 96 символов";
    rows[height] = row;
    height++;
    if ((int)row.length() > width)
        width = row.length();
}

char grid_map::symbol_at(cell c) {
    // короткие строки дополнены стенами
    if (c.z < 0 || c.z >= height || c.x < 0 || c.x >= (int)rows[c.z].length())
        return '#';
    return rows[c.z][c.x];
}

void grid_map::clear_cell(cell c) {
    rows[c.z][c.x] = '.';
}

bool is_door_symbol(char s) {
    return s == '1' || s == '2' || s == '3' || s == 'T';
}

bool grid_map::is_solid(cell c) {
    char s = symbol_at(c);
    return s == '#' || s == 'w' || s == 'L' || is_door_symbol(s);
}

bool grid_map::collides(float x, float z, float radius) {
    // проверяем четыре угла квадрата вокруг героя
    float shift[2] = {-radius, radius};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            Vector3 corner = {x + shift[i], 0.0f, z + shift[j]};
            if (is_solid(cell_at(corner)))
                return true;
        }
    }
    return false;
}

bool grid_map::line_of_sight(Vector3 from, Vector3 to) {
    // идём по прямой маленькими шагами и смотрим, не попали ли в стену
    int steps = (int)(flat_distance(from, to) / SIGHT_STEP);
    for (int i = 1; i < steps; i++) {
        Vector3 point = Vector3Lerp(from, to, (float)i / steps);
        if (is_solid(cell_at(point)))
            return false;
    }
    return true;
}

// поиск в ширину от цели: каждая клетка запоминает соседа, через которого идти к цели
cell grid_map::next_step(cell from, cell to) {
    if (from == to || is_solid(to))
        return from;

    int towards_goal[MAX_SIZE * MAX_SIZE];   // номер соседа или -1, если клетку ещё не нашли
    for (int i = 0; i < width * height; i++)
        towards_goal[i] = -1;

    cell queue_cells[MAX_SIZE * MAX_SIZE];   // очередь: берём из head, кладём в tail
    int head = 0, tail = 0;
    queue_cells[tail] = to;
    tail++;
    towards_goal[to.z * width + to.x] = to.z * width + to.x;

    int dx[4] = {1, -1, 0, 0};
    int dz[4] = {0, 0, 1, -1};
    while (head < tail) {
        cell current = queue_cells[head];
        head++;
        if (current == from)
            break;
        for (int i = 0; i < 4; i++) {
            cell next_cell = {current.x + dx[i], current.z + dz[i]};
            if (is_solid(next_cell))
                continue;
            int n = next_cell.z * width + next_cell.x;
            if (towards_goal[n] != -1)
                continue;
            towards_goal[n] = current.z * width + current.x;
            queue_cells[tail] = next_cell;
            tail++;
        }
    }

    int step = towards_goal[from.z * width + from.x];
    if (step == -1)
        return from;
    cell result = {step % width, step / width};
    return result;
}

bool grid_map::is_reachable(cell from, cell to) {
    if (from == to)
        return true;
    cell step = next_step(from, to);
    return !(step == from);
}

cell grid_map::random_floor_cell() {
    cell center = {width / 2, height / 2};
    int radius = width > height ? width : height;
    return random_cell_near(center, radius);
}

cell grid_map::random_cell_near(cell center, int radius) {
    while (true) {
        cell c = {center.x + GetRandomValue(-radius, radius), center.z + GetRandomValue(-radius, radius)};
        if (!is_solid(c))
            return c;
    }
}
