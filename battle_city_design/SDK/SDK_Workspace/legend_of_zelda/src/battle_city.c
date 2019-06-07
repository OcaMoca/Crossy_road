#include "battle_city.h"
#include "map.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>
#include "sprites.h"
#include <string.h>
#include <stdio.h>

/*          COLOR PALETTE - base addresses in ram.vhd         */
#define FRAME_COLORS_OFFSET         7
#define KIRBY_COLORS_OFFSET         0
#define ENEMY_COLORS_OFFSET         35

/*		SCREEN PARAMETERS		 - in this case, "screen" stands for one full-screen picture	 */
#define SCREEN_BASE_ADDRESS			6900	//	old: 6900	,	new: 6992
#define SCR_HEIGHT					30
#define SCR_WIDTH					40
#define SPRITE_SIZE					16

/*		FRAME HEADER		*/
#define HEADER_BASE_ADDRESS			7192	//	old: 7192	,	new: 7124
#define HEADER_HEIGHT				5

/*      FRAME       */
#define FRAME_BASE_ADDRESS			1000 // 	old: 7392	,	new: 7284		FRAME_OFFSET in battle_city.vhd
#define SIDE_PADDING				10
#define VERTICAL_PADDING			7
#define INITIAL_FRAME_X				7
#define INITIAL_FRAME_Y				7
#define INITIAL_KIRBY_POSITION_X    257
#define INITIAL_KIRBY_POSITION_Y    328

/*      LINK SPRITES START ADDRESS - to move to next add 64    */
#define LINK_SPRITES_OFFSET             255		//	old: 5172	,	new: 5648
#define SWORD_SPRITE                    LINK_SPRITES_OFFSET + 14*64			//6068		//	old: 7192	,	new: 7124
#define LINK_STEP						10

/*      ENEMIE SPRITES START ADDRESS - to move to next add 64    */
#define ENEMIE_SPRITES_OFFSET           5072			//	old: 4596	,	new: 5072
#define ENEMY_STEP						10
#define GHOST_SPRITES_OFFSET			5072 + 64*5

#define REGS_BASE_ADDRESS               ( SCREEN_BASE_ADDRESS + SCR_WIDTH * SCR_HEIGHT )

#define BTN_DOWN( b )                   ( !( b & 0x01 ) )
#define BTN_UP( b )                     ( !( b & 0x10 ) )
#define BTN_LEFT( b )                   ( !( b & 0x02 ) )
#define BTN_RIGHT( b )                  ( !( b & 0x08 ) )
#define BTN_SHOOT( b )                  ( !( b & 0x04 ) )


/*			these are the high and low registers that store moving sprites - two registers for each sprite		 */
#define LINK_REG_L                     8
#define LINK_REG_H                     9
#define WEAPON_REG_L                   4
#define WEAPON_REG_H                   5
#define ENEMY_2_REG_L                  6
#define ENEMY_2_REG_H                  7
#define ENEMY_3_REG_L                  2
#define ENEMY_3_REG_H                  3
#define ENEMY_4_REG_L                  10
#define ENEMY_4_REG_H                  11
#define ENEMY_5_REG_L                  12
#define ENEMY_5_REG_H                  13
#define ENEMY_6_REG_L                  14
#define ENEMY_6_REG_H                  15
#define ENEMY_7_REG_L                  0
#define ENEMY_7_REG_H                  1
#define GRANDPA_REG_L				   16
#define GRANDPA_REG_H	               17

#define MAX_HEALTH					   8

#define ENEMY_FRAMES_NUM 			   34
/*			contains the indexes of frames in overworld which have enemies  	*/
bool ENEMY_FRAMES[] = {32, 33, 45, 48, 49, 55, 56, 62, 64, 65, 68, 73, 76, 79,
					   84, 85, 86, 87, 88, 90, 95, 99, 100, 101, 102, 103, 104,
					   105, 106, 110, 111, 120, 125, 126};

