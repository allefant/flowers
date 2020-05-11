#ifndef _ACTOR_
#define _ACTOR_
typedef struct Actor Actor;
#include "common.h"
#include "game.h"
struct Actor {
    LandCamera * camera;
    float speed;
    Object * ob;
    int cycle_t;
    int trail_t;
    int tumbling_t;
    int ghost_t;
    float trail_a;
    float target_z;
    bool home;
    Object * target;
    Object * waypoints [10];
    int waypoint;
};
Actor* actor_new(float x, float y, float z, float scale, str kind);
void actor_tick(Actor * a);
void actor_move(Actor * a, float speed);
void actors_tick(void);
void actor_set_target(Actor * a, Object * target);
void actor_tumble(Actor * a);
void actor_place(Actor * a, float x, float y, float z, float yaw);
#endif
