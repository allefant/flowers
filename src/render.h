#ifndef _RENDER_
#define _RENDER_
typedef struct Cell Cell;
typedef struct SpatialHash SpatialHash;
typedef struct Render Render;
#include "common.h"
#include "game.h"
#include "music.h"
struct Cell {
    LandArray * objects;
    int x, y;
    int tag;
};
struct SpatialHash {
    LandArray * cells /* [Cell] */;
    float cell_size;
    float min_pos;
    int size;
};
struct Render {
    LandCamera * cam;
    LandArray * cellstack;
    int tag;
    SpatialHash * spatial_hash;
    LandVector light, campos;
    Land4x4Matrix projection;
    char * vertex_shader;
    char * fragment_shader;
    int stats_triangles;
    bool more;
    float minz;
    float hd_distance;
    Music * music;
};
extern Render render;
SpatialHash* spatial_hash_new(void);
Cell* hash_get_cell(SpatialHash * h, float x, float y);
void hash_static_object(SpatialHash * h, Object * o);
void hash_static_object_remove(SpatialHash * h, Object * o);
Object* find_closest_object_in_cell(SpatialHash * h, float x, float y, float maxd, int flags);
Object* find_closest_object(SpatialHash * h, float x, float y, float maxd, int flags);
void find_cell_objects(SpatialHash * h, Cell * cell, float x, float y, float z, float maxd, LandArray * (* result));
LandArray* find_objects(SpatialHash * h, float x, float y, float z, float maxd);
void actors_clear_out(SpatialHash * h, float x, float y, float radius, int flags);
void draw_object(Object * ob, Land4x4Matrix tm);
bool draw_cell(SpatialHash * h, Cell * cell);
void draw_cells(SpatialHash * h);
void music_start(void);
void music_tick(void);
void draw_scroller(str text, int counter);
#endif
