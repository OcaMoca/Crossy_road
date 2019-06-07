
#ifndef BATTLE_CITY_H_
#define BATTLE_CITY_H_

#define FRAME_WIDTH           20
#define FRAME_HEIGHT          15

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480

typedef int bool;
#define true 1
#define false 0

typedef enum {
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL, DIR_ATTACK
} direction_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	direction_t dir;
	unsigned short sprite;
	bool active;
	unsigned int reg_l;
	unsigned int reg_h;
} characters;

void load_frame( direction_t);
void chhar_spawn( characters *, int);
void reset_memory();
bool kirby_move(characters *, direction_t);
void car_move(characters*, characters*);
void crash(characters *, characters*, characters*, characters*);
void crossy_road();

#endif /* BATTLE_CITY_H_ */
