#include "game.h"
static Game * game;
static LandFont * smallfont;
static LandFont * bigfont;
Game* game_global(void) {
    return game;
}
Kind* add_kind(str name, str pattern, int flags) {
    Kind * k;
    land_alloc(k);
    k->flags = flags;
    if (name) {
        k->name = land_strdup(name);
    }
    k->kid = land_array_count(game->kinds) + 1;
    land_array_add(game->kinds, k);
    if (name) {
        land_hash_insert(game->kinds_by_name, name, k);
    }
    k->frames = land_array_new();
    bool star = land_contains(pattern, "*");
    int frame = 1;
    while (1) {
        char * path = land_strdup(pattern);
        if (star) {
            char sframe [10];
            sprintf(sframe, "%04d", frame++);
            land_replace(& path, 0, "*", sframe);
        }
        LandObjFile * of = land_objfile_new_from_filename(path);
        if (of->error) {
            land_free(path);
            break;
        }
        Land4x4Matrix matrix = land_4x4_matrix_rotate(1, 0, 0, pi / 2);
        land_obj_transform(of, & matrix, 1);
        LandArray * tris = land_obj_triangles(of);
        k->height = 0;
        {
            LandArrayIterator __iter0__ = LandArrayIterator_first(tris);
            for (LandTriangles * t = LandArrayIterator_item(tris, &__iter0__); LandArrayIterator_next(tris, &__iter0__); t = LandArrayIterator_item(tris, &__iter0__)) {
                land_triangles_shader(t, "flower", render.vertex_shader, render.fragment_shader);
                printf("%s vertices: %d\n", path, t->n);
                float z = land_triangles_get_max_z(t);
                if (z > k->height) {
                    k->height = z;
                }
            }
        }
        land_array_add(k->frames, tris);
        land_free(path);
        if (! star) {
            break;
        }
    }
    return k;
}
Kind* add_kind_hd2(str name, str graphic, int flags) {
    char * path = land_strdup("");
    land_append(& path, "obj/%s_*.obj.b", graphic);
    Kind * k = add_kind(name, path, flags);
    land_free(path);
    path = land_strdup("obj/hd/");
    land_append(& path, "%s_*.obj.b", graphic);
    Kind * khd = add_kind(NULL, path, flags);
    land_free(path);
    if (land_array_count(khd->frames) > 0) {
        k->hd = khd;
    }
    return k;
}
Kind* add_kind_hd(str name, int flags) {
    return add_kind_hd2(name, name, flags);
}
Object* add_object(float x, float y, float z, float scale, str kind) {
    Object * o;
    land_alloc(o);
    Kind * k = land_hash_get(game->kinds_by_name, kind);
    o->matrix = land_4x4_matrix_translate(x, y, z);
    o->matrix = land_4x4_matrix_mul(o->matrix, land_4x4_matrix_scale(scale, scale, scale));
    o->kind = k;
    if (land_equals(kind, "bumblebee") || land_equals(kind, "bumblebeequeen")) {
        land_array_add(game->dynamic, o);
    }
    else {
        hash_static_object(render.spatial_hash, o);
    }
    o->height = k->height * scale;
    return o;
}
void remove_object(Object * self) {
    hash_static_object_remove(render.spatial_hash, self);
}
int game_ticks(void) {
    return game->ticks;
}
void game_init(void) {
    land_alloc(game);
    game->start_t = land_get_ticks();
    game->kinds = land_array_new();
    land_array_add(game->kinds, NULL);
    game->dynamic = land_array_new();
    land_array_add(game->dynamic, NULL);
    game->kinds_by_name = land_hash_new();
    game->actors = land_array_new();
    land_find_data_prefix("data/");
    int h = land_display_width();
    smallfont = land_font_load("Muli-Regular.ttf", h / 100);
    bigfont = land_font_load("Muli-Regular.ttf", h / 20);
}
void game_load(void) {
    game->buzz = land_sound_load("buzz.ogg");
    game->buzz2 = land_sound_load("buzz.ogg");
    game->ding = land_sound_load("ding.ogg");
    land_sound_play(game->buzz, 1, 0, 1);
    render.vertex_shader = land_read_text("flower.vert");
    render.fragment_shader = land_read_text("flower.frag");
    add_kind_hd("bumblebee", 0);
    add_kind_hd("bumblebeequeen", 0);
    Kind * f1 = add_kind_hd("flower1", Flower + Red);
    f1->height -= 1.5;
    Kind * f2 = add_kind_hd("flower2", Flower + Yellow);
    f2->height -= 1.5;
    Kind * f3 = add_kind_hd("flower3", Flower + Blue);
    f3->height -= 1.5;
    Kind * t = add_kind_hd("tinstile", 0);
    t->no_dot = 1;
    add_kind_hd2("pollen1", "pollen", Pollen + Red);
    add_kind_hd2("pollen2", "pollen", Pollen + Yellow);
    add_kind_hd2("pollen3", "pollen", Pollen + Blue);
}
void game_create(void) {
    render.spatial_hash = spatial_hash_new();
    render.cellstack = land_array_new();
    render.hd_distance = 500;
    render.cam = land_camera_new();
    create_world();
    reset_world();
    land_sound_loop(game->buzz, 1, 0, 1);
}
void game_draw_loading_screen(void) {
    land_clear(.7, .8, .9, 1);
    float w = land_display_width();
    float h = land_display_height();
    land_text_pos(w / 2, h / 2 - land_line_height());
    land_color(1, 1, 1, 1);
    land_print_center("Loading");
    land_print_center("Please Wait");
}
void game_draw(void) {
    game->drawn = 1;
    if (! game->started) {
        game_draw_loading_screen();
        return ;
    }
    render.stats_triangles = 0;
    land_render_state(LAND_DEPTH_TEST, 1);
    land_clear_depth(1);
    land_clear(.7, .8, .9, 1);
    glEnable(GL_CULL_FACE);
    LandFloat scale = land_camera_get_scale(render.cam);
    LandFloat ratio = 1.0 * land_display_width() / land_display_height();
    LandFloat size = 5;
    LandFloat depth = size * scale;
    LandFloat zrange = 2000;
    LandFloat viewer_distance = 50 * scale;
    // make sure the viewer doesn't penetrate the ground plane
    render.minz = depth * sqrt(2);
    render.campos = land_vector_add(render.cam->p, land_vector_mul(render.cam->z, viewer_distance));
    if (render.campos.z < render.minz) {
        render.cam->p.z -= render.campos.z - render.minz;
    }
    Land4x4Matrix pm = land_4x4_matrix_perspective(- size, - size / ratio, depth, size, size / ratio, depth + zrange);
    pm = land_4x4_matrix_mul(pm, land_4x4_matrix_translate(0, 0, - viewer_distance));
    pm = land_4x4_matrix_mul(pm, land_camera_matrix(render.cam));
    land_projection(pm);
    render.projection = pm;
    LandCamera * cam = render.cam;
    render.light = land_vector_normalize(land_vector(0, 1, 0));
    render.campos = land_vector_add(cam->p, land_vector_mul(cam->z, viewer_distance));
    land_triangles_set_light_direction(render.light);
    render.more = 0;
    draw_cells(render.spatial_hash);
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(game->dynamic);
        for (Object * ob = LandArrayIterator_item(game->dynamic, &__iter0__); LandArrayIterator_next(game->dynamic, &__iter0__); ob = LandArrayIterator_item(game->dynamic, &__iter0__)) {
            if (! ob) {
                continue;
            }
            Land4x4Matrix tm = ob->matrix;
            LandVector l = land_vector_backmul3x3(render.light, & tm);
            land_triangles_set_light_direction(land_vector_normalize(l));
            draw_object(ob, tm);
        }
    }
    land_reset_projection();
    land_reset_transform();
    glDisable(GL_CULL_FACE);
    land_render_state(LAND_DEPTH_TEST, 0);
    land_display_set_default_shaders();
    land_color(1, 1, 1, 1);
    land_text_pos(0, 0);
    //land_print("pitch %.0f", land_camera_get_pitch(cam) * 180 / pi)
    //land_print("yaw %.0f", land_camera_get_yaw(cam) * 180 / pi)
    land_font_set(smallfont);
    land_print("Level %d", game->level);
    //land_print("triangles: %dk", render.stats_triangles // 1000)
    //land_print("queen distance: %.0f", game.queenfar)
    land_write("Pollen ");
    land_color(1, 0, 0.3, 1);
    land_write("%d ", game->pollen1);
    land_color(0.9, 0.8, 0, 1);
    land_write("%d ", game->pollen2);
    land_color(0.3, 0.3, 1, 1);
    land_write("%d ", game->pollen3);
    land_print("");
    land_text_pos(0, land_text_y_pos());
    land_color(1, 1, 1, 1);
    land_print("Queen is %.2f yards away", game->queenfar * 2 / 1000.0);
    land_font_set(bigfont);
    if (game->scroller_text) {
        draw_scroller(game->scroller_text, game_ticks() - game->scroller_start_t);
    }
}
void message(str text) {
    game->scroller_text = text;
    game->scroller_start_t = game_ticks();
}
void queen_distance(void) {
    LandVector d = land_vector_sub(game->queen->camera->p, game->bee->camera->p);
    game->queenfar = land_vector_norm(d);
    if (game->queenfar < 1000) {
        if (! game->buzzing2) {
            game->buzzing2 = 1;
            land_sound_loop(game->buzz2, 0, 0, 1);
        }
        float v = (2000 - game->queenfar * 2) / 1000.0;
        if (v > 1) {
            v = 1;
        }
        LandVector cross = land_vector_cross(game->bee->camera->y, d);
        float dot = cross.z / game->queenfar;
        land_sound_change(game->buzz2, v, dot, 0.5);
    }
    else if (game->buzzing2) {
        game->buzzing2 = 0;
        land_sound_stop(game->buzz2);
    }
}
void game_tick(void) {
    if (land_key_pressed(LandKeyEscape)) {
        land_quit();
    }
    if (! game->started) {
        if (! game->loaded) {
            if (game->drawn) {
                game_load();
                game->loaded = 1;
            }
        }
        else if (! game->created) {
            game_create();
            game->created = 1;
        }
        else {
            if (land_get_ticks() > game->start_t + 120) {
                game->started = 1;
            }
        }
        return ;
    }
    if (! game->music) {
        game->music = 1;
        //land_stream_music(land_stream_default(), "flightofthebumblebee.ogg")
        music_start();
    }
    music_tick();
    float kx = 0, ky = 0;
    if (land_key(LandKeyLeft)) {
        kx = - 1;
    }
    if (land_key(LandKeyRight)) {
        kx = 1;
    }
    if (land_key(LandKeyUp)) {
        ky = - 1;
    }
    if (land_key(LandKeyDown)) {
        ky = 1;
    }
    if (land_key_pressed('1')) {
        game->setspeed = 1;
    }
    if (land_key_pressed('2')) {
        game->setspeed = 2;
    }
    if (land_key_pressed('3')) {
        game->setspeed = 3;
    }
    if (land_key_pressed('4')) {
        game->setspeed = 4;
    }
    if (land_key_pressed('0')) {
        game->setspeed = 0;
    }
    if (land_key_pressed('p')) {
        game->paused = ! game->paused;
    }
    if (land_key_pressed('r')) {
        reset_world();
    }
    if (land_key_pressed('l')) {
        game->level++;
        reset_world();
    }
    if (land_key_pressed('m')) {
        music_toggle(render.music);
    }
    if (land_mouse_button(0) || land_mouse_button(1) || kx || ky) {
        float rotx = land_mouse_delta_y() * LandPi / 180;
        float roty = land_mouse_delta_x() * LandPi / 180;
        rotx += 0.02 * ky;
        roty += 0.02 * kx;
        land_camera_change_locked_constrained(render.cam, rotx, roty, - pi * 0.6, - pi * 0.3);
        if (! game->buzzing) {
            land_sound_change(game->buzz, 1.25, 0, 1.5);
            game->buzzing = 1;
        }
    }
    else {
        if (game->buzzing) {
            land_sound_change(game->buzz, 1, 0, 1.0);
            game->buzzing = 0;
        }
    }
    if (game->paused) {
        return ;
    }
    game->ticks++;
    if (game->gameover) {
        if (land_mouse_button_clicked(0) || land_mouse_button_clicked(1)) {
            reset_world();
        }
    }
    if (game_ticks() < game->bee->tumbling_t || game->gameover) {
        ;
    }
    else {
        game->bee->speed += 0.01;
        if (game->bee->speed < 0) {
            game->bee->speed = 0;
        }
        if (game->bee->speed > game->setspeed) {
            game->bee->speed = game->setspeed;
        }
        bool orient_to_camera = 1;
        if (orient_to_camera) {
            LandCamera * cam = render.cam;
            LandQuaternion qbee = land_quaternion_from_vectors(game->bee->camera->x, game->bee->camera->y, game->bee->camera->z);
            LandQuaternion qcam = land_quaternion_from_vectors(land_vector_mul(cam->x, - 1), cam->z, cam->y);
            qbee = land_quaternion_towards(qbee, qcam, 0.01);
            double norm = land_quaternion_normalize(& qbee);
            if (isnan(norm)) {
                game->bee->camera->x = land_vector_mul(cam->x, - 1);
                game->bee->camera->y = cam->z;
                game->bee->camera->z = cam->y;
            }
            else {
                land_quaternion_vectors(qbee, & game->bee->camera->x, & game->bee->camera->y, & game->bee->camera->z);
            }
        }
        if (game->bee->speed > 0) {
            LandCamera * bc = game->bee->camera;
            actor_move(game->bee, game->bee->speed);
            render.cam->p = bc->p;
            LandArray * ca = find_objects(render.spatial_hash, bc->p.x, bc->p.y, bc->p.z, 15);
            if (ca) {
                {
                    LandArrayIterator __iter0__ = LandArrayIterator_first(ca);
                    for (Object * o = LandArrayIterator_item(ca, &__iter0__); LandArrayIterator_next(ca, &__iter0__); o = LandArrayIterator_item(ca, &__iter0__)) {
                        //print("collides %s", o.kind.name)
                        if (o->kind->flags & Pollen) {
                            if (game->pollen1 + game->pollen2 + game->pollen3 < 6) {
                                remove_object(o);
                                if ((game->want_pollen & o->kind->flags) != game->want_pollen) {
                                    game->pollen1 = game->pollen2 = game->pollen3 = 0;
                                }
                                else {
                                    land_sound_play(game->ding, 1, 0, 1);
                                    if (o->kind->flags & Red) {
                                        game->pollen1++;
                                    }
                                    if (o->kind->flags & Yellow) {
                                        game->pollen2++;
                                    }
                                    if (o->kind->flags & Blue) {
                                        game->pollen3++;
                                    }
                                }
                            }
                        }
                        else {
                            if (game_ticks() >= game->bee->ghost_t) {
                                actor_tumble(game->bee);
                                game->pollen1 = game->pollen2 = game->pollen3 = 0;
                            }
                        }
                    }
                }
                land_array_destroy(ca);
            }
        }
    }
    queen_distance();
    check_level();
    //render.cam->zoom += land_mouse_delta_z() * 0.01
    actors_tick();
}
void game_done(void) {
    ;
}
