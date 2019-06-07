#include "crossy_road.h"
#include "map.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>
#include "sprites.h"
#include <string.h>

/*          COLOR PALETTE - base addresses in ram.vhd         */
#define FRAME_COLORS_OFFSET         7
#define KIRBY_COLORS_OFFSET         0

/*		SCREEN PARAMETERS		 - in this case, "screen" stands for one full-screen picture	 */
#define SCREEN_BASE_ADDRESS			6900
#define SCR_HEIGHT					30
#define SCR_WIDTH					40
#define SPRITE_SIZE					16

/*      FRAME       */
#define FRAME_BASE_ADDRESS						1000 //		FRAME_OFFSET in battle_city.vhd
#define SIDE_PADDING							10
#define VERTICAL_PADDING						7
#define INITIAL_FRAME_X							7
#define INITIAL_FRAME_Y							7
#define INITIAL_KIRBY_POSITION_X    			257
#define INITIAL_KIRBY_POSITION_Y    			328
#define INITIAL_RED_CAR_FRONT_POSITION_X    	160
#define INITIAL_RED_CAR_FRONT_POSITION_Y    	208
#define INITIAL_RED_CAR_BACK_POSITION_X    		144
#define INITIAL_RED_CAR_BACK_POSITION_Y    		208
#define INITIAL_BLUE_CAR_1_FRONT_POSITION_X    	0
#define INITIAL_BLUE_CAR_1_FRONT_POSITION_Y    	160
#define INITIAL_BLUE_CAR_1_BACK_POSITION_X    	0
#define INITIAL_BLUE_CAR_1_BACK_POSITION_Y    	160

#define INITIAL_BLUE_CAR_2_FRONT_POSITION_X    	0
#define INITIAL_BLUE_CAR_2_FRONT_POSITION_Y    	288

#define INITIAL_BLUE_CAR_2_BACK_POSITION_X    	0
#define INITIAL_BLUE_CAR_2_BACK_POSITION_Y    	288

/*      SPRITES START ADDRESS - to move to next add 64    */
#define KIRBY_SPRITES_OFFSET             255
#define RED_CAR_FRONT_SPRITES_OFFSET     447
#define RED_CAR_BACK_SPRITES_OFFSET      511
#define BLUE_CAR_FRONT_1_SPRITES_OFFSET  575
#define BLUE_CAR_BACK_1_SPRITES_OFFSET   639
#define BLUE_CAR_FRONT_2_SPRITES_OFFSET  575
#define BLUE_CAR_BACK_2_SPRITES_OFFSET   639

#define REGS_BASE_ADDRESS               ( SCREEN_BASE_ADDRESS + SCR_WIDTH * SCR_HEIGHT )

#define BTN_DOWN( b )                   ( !( b & 0x01 ) )
#define BTN_UP( b )                     ( !( b & 0x10 ) )
#define BTN_LEFT( b )                   ( !( b & 0x02 ) )
#define BTN_RIGHT( b )                  ( !( b & 0x08 ) )
#define BTN_SHOOT( b )                  ( !( b & 0x04 ) )


/*			these are the high and low registers that store moving sprites - two registers for each sprite		 */
#define KIRBY_REG_L                     8
#define KIRBY_REG_H                     9
#define RED_CAR_FRONT_REG_L             6
#define RED_CAR_FRONT_REG_H             7
#define RED_CAR_BACK_REG_L              2
#define RED_CAR_BACK_REG_H              3
#define BLUE_CAR_1_FRONT_REG_L          10
#define BLUE_CAR_1_FRONT_REG_H          11
#define BLUE_CAR_1_BACK_REG_L           12
#define BLUE_CAR_1_BACK_REG_H           13
#define BLUE_CAR_2_FRONT_REG_L          4
#define BLUE_CAR_2_FRONT_REG_H          5
#define BLUE_CAR_2_BACK_REG_L		    16
#define BLUE_CAR_2_BACK_REG_H	        17

			/*      unused registers      */
/*
	#define UNUSED_1_REG_L                   14
	#define UNUSED_1_REG_H                   15
	#define UNUSED_2_REG_L                   0
	#define UNUSED_2_REG_H                   1
*/


/*		 ACTIVE FRAME		*/
unsigned short* frame;

