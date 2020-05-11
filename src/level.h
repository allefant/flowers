#ifndef _LEVEL_
#define _LEVEL_
typedef struct Level Level;
#include "game.h"
struct Level {
    LandVector waypoints [6];
    Actor * workers [5];
};
void create_world(void);
void level_1(void);
void level_7(bool reset);
void level_10(bool reset);
void level_2(bool reset);
void level_3(bool reset);
void level_4(bool reset);
void level_6(bool reset);
void level_9(bool reset);
void level_5(bool reset);
void level_8(bool reset);
void level_11(bool reset);
void check_level(void);
void reset_world(void);
#endif
