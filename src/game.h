#ifndef _GAME_
#define _GAME_
typedef enum Flags Flags;
typedef struct Kind Kind;
typedef struct Object Object;
typedef struct Game Game;
#include "land/land.h"
#include "land/obj/obj.h"
#include "land/glsl.h"
#include "land/util2d.h"
#include "land/util3d.h"
#include "render.h"
#include "actor.h"
#include "level.h"
enum Flags {
    Flower=1,
    Red=2,
    Yellow=4,
    Blue=8,
    Pollen=16
};
struct Kind {
    char * name;
    int kid;
    LandArray * frames;
    Kind * hd;
    bool no_dot /* draw even if negative dot product with camera */;
    float height;
    int flags;
};
struct Object {
    Land4x4Matrix matrix;
    Kind * kind;
    float height;
};
struct Game {
    LandArray * kinds;
    LandHash * kinds_by_name;
    LandArray * dynamic;
    Actor * bee;
    Actor * queen;
    LandArray * actors;
    bool music;
    bool drawn;
    bool loaded;
    bool created;
    bool paused;
    bool started;
    bool gameover;
    int start_t;
    float setspeed;
    LandSound * buzz, * buzz2, * ding;
    bool buzzing, buzzing2;
    int ticks;
    int scroller_start_t;
    str scroller_text;
    float queenfar;
    int level;
    int gameover_t;
    int pollen1, pollen2, pollen3;
    int want_pollen;
};
Game* game_global(void);
Kind* add_kind(str name, str pattern, int flags);
Kind* add_kind_hd2(str name, str graphic, int flags);
Kind* add_kind_hd(str name, int flags);
Object* add_object(float x, float y, float z, float scale, str kind);
void remove_object(Object * self);
int game_ticks(void);
void game_init(void);
void game_load(void);
void game_create(void);
void game_draw_loading_screen(void);
void game_draw(void);
void message(str text);
void queen_distance(void);
void game_tick(void);
void game_done(void);
#endif