unsigned short fire1 = FIRE_0;
unsigned short fire2 = FIRE_1;
int HEALTH;
int counter = 0;
int last = 0; //last state link was in before current iteration (if he is walking it keeps walking)
/*For testing purposes - values for last - Link sprites
	0 - down stand
	1 - down walk
	2 - up walk
	3 - right walk
	4 - right stand
	5 - down stand shield
	6 - down walk shield
	7 - right walk shield
	8 - right stand shield
	9 - down attack
	10 - up attack
	11 - right attack
	12 - item picked up
	13 - triforce picked up
	14 - sword
	15 - up flipped
	16 - left walk
	17 - left stand
	18 - left walk shield
	19 - left stand shield
	20 - left attack
*/

/*		 ACTIVE FRAME		*/
unsigned short* frame;

characters red_car_1 = {
		10*16,		// x
		13*16,		// y
		DIR_RIGHT, 	             		// dir
		0x01BF,							// type - sprite address in ram.vhdl
		true,                			// active
		ENEMY_2_REG_L,            		// reg_l
	    ENEMY_2_REG_H             		// reg_h
		};

characters red_car_2 = {
		9*16,		// x
		13*16,		// y
		DIR_RIGHT, 	             		// dir
		0x01FF,							// type - sprite address in ram.vhdl
		true,                			// active
		ENEMY_3_REG_L,            		// reg_l
	    ENEMY_3_REG_H             		// reg_h
		};

characters blue_car_1 = {
		0,		// x
		10*16,		// y
		DIR_LEFT, 	             		// dir
		0x023F,							// type - sprite address in ram.vhdl
		true,                			// active
		ENEMY_4_REG_L,            		// reg_l
	    ENEMY_4_REG_H             		// reg_h
		};

characters blue_car_2 = {
		0,		// x
		10*16,		// y
		DIR_LEFT, 	             		// dir
		0x027F,							// type - sprite address in ram.vhdl
		true,                			// active
		ENEMY_5_REG_L,            		// reg_l
	    ENEMY_5_REG_H             		// reg_h
		};




characters link = { 
		INITIAL_KIRBY_POSITION_X,		// x
		INITIAL_KIRBY_POSITION_Y,		// y
		DIR_LEFT, 	             		// dir
		0x00FF,  						// type - sprite address in ram.vhdl
		true,                			// active
		LINK_REG_L,            			// reg_l
		LINK_REG_H             			// reg_h
		};


characters blue_car_3 = {
		0,		// x
		18*16,		// y
		DIR_LEFT,              			// dir
		0x023F,  					// type
		true,                			// active
		WEAPON_REG_L,            		// reg_l
		WEAPON_REG_H             		// reg_h
		};

characters blue_car_4 = {
		0,								// x
		18*16,								// y
		DIR_LEFT,              			// dir
		0x027F,  					// type
		true,                			// active
		GRANDPA_REG_L,            		// reg_l
		GRANDPA_REG_H             		// reg_h
		};


characters sword = {
		INITIAL_KIRBY_POSITION_X,		// x
		INITIAL_KIRBY_POSITION_Y,		// y
		DIR_LEFT,              			// dir
		SWORD_SPRITE,  					// type
		false,                			// active
		WEAPON_REG_L,            		// reg_l
		WEAPON_REG_H             		// reg_h
		};

characters grandpa = {
		0,								// x
		0,								// y
		DIR_LEFT,              			// dir
		GRANDPA_SPRITE,  					// type
		true,                			// active
		GRANDPA_REG_L,            		// reg_l
		GRANDPA_REG_H             		// reg_h
		};

characters octorok1 = {
		0,								// x
		0,								// y
		DIR_LEFT,              			// dir
		ENEMIE_SPRITES_OFFSET,  					// type
		true,                			// active
		ENEMY_2_REG_L,            		// reg_l
		ENEMY_2_REG_H             		// reg_h
		};


