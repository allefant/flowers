#include "render.h"
Render render;
SpatialHash* spatial_hash_new(void) {
    SpatialHash * h;
    land_alloc(h);
    h->cell_size = 100;
    h->min_pos = - 15000;
    h->size = 300;
    h->cells = land_array_new();
    for (int y = 0; y < h->size; y += 1) {
        for (int x = 0; x < h->size; x += 1) {
            Cell * cell;
            land_alloc(cell);
            cell->x = x;
            cell->y = y;
            cell->objects = land_array_new();
            land_array_add(h->cells, cell);
        }
    }
    return h;
}
Cell* hash_get_cell(SpatialHash * h, float x, float y) {
    int cx = (int)((x - h->min_pos) / h->cell_size);
    int cy = (int)((y - h->min_pos) / h->cell_size);
    if (cx < 0) {
        cx = 0;
    }
    if (cx > h->size - 1) {
        cx = h->size - 1;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (cy > h->size - 1) {
        cy = h->size - 1;
    }
    return land_array_get(h->cells, cx + cy * h->size);
}
void hash_static_object(SpatialHash * h, Object * o) {
    LandVector v = land_4x4_matrix_get_position(& o->matrix);
    Cell * in_cell = hash_get_cell(h, v.x, v.y);
    land_array_add(in_cell->objects, o);
}
void hash_static_object_remove(SpatialHash * h, Object * o) {
    LandVector v = land_4x4_matrix_get_position(& o->matrix);
    Cell * in_cell = hash_get_cell(h, v.x, v.y);
    int n = land_array_count(in_cell->objects);
    for (int i = 0; i < n; i += 1) {
        Object * ob = land_array_get(in_cell->objects, i);
        if (ob == o) {
            land_array_remove(in_cell->objects, i);
            return ;
        }
    }
}
Object* find_closest_object_in_cell(SpatialHash * h, float x, float y, float maxd, int flags) {
    Cell * cell = hash_get_cell(h, x, y);
    Object * found = NULL;
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(cell->objects);
        for (Object * ob = LandArrayIterator_item(cell->objects, &__iter0__); LandArrayIterator_next(cell->objects, &__iter0__); ob = LandArrayIterator_item(cell->objects, &__iter0__)) {
            if ((ob->kind->flags & flags) == 0) {
                continue;
            }
            LandVector p = land_4x4_matrix_get_position(& ob->matrix);
            float dx = p.x - x;
            float dy = p.y - y;
            if (dx * dx + dy * dy < maxd * maxd) {
                found = ob;
            }
        }
    }
    return found;
}
Object* find_closest_object(SpatialHash * h, float x, float y, float maxd, int flags) {
    float c = h->cell_size;
    int n = c / maxd + 1;
    for (int i = 0; i < n; i += 1) {
        if (i == 0) {
            Object * found = find_closest_object_in_cell(h, x, y, maxd, flags);
            if (found) {
                return found;
            }
        }
        else if (i == 1) {
            float dx [] = {- 1, 0, 1, 0};
            float dy [] = {0, - 1, 0, 1};
            for (int j = 0; j < 4; j += 1) {
                Object * found = find_closest_object_in_cell(h, x + dx [i] * c, y + dy [i] * c, maxd, flags);
                if (found) {
                    return found;
                }
            }
        }
        else if (i == 2) {
            float dx [] = {- 2, - 1, 0, 1, 2, 1, 0, - 1};
            float dy [] = {0, - 1, - 2, - 1, 0, 1, 2, 1};
            for (int j = 0; j < 8; j += 1) {
                Object * found = find_closest_object_in_cell(h, x + dx [i] * c, y + dy [i] * c, maxd, flags);
                if (found) {
                    return found;
                }
            }
        }
    }
    return NULL;
}
void find_cell_objects(SpatialHash * h, Cell * cell, float x, float y, float z, float maxd, LandArray * (* result)) {
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(cell->objects);
        for (Object * ob = LandArrayIterator_item(cell->objects, &__iter0__); LandArrayIterator_next(cell->objects, &__iter0__); ob = LandArrayIterator_item(cell->objects, &__iter0__)) {
            LandVector p = land_4x4_matrix_get_position(& ob->matrix);
            float dx = p.x - x;
            float dy = p.y - y;
            if (p.z > z + maxd) {
                continue;
            }
            if (p.z + ob->height < z - maxd) {
                continue;
            }
            if (dx * dx + dy * dy < maxd * maxd) {
                if (! (* result)) {
                    * result = land_array_new();
                }
                land_array_add(* result, ob);
            }
        }
    }
}
LandArray* find_objects(SpatialHash * h, float x, float y, float z, float maxd) {
    LandArray * result = NULL;
    float cy = y - maxd;
    while (cy < y + maxd) {
        float cx = x - maxd;
        while (cx < x + maxd) {
            Cell * cell = hash_get_cell(h, cx, cy);
            find_cell_objects(h, cell, x, y, z, maxd, & result);
            cx += h->cell_size;
        }
        cy += h->cell_size;
    }
    return result;
}
void actors_clear_out(SpatialHash * h, float x, float y, float radius, int flags) {
    LandArray * a = find_objects(h, x, y, 0, radius);
    if (! a) {
        return ;
    }
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(a);
        for (Object * o = LandArrayIterator_item(a, &__iter0__); LandArrayIterator_next(a, &__iter0__); o = LandArrayIterator_item(a, &__iter0__)) {
            if ((o->kind->flags & flags) == 0) {
                continue;
            }
            //print("clearing %s", o.kind.name)
            remove_object(o);
        }
    }
    land_array_destroy(a);
}
void draw_object(Object * ob, Land4x4Matrix tm) {
    if (! ob) {
        return ;
    }
    land_display_transform_4x4(& tm);
    land_color(1, 1, 1, 1);
    Kind * k = ob->kind;
    LandVector pos = land_4x4_matrix_get_position(& tm);
    LandVector rel = land_vector_sub(pos, render.campos);
    LandFloat dot = land_vector_dot(rel, render.cam->z);
    if (dot > 0 && ! k->no_dot) {
        return ;
    }
    float dx = rel.x;
    float dy = rel.y;
    if (dx * dx + dy * dy < render.hd_distance * render.hd_distance) {
        if (k->hd) {
            k = k->hd;
        }
    }
    int n = land_array_count(k->frames);
    if (n == 0) {
        return ;
    }
    int t = 0;
    if (n == 5) {
        t = ((int)(land_get_ticks() / 4)) % 8;
        if (t >= 5) {
            t = t - 5 /* 0, 1, 2 */;
            t = 3 - t;
        }
    }
    LandArray * f = land_array_get(k->frames, t);
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(f);
        for (LandTriangles * tr = LandArrayIterator_item(f, &__iter0__); LandArrayIterator_next(f, &__iter0__); tr = LandArrayIterator_item(f, &__iter0__)) {
            land_triangles_draw_more(tr, render.more);
            render.stats_triangles += (int)(tr->n / 3);
            render.more = 1;
        }
    }
}
bool draw_cell(SpatialHash * h, Cell * cell) {
    if (cell->tag == render.tag) {
        return 0;
    }
    cell->tag = render.tag;
    LandFloat cs = h->cell_size;
    LandFloat vx = cell->x * cs + h->min_pos;
    LandFloat vy = cell->y * cs + h->min_pos;
    //land_color(1, 0.5, 0, 1)
    //Land4x4Matrix tm = land_4x4_matrix_identity()
    //tm = land_4x4_matrix_mul(tm, land_4x4_matrix_translate(vx, vy, 0))
    //tm = land_4x4_matrix_mul(tm, land_4x4_matrix_rotate(1, 0, 0, pi))
    //land_display_transform_4x4(&tm)
    //land_rectangle(0, 0, cs, cs)
    // TODO: The below check for just the corners is not sufficient,
    // we need to check if edges intersect -1/-1 to 1/1 instead. Until
    // we fix that lets just always draw but still return False -
    // effectively anything adjacent will be drawn as well that way.
    {
        LandArrayIterator __iter0__ = LandArrayIterator_first(cell->objects);
        for (Object * ob = LandArrayIterator_item(cell->objects, &__iter0__); LandArrayIterator_next(cell->objects, &__iter0__); ob = LandArrayIterator_item(cell->objects, &__iter0__)) {
            draw_object(ob, ob->matrix);
        }
    }
    LandVector p1 = land_vector_project(land_vector(vx, vy, 0), & render.projection);
    LandVector p2 = land_vector_project(land_vector(vx + cs, vy, 0), & render.projection);
    LandVector p3 = land_vector_project(land_vector(vx + cs, vy + cs, 0), & render.projection);
    LandVector p4 = land_vector_project(land_vector(vx, vy + cs, 0), & render.projection);
    bool out1 = p1.x < - 1 || p1.x > 1 || p1.z < - 1 || p1.z > 1;
    bool out2 = p2.x < - 1 || p2.x > 1 || p2.z < - 1 || p2.z > 1;
    bool out3 = p3.x < - 1 || p3.x > 1 || p3.z < - 1 || p3.z > 1;
    bool out4 = p4.x < - 1 || p4.x > 1 || p4.z < - 1 || p4.z > 1;
    if (out1 && out2 && out3 && out4) {
        return 0;
    }
    return 1;
}
void draw_cells(SpatialHash * h) {
    render.tag++;
    bool found = 0;
    land_array_clear(render.cellstack);
    LandCamera * cam = render.cam;
    LandVector p = cam->p;
    for (int t = 0; t < 10; t += 1) {
        Cell * f = hash_get_cell(h, p.x, p.y);
        land_array_add(render.cellstack, f);
        while (! land_array_is_empty(render.cellstack)) {
            Cell * cell = land_array_pop(render.cellstack);
            if (draw_cell(render.spatial_hash, cell)) {
                found = 1;
                if (cell->x > 0) {
                    land_array_add(render.cellstack, land_array_get(h->cells, cell->x - 1 + cell->y * h->size));
                }
                if (cell->y > 0) {
                    land_array_add(render.cellstack, land_array_get(h->cells, cell->x + (cell->y - 1) * h->size));
                }
                if (cell->x < h->size - 1) {
                    land_array_add(render.cellstack, land_array_get(h->cells, cell->x + 1 + cell->y * h->size));
                }
                if (cell->y < h->size - 1) {
                    land_array_add(render.cellstack, land_array_get(h->cells, cell->x + (cell->y + 1) * h->size));
                }
            }
        }
        if (found) {
            break;
        }
        // Since I don't have time to calculate the intersection of the
        // bottom ray with the grid plane - let's just search along
        // the view direction.
        p = land_vector_add(p, land_vector_mul(cam->z, - h->cell_size));
    }
}
void music_start(void) {
    MidiFile * midi = midifile_load("bumble_bee.mid");
    render.music = music_new(44100, 1024, 4, 16);
    render.music->midi = midi;
    music_unpause(render.music);
}
//music_simulate(music)
void music_tick(void) {
    if (render.music) {
        music_poll(render.music);
    }
}
void draw_scroller(str text, int counter) {
    float w = land_display_width();
    float s = w / 100;
    float advance = counter * w / 300 - w;
    int i = 0;
    int n = strlen(text);
    float x0 = s;
    float y0 = s;
    float off = - advance;
    int j = land_text_get_char_index(text, advance);
    if (j == n) {
        return ;
    }
    i += j;
    char letter [10] = " ";
    while (1) {
        letter [0] = text [i];
        float x1 = land_text_get_char_offset(text, i) + off;
        float x2 = land_text_get_char_offset(text, i + 1) + off;
        float t1 = x1 * 2 * pi / s / 40;
        float t2 = x2 * 2 * pi / s / 40;
        float y1 = sin(t1) * s;
        float y2 = sin(t2) * s;
        float rot = atan2(y2 - y1, x2 - x1);
        //land_thickness(5)
        //land_line(x0 + x1, y0 + y1, x0 + x2, y0 + y2)
        float wobble = sin(x1 * 2 * pi / s / 10);
        land_premul(1, 1, 1, 0.8 + wobble * 0.2);
        land_push_transform();
        land_translate(x0 + x1, y0 + y1);
        land_rotate(rot);
        land_translate(0, 0);
        land_text_pos(0, wobble * s / 2);
        land_print(letter);
        land_pop_transform();
        i++;
        if (i == n) {
            break;
        }
        if (x1 > s * 101) {
            break;
        }
    }
}
