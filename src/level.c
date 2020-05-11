#include "level.h"
static Level level;
static void _assign_waypoint(int i, int w);
static void _setup(void);
void create_world(void) {
    Game * game = game_global();
    str flowers [] = {"flower1", "flower2", "flower3"};
    str pollens [] = {"pollen1", "pollen2", "pollen3"};
    for (int v = 0; v < 300; v += 1) {
        for (int u = 0; u < 300; u += 1) {
            float x = u * 100 - 15000;
            float y = v * 100 - 15000;
            float z = 0;
            add_object(x + 50, y + 50, - 1, 50, "tinstile");
            int r = land_rand(0, 2);
            x += land_rnd(0, 100);
            y += land_rnd(0, 100);
            Object * ob = add_object(x, y, z, 30 + land_rnd(- 5, 5), flowers [r]);
            ob->matrix = land_4x4_matrix_mul(ob->matrix, land_4x4_matrix_rotate(0, 0, 1, land_rnd(- pi, pi)));
            add_object(x, y, z + ob->height + 45, 5, pollens [r]);
        }
    }
    game->bee = actor_new(0, 0, 50, 5, "bumblebee");
    game->queen = actor_new(0, 100, 150, 10, "bumblebeequeen");
    for (int i = 0; i < 5; i += 1) {
        Actor * worker = actor_new(- 200 + i * 100, 100, 70, 5, "bumblebee");
        actor_set_target(worker, game->queen->ob);
        level.workers [i] = worker;
    }
    // find waypoints
    for (int i = 0; i < 6; i += 1) {
        LandVector * p = level.waypoints + i;
        float r = 5000;
        float a = 2 * pi / 5 * i;
        float x = sin(a) * r;
        float y = cos(a) * r;
        if (i > 0) {
            a += land_rnd(- 2 * pi / 10, 2 * pi / 10);
            r += land_rnd(- 1000, 1000);
        }
        if (i == 5) {
            x = 0;
            y = 0;
        }
        p->x = x;
        p->y = y;
    }
    Object * base [6];
    for (int i = 0; i < 6; i += 1) {
        LandVector * p = level.waypoints + i;
        Object * o = find_closest_object(render.spatial_hash, p->x, p->y, 500, Flower);
        actors_clear_out(render.spatial_hash, p->x, p->y, 400, Flower + Pollen);
        base [i] = o;
    }
    // clear random splotches
    for (int i = 0; i < 100; i += 1) {
        float x = land_rnd(- 8000, 8000);
        float y = land_rnd(- 8000, 8000);
        float r = land_rnd(500, 1500);
        int color = land_rand(1, 6);
        int flags = 0;
        if (color & 1) {
            flags += Red;
        }
        if (color & 2) {
            flags += Yellow;
        }
        if (color & 4) {
            flags += Blue;
        }
        actors_clear_out(render.spatial_hash, x, y, r, flags);
    }
    // add it back bases since queen relies on some object
    for (int i = 0; i < 6; i += 1) {
        hash_static_object(render.spatial_hash, base [i]);
    }
    game->level = 1;
}
static void _assign_waypoint(int i, int w) {
    Game * game = game_global();
    LandVector * p = level.waypoints + w;
    Object * o = find_closest_object(render.spatial_hash, p->x, p->y, 500, Flower);
    game->queen->waypoints [i] = o;
    game->queen->waypoints [i + 1] = NULL;
}
static void _setup(void) {
    Game * game = game_global();
    game->queen->waypoint = 0;
    game->queen->home = 0;
    game->gameover = 0;
    game->want_pollen = 0;
    actor_set_target(game->queen, game->queen->waypoints [0]);
}
// 1. follow
// 2. reach
// 3. collect 6
// 4. reach
// 5. collect 6 blue
// 6. reach
// 7. follow
// 8. collect 6 yellow
// 9. reach
// 10. follow
void level_1(void) {
    Game * game = game_global();
    _assign_waypoint(0, 0);
    _assign_waypoint(1, 5);
    actor_place(game->queen, 0, 100, 150, pi);
    actor_place(game->bee, 0, 0, 50, 0);
    game->bee->ghost_t = 300;
    for (int i = 0; i < 5; i += 1) {
        Actor * worker = level.workers [i];
        actor_place(worker, - 200 + i * 100, 100, 70, 0);
    }
    message("Follow the Queen!   Drag with either mouse button to turn.");
    game->level = 1;
    _setup();
}
void level_7(bool reset) {
    Game * game = game_global();
    _assign_waypoint(0, 1);
    _assign_waypoint(1, 3);
    _assign_waypoint(2, 5);
    if (reset) {
        actor_place(game->queen, 0, 100, 150, pi);
        actor_place(game->bee, 0, 0, 50, 0);
        game->bee->ghost_t = 300;
    }
    message("Follow the Queen!   She is looking for a new nesting place and may take a while.");
    game->level = 7;
    _setup();
}
void level_10(bool reset) {
    Game * game = game_global();
    _assign_waypoint(0, 0);
    _assign_waypoint(1, 1);
    _assign_waypoint(2, 2);
    _assign_waypoint(3, 3);
    _assign_waypoint(4, 4);
    _assign_waypoint(5, 5);
    if (reset) {
        actor_place(game->queen, 0, 100, 150, pi);
        actor_place(game->bee, 0, 0, 50, 0);
        game->bee->ghost_t = 300;
    }
    message("Follow the Queen!   Last time I promise!");
    game->level = 10;
    _setup();
}
void level_2(bool reset) {
    Game * game = game_global();
    _assign_waypoint(0, 5);
    game->level = 2;
    if (reset) {
        actor_place(game->queen, level.waypoints [5].x, level.waypoints [5].y, 150, pi);
        actor_place(game->bee, level.waypoints [5].x, level.waypoints [5].y - 1000, 50, 0);
    }
    message("Reach the queen!");
    _setup();
}
void level_3(bool reset) {
    Game * game = game_global();
    _assign_waypoint(0, 5);
    game->pollen1 = game->pollen2 = game->pollen3 = 0;
    game->level = 3;
    message("Collect 6 pollen!");
    _setup();
}
void level_4(bool reset) {
    Game * game = game_global();
    level_2(reset);
    game->level = 4;
}
void level_6(bool reset) {
    Game * game = game_global();
    level_2(reset);
    game->level = 6;
}
void level_9(bool reset) {
    Game * game = game_global();
    level_2(reset);
    game->level = 9;
}
void level_5(bool reset) {
    Game * game = game_global();
    _assign_waypoint(0, 5);
    game->pollen1 = game->pollen2 = game->pollen3 = 0;
    game->level = 5;
    message("Collect 6 pollen from blue flowers!   Only blue will do - if you pick another you have to start over.");
    _setup();
    game->want_pollen = Blue;
}
void level_8(bool reset) {
    Game * game = game_global();
    _assign_waypoint(0, 5);
    game->pollen1 = game->pollen2 = game->pollen3 = 0;
    game->level = 8;
    message("Collect 6 pollen from yellow flowers!   No reds or blues please.");
    _setup();
    game->want_pollen = Yellow;
}
void level_11(bool reset) {
    _assign_waypoint(0, 5);
    _setup();
    message("Collect *ALL* the pollen and deliver to the queen!""   Just kidding, this is the end of the game, you have won!""   Thanks for playing!""   I had a lot of fun making this!""   Thanks Amarillion for hosting TINS 2020!""   And thanks to everyone else particpating, see you next year!");
}
void check_level(void) {
    Game * game = game_global();
    if (game->level == 1 || game->level == 2 || game->level == 7 || game->level == 10) {
        if (game->queenfar > 2000 && ! game->gameover) {
            game->gameover = 1;
            game->gameover_t = game_ticks();
            message("Level Failed.   You are too far from the queen.   Please press a mouse button to try again!");
        }
    }
    if (game->level == 2 || game->level == 4 || game->level == 6 || game->level == 9 || game->level == 11) {
        if (game->queenfar < 50) {
            if (game->level == 2) {
                level_3(0);
            }
            if (game->level == 4) {
                level_5(0);
            }
            if (game->level == 6) {
                level_7(0);
            }
            if (game->level == 9) {
                level_10(0);
            }
            if (game->level == 11) {
                game->pollen1 = game->pollen2 = game->pollen3 = 0;
                message("Thank you for the pollen!   Now go collect more!");
            }
        }
    }
    if (game->level == 1 || game->level == 7 || game->level == 10) {
        if (game->queen->home) {
            if (game->level == 1) {
                level_2(0);
            }
            if (game->level == 7) {
                level_8(0);
            }
            if (game->level == 10) {
                level_11(0);
            }
        }
    }
    if (game->level == 3) {
        if (game->pollen1 + game->pollen2 + game->pollen3 == 6) {
            level_4(0);
        }
    }
    if (game->level == 5) {
        if (game->pollen3 == 6) {
            level_6(0);
        }
    }
    if (game->level == 8) {
        if (game->pollen2 == 6) {
            level_9(0);
        }
    }
}
void reset_world(void) {
    Game * game = game_global();
    game->queen->cycle_t = game_ticks() + 600;
    game->queen->trail_t = 0;
    land_camera_init(render.cam);
    land_camera_translate(render.cam, land_vector(0, 0, 0));
    land_camera_change_freely(render.cam, - pi * 0.55, 0.001, 0);
    game->setspeed = 3;
    if (game->level == 1) {
        level_1();
    }
    if (game->level == 2) {
        level_2(1);
    }
    if (game->level == 3) {
        level_3(1);
    }
    if (game->level == 4) {
        level_4(1);
    }
    if (game->level == 5) {
        level_5(1);
    }
    if (game->level == 6) {
        level_6(1);
    }
    if (game->level == 7) {
        level_7(1);
    }
    if (game->level == 8) {
        level_8(1);
    }
    if (game->level == 9) {
        level_9(1);
    }
    if (game->level == 10) {
        level_10(1);
    }
    if (game->level == 11) {
        level_11(1);
    }
}