characters octorok2 = {
		0,								// x
		0,								// y
		DIR_UP,              			// dir
		ENEMIE_SPRITES_OFFSET,  					// type
		true,                			// active
		ENEMY_3_REG_L,            		// reg_l
		ENEMY_3_REG_H             		// reg_h
		};

characters octorok3 = {
		0,								// x
		0,								// y
		DIR_DOWN,              			// dir
		ENEMIE_SPRITES_OFFSET,  					// type
		true,                			// active
		ENEMY_4_REG_L,            		// reg_l
		ENEMY_4_REG_H             		// reg_h
		};

characters octorok4 = {
		0,								// x
		0,								// y
		DIR_LEFT,              			// dir
		ENEMIE_SPRITES_OFFSET,  					// type
		true,                			// active
		ENEMY_5_REG_L,            		// reg_l
		ENEMY_5_REG_H             		// reg_h
		};

characters ghost = {
		0,								// x
		0,								// y
		DIR_LEFT,              			// dir
		GHOST_SPRITES_OFFSET + 64,  	// type 64*5 base
		true,                			// active
		ENEMY_6_REG_L,            		// reg_l
		ENEMY_6_REG_H             		// reg_h
		};



//int unwalkable_x[15] = {{3, 0}, {1, 4}, {10, 1}, {11, 1}, {0, 4}, {15, 4}, {7, 7}, {10, 7}, {2 , 13}, {3, 13}, {17, 13}, {2, 14}, {3, 14}, {9, 14}, {16, 14}};
int unwalkable_x[15] = {3*16, 1*16, 10*16, 11*16, 0*16, 15*16, 7*16, 10*16, 2*16, 3*16, 17*16, 2*16, 3*16, 9*16, 16*16};
int unwalkable_y[15] = {0*16, 4*16, 1*16, 1*16, 4*16, 4*16, 7*16, 7*16, 13*16, 13*16, 13*16, 14*16, 14*16, 14*16, 14*16};

/*      indexes of the active frame in overworld        */
int overw_x;
int overw_y;

int enemy_exists = 0;
int enemy_step = 0;

bool inCave = false;
/*      the position of the door so link could have the correct position when coming out of the cave    */
int door_x, door_y;
int rupees = 0, bombs = 0;

int random_number() {
	static short a[29] = {12,6,331,58,876,532,18,97,577,97,827,643,163,509,313,937,157, 853,752,16,85,11,54,977,61,45,46,314,88};
	static short p[17] = {7,179,5,13,59,29,37,31, 61,191 ,17,11, 113,19, 127,47, 71};
	static short i,j,rnd;

	i = (i+=5) < 231 ? i : 0;
	j = (j+=3) < 587 ? j : 0;

	rnd = a[i%29]*p[j%17];

	return rnd;
}

void load_frame( direction_t dir ) {

    /*      loading next frame into memory      */

		frame = overworld;
		int x,y;
		long int addr;
		for ( y = 0; y < FRAME_HEIGHT; y++ ) {
			for ( x = 0; x < FRAME_WIDTH; x++ ) {
				addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (y+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + x + SIDE_PADDING);
				Xil_Out32( addr, overworld[ y * FRAME_WIDTH + x ] );
			}
		}
}

void chhar_spawn( characters * chhar, int rotation ) {
	if ( rotation == 1 ) {																			 //rotate 90degrees clockwise
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F100000 | (unsigned int) chhar->sprite );
	} else if ( rotation == 2 ) { 																	//rotate 90degrees aniclockwise
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F010000 | (unsigned int) chhar->sprite );
	} else if ( rotation == 3 ) { 																	//rotate 180degrees
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F020000 | (unsigned int) chhar->sprite );
	}else {					    																	//no rotation
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F000000 | (unsigned int) chhar->sprite );
	}
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x );      //  the higher 2 bytes represent the row (y)
}

void delete_sword( characters* chhar ){
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
			(unsigned int) 0x80000000 | (unsigned int) chhar->sprite );
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x );      //  the higher 2 bytes represent the row (y)
}