characters kirby = {
		INITIAL_KIRBY_POSITION_X,				// x
		INITIAL_KIRBY_POSITION_Y,				// y
		DIR_LEFT, 	             				// dir
		KIRBY_SPRITES_OFFSET,  					// type - sprite address in ram.vhdl
		true,                					// active
		KIRBY_REG_L,            				// reg_l
		KIRBY_REG_H             				// reg_h
		};

characters red_car_front = {
		INITIAL_RED_CAR_FRONT_POSITION_X,		// x
		INITIAL_RED_CAR_FRONT_POSITION_Y,		// y
		DIR_RIGHT, 	             				// dir
		RED_CAR_FRONT_SPRITES_OFFSET,   		// type - sprite address in ram.vhdl
		true,                					// active
		RED_CAR_FRONT_REG_L,            		// reg_l
		RED_CAR_FRONT_REG_H             		// reg_h
		};

characters red_car_back = {
		INITIAL_RED_CAR_BACK_POSITION_X,		// x
		INITIAL_RED_CAR_BACK_POSITION_Y,		// y
		DIR_RIGHT, 	             				// dir
		RED_CAR_BACK_SPRITES_OFFSET,    		// type - sprite address in ram.vhdl
		true,                					// active
		RED_CAR_BACK_REG_L,            			// reg_l
		RED_CAR_BACK_REG_H             			// reg_h
		};

characters blue_car_front_1 = {
		INITIAL_BLUE_CAR_1_FRONT_POSITION_X,	// x
		INITIAL_BLUE_CAR_1_FRONT_POSITION_Y,	// y
		DIR_LEFT, 	             				// dir
		BLUE_CAR_FRONT_1_SPRITES_OFFSET,		// type - sprite address in ram.vhdl
		true,                					// active
		BLUE_CAR_1_FRONT_REG_L,            		// reg_l
		BLUE_CAR_1_FRONT_REG_H             		// reg_h
		};

characters blue_car_back_1 = {
		INITIAL_BLUE_CAR_1_BACK_POSITION_X,	    // x
		INITIAL_BLUE_CAR_1_BACK_POSITION_Y,		// y
		DIR_LEFT, 	             				// dir
		BLUE_CAR_BACK_1_SPRITES_OFFSET, 		// type - sprite address in ram.vhdl
		true,                					// active
		BLUE_CAR_1_BACK_REG_L,            		// reg_l
		BLUE_CAR_1_BACK_REG_H             		// reg_h
		};

characters blue_car_front_2 = {
		INITIAL_BLUE_CAR_2_FRONT_POSITION_X,	// x
		INITIAL_BLUE_CAR_2_FRONT_POSITION_Y,	// y
		DIR_LEFT,              					// dir
		BLUE_CAR_FRONT_2_SPRITES_OFFSET,		// type
		true,                					// active
		BLUE_CAR_2_FRONT_REG_L,            		// reg_l
		BLUE_CAR_2_FRONT_REG_H             		// reg_h
		};

characters blue_car_back_2 = {
		INITIAL_BLUE_CAR_2_BACK_POSITION_X,		// x
		INITIAL_BLUE_CAR_2_BACK_POSITION_Y,		// y
		DIR_LEFT,              					// dir
		BLUE_CAR_BACK_2_SPRITES_OFFSET, 		// type
		true,                					// active
		BLUE_CAR_2_BACK_REG_L,            		// reg_l
		BLUE_CAR_2_BACK_REG_H             		// reg_h
		};

/*      indexes of the active frame in overworld        */
int overw_x;
int overw_y;

	/*      loading next frame into memory      */
