#include "actor.h"
Actor* actor_new(float x, float y, float z, float scale, str kind) {
    Game * game = game_global();
    Actor * a;
    land_alloc(a);
    a->camera = land_camera_new();
    land_camera_warp(a->camera, x, y, z);
    land_camera_scale(a->camera, scale);
    a->ob = add_object(0, 0, 0, 0, kind);
    a->target_z = z;
    a->speed = 3.2;
    actor_tick(a);
    land_array_add(game->actors, a);
    return a;
}
void actor_tick(Actor * a) {
    int t = game_ticks();
    if (t < a->trail_t) {
        land_camera_change_locked(a->camera, 0, a->trail_a);
        actor_move(a, 2);
    }
    else if (t < a->tumbling_t) {
        land_camera_change_freely(a->camera, 0, a->trail_a / 2, a->trail_a);
        actor_move(a, 1);
    }
    else if (a->target) {
        Object * o = a->target;
        LandVector pos = land_4x4_matrix_get_position(& o->matrix);
        float dx = pos.x - a->camera->p.x;
        float dy = pos.y - a->camera->p.y;
        float d = sqrt(dx * dx + dy * dy);
        float ato = atan2(dx, dy);
        float anow = land_camera_get_up_down(a->camera);
        float av = 0.01;
        float adiff = - (ato - anow);
        if (adiff > pi) {
            adiff -= pi * 2;
        }
        if (adiff < - pi) {
            adiff += pi * 2;
        }
        if (adiff < 0 && adiff > - pi) {
            av *= - 1;
        }
        if (fabs(adiff) < 3 * pi / 180) {
            av = 0;
        }
        float up = 0;
        float zdiff = a->camera->p.z - a->camera->y.z * 100 - a->target_z;
        if (zdiff > 10) {
            up = - 0.001;
        }
        if (zdiff < - 10) {
            up = + 0.001;
        }
        land_camera_change_locked(a->camera, up, av);
        if (d > 100) {
            actor_move(a, a->speed);
            if (t > a->cycle_t) {
                a->cycle_t = t + land_rand(300, 900);
                a->trail_t = t + land_rand(60, 120);
                a->trail_a = land_rnd(- 0.04, 0.04);
                a->target_z = land_rnd(50, 200);
            }
        }
        else {
            actor_move(a, 1);
            a->trail_t = t + 180;
            if (a->waypoints [a->waypoint]) {
                a->waypoint++;
                if (! a->waypoints [a->waypoint]) {
                    a->waypoint--;
                    a->home = 1;
                }
                a->target = a->waypoints [a->waypoint];
            }
        }
    }
    Land4x4Matrix tm = land_camera_forward_matrix(a->camera);
    float s = land_camera_get_scale(a->camera);
    tm = land_4x4_matrix_mul(tm, land_4x4_matrix_scale(s, s, s));
    a->ob->matrix = tm;
}
void actor_move(Actor * a, float speed) {
    LandVector backward = a->camera->y;
    backward = land_vector_mul(backward, - speed);
    land_camera_translate(a->camera, backward);
    if (a->camera->p.z < 0) {
        a->camera->p.z = 0;
    }
}
void actors_tick(void) {
    Game * game = game_global();
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(game->actors);
        for (Actor * a = LandArrayIterator_item(game->actors, &__iter0__); LandArrayIterator_next(game->actors, &__iter0__); a = LandArrayIterator_item(game->actors, &__iter0__)) {
            actor_tick(a);
        }
    }
}
void actor_set_target(Actor * a, Object * target) {
    a->target = target;
}
void actor_tumble(Actor * a) {
    a->tumbling_t = game_ticks() + 180;
    a->ghost_t = a->tumbling_t + 120;
    a->trail_a = (land_rand(0, 1) * 2 - 1) * 0.1;
}
void actor_place(Actor * a, float x, float y, float z, float yaw) {
    float zoom = a->camera->zoom;
    land_camera_init(a->camera);
    a->camera->zoom = zoom;
    a->target_z = z;
    a->camera->p = land_vector(x, y, z);
    land_camera_change_freely(a->camera, 0, 0, yaw);
}
