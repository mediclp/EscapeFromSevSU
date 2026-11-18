// renderer.cpp - отрисовка сцены
#include "renderer.h"
#include "grid_map.h"
#include "player.h"
#include "clamp.h"
#include "raymath.h"

const int SCENE_WIDTH = 320;
const int SCENE_HEIGHT = 240;
const float DRAW_DISTANCE = 16.0f;   // дальше всё скрыто туманом
const float SLAB = 0.1f;             // толщина пола и потолка
const int TEXTURE_SIZE = 64;

// текстура-заглушка: шум вокруг цвета base, а ниже строки stripe_from цвет stripe
Texture2D make_noise_texture(Color base, int spread, int stripe_from, Color stripe) {
    Image image = GenImageColor(TEXTURE_SIZE, TEXTURE_SIZE, base);
    for (int y = 0; y < TEXTURE_SIZE; y++) {
        for (int x = 0; x < TEXTURE_SIZE; x++) {
            Color c = y >= stripe_from ? stripe : base;
            int shift = GetRandomValue(-spread, spread);
            c.r = clamp_value(c.r + shift, 0, 255);
            c.g = clamp_value(c.g + shift, 0, 255);
            c.b = clamp_value(c.b + shift, 0, 255);
            ImageDrawPixel(&image, x, y, c);
        }
    }
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

renderer::renderer() {
    target = LoadRenderTexture(SCENE_WIDTH, SCENE_HEIGHT);
    camera.position = Vector3Zero();
    camera.target = Vector3Zero();
    camera.up.x = 0.0f;
    camera.up.y = 1.0f;
    camera.up.z = 0.0f;
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    load_shader();
    load_models();
}

renderer::~renderer() {
    UnloadModel(wall_model);
    UnloadModel(floor_model);
    UnloadModel(ceiling_model);
    UnloadModel(box_model);
    UnloadTexture(wall_texture);
    UnloadTexture(floor_texture);
    UnloadTexture(ceiling_texture);
    UnloadTexture(white_texture);
    UnloadShader(shader);
    UnloadRenderTexture(target);
}

void renderer::load_shader() {
    const char *dir = GetApplicationDirectory();
    shader = LoadShader(TextFormat("%sassets/shaders/lighting.vs", dir),
                        TextFormat("%sassets/shaders/lighting.fs", dir));
    view_pos_loc = GetShaderLocation(shader, "viewPos");
    light_dir_loc = GetShaderLocation(shader, "lightDir");
    flashlight_loc = GetShaderLocation(shader, "flashlight");
    alarm_loc = GetShaderLocation(shader, "alarm");
}

void renderer::bind_model(Model &m, Texture2D t) {
    m.materials[0].shader = shader;
    m.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = t;
}

void renderer::load_models() {
    Color wall_color = {150, 145, 125, 255};
    Color stripe_color = {70, 90, 80, 255};   // крашеная полоса на стене
    Color floor_color = {95, 80, 70, 255};
    Color ceiling_color = {120, 120, 115, 255};
    wall_texture = make_noise_texture(wall_color, 8, 42, stripe_color);
    floor_texture = make_noise_texture(floor_color, 12, TEXTURE_SIZE, BLANK);
    ceiling_texture = make_noise_texture(ceiling_color, 6, TEXTURE_SIZE, BLANK);
    white_texture = make_noise_texture(WHITE, 0, TEXTURE_SIZE, BLANK);

    wall_model = LoadModelFromMesh(GenMeshCube(CELL, WALL_HEIGHT, CELL));
    floor_model = LoadModelFromMesh(GenMeshCube(CELL, SLAB, CELL));
    ceiling_model = LoadModelFromMesh(GenMeshCube(CELL, SLAB, CELL));
    box_model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    bind_model(wall_model, wall_texture);
    bind_model(floor_model, floor_texture);
    bind_model(ceiling_model, ceiling_texture);
    bind_model(box_model, white_texture);
}

void renderer::begin_scene(player &p, float alarm) {
    camera.position = p.eye();
    Vector3 direction = p.forward();
    camera.target = Vector3Add(camera.position, direction);

    float light = p.light_on() ? 1.0f : 0.0f;
    SetShaderValue(shader, view_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, light_dir_loc, &direction, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, flashlight_loc, &light, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, alarm_loc, &alarm, SHADER_UNIFORM_FLOAT);

    BeginTextureMode(target);
    ClearBackground(BLACK);
    BeginMode3D(camera);
}

void renderer::end_scene() {
    EndMode3D();
    EndTextureMode();
}

void renderer::present() {
    float scale_x = (float)GetScreenWidth() / SCENE_WIDTH;
    float scale_y = (float)GetScreenHeight() / SCENE_HEIGHT;
    float scale = scale_x < scale_y ? scale_x : scale_y;
    float width = SCENE_WIDTH * scale;
    float height = SCENE_HEIGHT * scale;
    // высота отрицательная: картинка рендера хранится перевёрнутой
    Rectangle source = {0.0f, 0.0f, (float)SCENE_WIDTH, -(float)SCENE_HEIGHT};
    Rectangle dest = {(GetScreenWidth() - width) / 2.0f, (GetScreenHeight() - height) / 2.0f, width, height};
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(target.texture, source, dest, origin, 0.0f, WHITE);
}

void renderer::draw_wall(Vector3 center) {
    Vector3 p = {center.x, WALL_HEIGHT / 2.0f, center.z};
    DrawModel(wall_model, p, 1.0f, WHITE);
}

void renderer::draw_floor_and_ceiling(Vector3 center) {
    Vector3 bottom = {center.x, -SLAB / 2.0f, center.z};
    Vector3 top = {center.x, WALL_HEIGHT + SLAB / 2.0f, center.z};
    DrawModel(floor_model, bottom, 1.0f, WHITE);
    DrawModel(ceiling_model, top, 1.0f, WHITE);
}

void renderer::draw_box(Vector3 center, Vector3 size, Color color) {
    Vector3 axis = {0.0f, 1.0f, 0.0f};
    DrawModelEx(box_model, center, axis, 0.0f, size, color);
}

bool renderer::is_near_camera(Vector3 p) {
    return flat_distance(p, camera.position) < DRAW_DISTANCE;
}