/*  cleaning the registers used for moving characters sprites; two registers are used for each sprite   */
void reset_memory() {
	unsigned int i;
	long int addr;

	for( i = 0; i < SCR_WIDTH*SCR_HEIGHT; i++ ) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( SCREEN_BASE_ADDRESS + i );
		Xil_Out32( addr, SPRITES[3] );             // SPRITES[10] is a black square
	}
}

bool tree_detection(characters * kirby){

	int i,x,y;

	x = kirby->x;
	y = kirby->y;

	bool p = false;
	for(i = 0; i < 15; i++){
		if(x == unwalkable_x[i] && y == unwalkable_y[i]){
			y = y + 16;
			kirby->y = y;
			p = true;
			break;
		}
	}

	return p;


}

/* logika kretanja --> zamniti za link_move dole u battle_city kad bude gotovo */

bool kirby_move(characters * kirby, direction_t direction) {
	unsigned int x;
	unsigned int y;
	int i;



	x = kirby->x;
	y = kirby->y;

	int temp_new, temp_old;

	if(direction == DIR_UP){
		if(y < 7*16){
			kirby->y = y;
		}else{
				y--;
				kirby->y = y;
				//temp_old = y;
				//temp_new = --y;
				/*for(i = 0; i < 15; i++) {
					if(x + 2 == 10*16 + 4  && temp_new + 2 == 7*16 + 4){
						kirby->y = temp_old;
						break;
					} else {
						kirby->y = temp_new;
					}
				}*/
		}

	}

	if(direction == DIR_DOWN){
		if(y > 21*16){
			kirby->y = y;
		}else{
			y++;
			kirby->y = y;
		}
	}

	if(direction == DIR_RIGHT){
		if(x > 29*16){
			kirby->x = x;
		}else{
			x++;
			kirby->x = x;
		}
	}

	if(direction == DIR_LEFT){
		if(x < 10*16){
			kirby->x = x;
		}else{
			x--;
			kirby->x = x;
		}
	}


	chhar_spawn(kirby, 0);


	return false;
}

/*void car_move(unsigned int x1, unsigned int x2) {
	int i;

	blue_car_1.x = x2;
	blue_car_1.y = 13*16;

	blue_car_2.x = x2+16;
	blue_car_2.y = 13*16;

	red_car_1.x = x1;
	red_car_1.y = 10*16;

	red_car_2.x = x1+16;
	red_car_2.y = 10*16;

	chhar_spawn(&red_car_1, 0);
	chhar_spawn(&red_car_2, 0);

	chhar_spawn(&blue_car_1, 0);
	chhar_spawn(&blue_car_2, 0);


}*/

void car_move(characters* chhar1, characters* chhar2){
	int x1,x2;



	if (chhar1->dir == DIR_LEFT){
		x1 = chhar1->x;
		chhar2->x = chhar1->x + 16;
		x2 = chhar2->x;

		if(x1 < 10*16){
			x1 = 28*16;
			x2 = 29*16;
			chhar1->x = x1;
			chhar2->x = x2;
		} else {
			x1--;
			x2--;
			chhar1->x = x1;
			chhar2->x = x2;
		}
	}

	if (chhar1->dir == DIR_RIGHT){

		x2 = chhar2->x;
		chhar1->x = chhar2->x - 16;
		x1 = chhar1->x;

		if(x2 > 29*16){
			x1 = 10*16;
			x2 = 11*16;
			chhar1->x = x1;
			chhar2->x = x2;
		} else {
			x1++;
			x2++;
			chhar1->x = x1;
			chhar2->x = x2;
		}
	}


	chhar_spawn(chhar1, 0);
	chhar_spawn(chhar2, 0);
}

