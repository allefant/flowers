#include "main.h"
#include "game.h"
void realmain(void) {
    land_init();
    int w, h;
    land_display_desktop_size(& w, & h);
    land_set_display_parameters((int)(w * 3 / 4), (int)(h * 3 / 4), LAND_WINDOWED | LAND_RESIZE | LAND_OPENGL | LAND_DEPTH | LAND_ANTIALIAS);
    land_callbacks(game_init, game_tick, game_draw, game_done);
    land_mainloop();
}
land_use_main(realmain);
