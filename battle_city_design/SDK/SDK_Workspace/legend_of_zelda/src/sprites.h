/*			16x16 IMAGES		 */
unsigned short SPRITES[51] = {0x00FF, 0x013F, 0x017F, 0x01BF, 0x01FF, 0x023F, 0x027F, 0x02BF,
							0x02FF, 0x033F, 0x03E8, 0x03BF, 0x03FF, 0x043F, 0x047F, 0x04BF,
							0x04FF, 0x053F, 0x057F, 0x05BF, 0x05FF, 0x063F, 0x067F, 0x06BF,
							0x06FF, 0x073F, 0x077F, 0x07BF, 0x07FF, 0x083F, 0x087F, 0x08BF,
							0x08FF, 0x093F, 0x097F, 0x09BF, 0x09FF, 0x0A3F, 0x0A7F, 0x0ABF,
							0x0AFF, 0x0B3F, 0x0B7F, 0x0BBF, 0x0BFF, 0x0C3F, 0x0C7F, 0x0CBF,
							0x0CFF, 0x0D3F, 0x0D7F};

#define HEART_FULL				 4671
#define HEART_HALF 				 HEART_FULL + 64
#define HEART_EMPTY				 HEART_HALF + 64

/*      pickups     */
#define RUPEE_SPRITE        64
#define BOMB_SPRITE         128

#define GRANDPA_SPRITE      4991
#define FIRE_0				5072 + 64			//5072 is the enemy_sprites_offset
#define FIRE_1				5072 + 4*64
#define MINIMAP_BLANK		4863

unsigned short CHAR_A = 0x0DBF;
unsigned short CHAR_D = 0x0DFF;
unsigned short CHAR_E = 0x0E3F;
unsigned short CHAR_F = 0x0E7F;
unsigned short CHAR_G = 0x0EBF;
unsigned short CHAR_H = 0x0EFF;
unsigned short CHAR_I = 0x0F3F;
unsigned short CHAR_K = 0x0F7F;
unsigned short CHAR_L = 0x0FBF;
unsigned short CHAR_N = 0x0FFF;
unsigned short CHAR_O = 0x103F;
unsigned short CHAR_R = 0x107F;
unsigned short CHAR_S = 0x10BF;
unsigned short CHAR_T = 0x10FF;
unsigned short CHAR_U = 0x113F;
unsigned short CHAR_COMA = 0x117F;
unsigned short CHAR_APOSTROPHE = 0x11BF;
unsigned short CHAR_DOT = 0x11FF;