void collision(characters * kirby, characters* car1, characters* car2, characters* car3){
		/*if((kirby->x == car1->x && kirby->y == car1->y) || (kirby->x == car2->x && kirby->y == car2->y) || (kirby->x == car3->x && kirby->y == car3->y)){
			kirby->x = INITIAL_KIRBY_POSITION_X;
			kirby->y = INITIAL_KIRBY_POSITION_Y;
		}*/

	if(kirby->x+2 >= car1->x+4 && kirby->x+2 <= car1->x+12 && kirby->y+16 >= car1->y + 6 && kirby->y+16 <= car1->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}else if(kirby->x+14 >= car1->x+4 && kirby->x+14 <= car1->x+12 && kirby->y+16 >= car1->y + 6 && kirby->y+16 <= car1->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}

	if(kirby->x+2 >= car2->x+4 && kirby->x+2 <= car2->x+12 && kirby->y+16 >= car2->y + 6 && kirby->y+16 <= car2->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}else if(kirby->x+14 >= car2->x+4 && kirby->x+14 <= car2->x+12 && kirby->y+16 >= car2->y + 6 && kirby->y+16 <= car2->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}

	if(kirby->x+2 >= car3->x+4 && kirby->x+2 <= car3->x+12 && kirby->y+16 >= car3->y + 6 && kirby->y+16 <= car3->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}else if(kirby->x+14 >= car3->x+4 && kirby->x+14 <= car3->x+12 && kirby->y+16 >= car3->y + 6 && kirby->y+16 <= car3->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}
}



bool tile_walkable_new(characters* kirby, direction_t direction){
	int i;
	unsigned x, y;

		x = kirby->x;
		y = kirby->y;

		if(direction == DIR_UP){
			if(y < 7*16){
				kirby->y = y;
			}else{
				y--;
				for(i = 0; i < 15; i++) {
					if(x == unwalkable_x[i] && y == unwalkable_y[i]){
						y++;
					}
				kirby->y = y;
			}
		}

		if(direction == DIR_DOWN){
			if(y > 21*16){
				kirby->y = y;
			}else{
				y++;
				kirby->y = y;
			}
		}

		if(direction == DIR_RIGHT){
			if(x > 29*16){
				kirby->x = x;
			}else{
				x++;
				kirby->x = x;
			}
		}

		if(direction == DIR_LEFT){
			if(x < 10*16){
				kirby->x = x;
			}else{
				x--;
				kirby->x = x;
			}
		}


		chhar_spawn(kirby, 0);
	}
}

void battle_city() {
	unsigned int buttons;
    
    /*      initialization      */
	reset_memory();
	overw_x = INITIAL_FRAME_X;
	overw_y = INITIAL_FRAME_Y;
    load_frame( DIR_STILL );
    HEALTH = MAX_HEALTH;
    //set_header();

	link.x = INITIAL_KIRBY_POSITION_X;
	link.y = INITIAL_KIRBY_POSITION_Y;
	link.sprite = LINK_SPRITES_OFFSET;

	chhar_spawn(&link, 0);


	while (1) {

		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

		direction_t d = DIR_STILL;
		if ( BTN_LEFT(buttons) ) {
			d = DIR_LEFT;
		} else if ( BTN_RIGHT(buttons) ) {
			d = DIR_RIGHT;
		} else if ( BTN_UP(buttons) ) {
			d = DIR_UP;
		} else if ( BTN_DOWN(buttons) ) {
			d = DIR_DOWN;
		} else if ( BTN_SHOOT(buttons) ) {
			d = DIR_ATTACK;
		}

		/*if(tile_walkable_new() == true){
			kirby_move(&link, d);
		}*/

		kirby_move(&link, d);

		if (tree_detection(&link) == true){
			link.x = INITIAL_KIRBY_POSITION_X;
			link.y = INITIAL_KIRBY_POSITION_Y;
		}

		car_move(&blue_car_1,&blue_car_2);
		car_move(&red_car_1,&red_car_2);
		car_move(&blue_car_3,&blue_car_4);

		collision(&link, &blue_car_1, &red_car_1, &blue_car_3);




		int i;
		for(i = 0; i < 100000; i++);
	}







}