void load_frame( direction_t dir ) {
	int x,y;
	long int addr;
	frame = overworld;

	for ( y = 0; y < FRAME_HEIGHT; y++ ) {
		for ( x = 0; x < FRAME_WIDTH; x++ ) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (y+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + x + SIDE_PADDING);
			Xil_Out32( addr, frame[ y * FRAME_WIDTH + x ] );
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

	/*  cleaning the registers used for moving characters sprites; two registers are used for each sprite   */
void reset_memory() {
	unsigned int i;
	long int addr;

	for( i = 0; i < SCR_WIDTH*SCR_HEIGHT; i++ ) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( SCREEN_BASE_ADDRESS + i );
		Xil_Out32( addr, SPRITES[3] );             // SPRITES[10] is a black square
	}
}

bool kirby_move(characters * kirby, direction_t direction) {
	unsigned int x;
	unsigned int y;

	x = kirby->x;
	y = kirby->y;

	if(direction == DIR_UP) {
		if(y < 7*16){
			kirby->y = y;
		} else {
			y--;
			kirby->y = y;
		}
	}

	if(direction == DIR_DOWN) {
		if(y > 21*16) {
			kirby->y = y;
		} else {
			y++;
			kirby->y = y;
		}
	}

	if(direction == DIR_RIGHT) {
		if(x > 29*16) {
			kirby->x = x;
		} else {
			x++;
			kirby->x = x;
		}
	}

	if(direction == DIR_LEFT) {
		if(x < 10*16) {
			kirby->x = x;
		} else {
			x--;
			kirby->x = x;
		}
	}

	chhar_spawn(kirby, 0);

	return false;
}

void car_move(characters* front, characters* back){
	int x1, x2;

	if (front->dir == DIR_LEFT) {
		x1 = front->x;
		back->x = front->x + 16;
		x2 = back->x;

		if(x1 < 10*16) {
			x1 = 28*16;
			x2 = 29*16;
			front->x = x1;
			back->x = x2;
		} else {
			x1--;
			x2--;
			front->x = x1;
			back->x = x2;
		}
	}

	if (front->dir == DIR_RIGHT) {
		x2 = back->x;
		front->x = back->x - 16;
		x1 = front->x;

		if(x2 > 29*16) {
			x1 = 10*16;
			x2 = 11*16;
			front->x = x1;
			back->x = x2;
		} else {
			x1++;
			x2++;
			front->x = x1;
			back->x = x2;
		}
	}

	chhar_spawn(front, 0);
	chhar_spawn(back, 0);
}

	/*      collision detection      */

void crash(characters * kirby, characters* car1, characters* car2, characters* car3){

	if(kirby->x + 2 >= car1->x + 4 && kirby->x + 2 <= car1->x + 12 && kirby->y + 16 >= car1->y + 6 && kirby->y + 16 <= car1->y + 16) {
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;

	} else if(kirby->x + 14 >= car1->x + 4 && kirby->x + 14 <= car1->x + 12 && kirby->y + 16 >= car1->y + 6 && kirby->y + 16 <= car1->y + 16) {
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}


	if(kirby->x + 2 >= car2->x + 4 && kirby->x + 2 <= car2->x + 12 && kirby->y + 16 >= car2->y + 6 && kirby->y + 16 <= car2->y + 16) {
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;

	} else if(kirby->x+14 >= car2->x+4 && kirby->x+14 <= car2->x+12 && kirby->y+16 >= car2->y + 6 && kirby->y+16 <= car2->y + 16) {
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}


	if(kirby->x + 2 >= car3->x + 4 && kirby->x + 2 <= car3->x + 12 && kirby->y + 16 >= car3->y + 6 && kirby->y + 16 <= car3->y + 16) {
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;

	} else if(kirby->x + 14 >= car3->x + 4 && kirby->x + 14 <= car3->x + 12 && kirby->y + 16 >= car3->y + 6 && kirby->y + 16 <= car3->y + 16){
		kirby->x = INITIAL_KIRBY_POSITION_X;
		kirby->y = INITIAL_KIRBY_POSITION_Y;
	}
}

	/*      initialization      */

void crossy_road() {
	unsigned int buttons;

	reset_memory();
	overw_x = INITIAL_FRAME_X;
	overw_y = INITIAL_FRAME_Y;
    load_frame( DIR_STILL );

    kirby.x = INITIAL_KIRBY_POSITION_X;
    kirby.y = INITIAL_KIRBY_POSITION_Y;
    kirby.sprite = KIRBY_SPRITES_OFFSET;

	chhar_spawn(&kirby, 0);

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

		kirby_move(&kirby, d);

		car_move(&blue_car_front_1,&blue_car_back_1);
		car_move(&red_car_front,&red_car_back);
		car_move(&blue_car_front_2,&blue_car_back_2);

		crash(&kirby, &blue_car_front_1, &red_car_front, &blue_car_front_2);

		int i;
		for(i = 0; i < 100000; i++);
	}
}
