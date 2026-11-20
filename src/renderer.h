// renderer.h - отрисовка 3D-сцены
#pragma once
#include "raylib.h"

class player;

// рисует сцену в маленьком разрешении 480x270 и растягивает её на окно (вид PS1)
class renderer {
private:
    RenderTexture2D target;   // маленькая картинка, в которую рисуется сцена
    Shader shader;            // свет фонарика, туман и красный свет тревоги
    Texture2D wall_texture, floor_texture, ceiling_texture, white_texture;
    Model wall_model, floor_model, ceiling_model, box_model;
    Camera3D camera;
    int view_pos_loc, light_dir_loc, flashlight_loc, alarm_loc;   // номера переменных шейдера

    // копировать нельзя: две копии выгрузили бы одни и те же текстуры
    renderer(renderer &ob);

    // загружают шейдер, текстуры и модели
    void load_shader();
    void load_models();

    // назначает модели шейдер и текстуру
    void bind_model(Model &m, Texture2D t);

public:
    // загружает шейдер, текстуры и модели; окно уже должно быть открыто
    renderer();

    // выгружает всё, что загрузил конструктор
    ~renderer();

    // начинает кадр: ставит камеру в глаза героя и передаёт шейдеру свет.
    // alarm - яркость красного света тревоги от 0 до 1
    void begin_scene(player &p, float alarm);

    // заканчивает рисование сцены
    void end_scene();

    // выводит сцену на экран с сохранением пропорций
    void present();

    // рисуют клетку уровня; center - центр клетки на уровне пола
    void draw_wall(Vector3 center);
    void draw_floor_and_ceiling(Vector3 center);

    // рисует цветной параллелепипед
    void draw_box(Vector3 center, Vector3 size, Color color);

    // близко ли точка к камере: дальние клетки всё равно скрыты туманом
    bool is_near_camera(Vector3 p);
};
