#include <SDL_image.h>
#include <SDL.h>
#include <SDL_rwops.h>
#include <stdio.h>

#include "bitmap-font.h"

Font::Font(std::string src_file)
{
	Uint8 *data = (Uint8*)malloc(2 * 1024*1024);
	FILE *fp = fopen(src_file.c_str(), "r");
	SDL_RWops *rw;
	
	if (!fp) {
		fprintf(stderr, "Could not open fonts\n");
		SDL_Delay(1000);
		free(data);
		exit(1);
	}
	else
		fprintf(stderr, "I could open fonts manually\n");
	fread(data, 1, 2 * 1024 * 1024, fp);
	rw = SDL_RWFromMem(data, 2 * 1024 * 1024);
	if (!rw) {
		fprintf(stderr, "Could not create RW: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Surface *surf = IMG_Load_RW(rw, 0);

	if (!surf) {
		fprintf(stderr, "Could not load %s\n", src_file.c_str());
		SDL_Delay(1000);
		exit(1);
	}
	m_pFontList = surf;

	SDL_FreeRW(rw);
	free(data);
}

Font::~Font(void)
{
}

int Font::ShowText(std::string text, int type, int pos_x, int pos_y, SDL_Surface* pScreen)
{
	/* TODO: We need to calculate the fonts height into the pos_y thing. */
	// Also, id like to see this stuff gathered from an ini file.
	// That way we can alter fonts without the need for recompilcation

	if(!pScreen) return 1;

	SDL_Rect rect, src_rect; // src_rect is the location of the character we need to fetch. rect will be the destenation
	rect.x = pos_x;
	rect.y = pos_y;
	SDL_Rect tmp_rect;

	for(int i=0; i < text.size(); i++) {
		
		tmp_rect.y = 39*type; // set right y axe

		switch(text[i]) {
			case 0x20:
				rect.x += 10;
				break;
			case 0x21: // !
				tmp_rect.x = 4;
				tmp_rect.w = 6;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x2D: // -
				tmp_rect.x = 184;
				tmp_rect.w = 8;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x30: // 0
				tmp_rect.x = 226;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;			
			case 0x31: // 1
				tmp_rect.x = 244;
				tmp_rect.w = 9;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;	
			case 0x32: // 2
				tmp_rect.x = 256;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;	
			case 0x33: // 3
				tmp_rect.x = 272;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;	
			case 0x34: // 4
				tmp_rect.x = 286;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;	
			case 0x35: // 5
				tmp_rect.x = 302;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x36: // 6
				tmp_rect.x = 317;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x37: // 7
				tmp_rect.x = 332;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x38: // 8
				tmp_rect.x = 347;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x39: // 9
				tmp_rect.x = 362;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x3A: // :
				tmp_rect.x = 379;
				tmp_rect.w = 6;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x3B: // ;
				tmp_rect.x = 394;
				tmp_rect.w = 5;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x3C: // <
				tmp_rect.x = 407;
				tmp_rect.w = 8;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x3D: // =
				tmp_rect.x = 424;
				tmp_rect.w = 8;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x3E: // >
				tmp_rect.x = 440;
				tmp_rect.w = 8;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x3F: // ?
				tmp_rect.x = 454;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x40: // ?
				tmp_rect.x = 465;
				tmp_rect.w = 16;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x41: // A
				tmp_rect.x = 482;
				tmp_rect.w = 13;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x42: // B
				tmp_rect.x = 498;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x43: // C
				tmp_rect.x = 511;
				tmp_rect.w = 13;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x44: // D
				tmp_rect.x = 527;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x45: // E
				tmp_rect.x = 542;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x46: // F
				tmp_rect.x = 558;
				tmp_rect.w = 9;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x47: // G
				tmp_rect.x = 571;
				tmp_rect.w = 14;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x48: // H
				tmp_rect.x = 586;
				tmp_rect.w = 13;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x49: // I
				tmp_rect.x = 602;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x4A: // J
				tmp_rect.x = 616;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x4B: // K
				tmp_rect.x = 631;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x4C: // L
				tmp_rect.x = 647;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x4D: // M
				tmp_rect.x = 659;
				tmp_rect.w = 16;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x4E: // N
				tmp_rect.x = 406;
				tmp_rect.w = 14;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x4F: // O
				tmp_rect.x = 675;
				tmp_rect.w = 15;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type; // <-- Here we fix that mistake :)
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x50: // P
				tmp_rect.x = 692;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x51: // Q
				tmp_rect.x = 705;
				tmp_rect.w = 14;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x52: // R
				tmp_rect.x = 722;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x53: // S
				tmp_rect.x = 736;
				tmp_rect.w = 13;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x54: // T
				tmp_rect.x = 751;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x55: // U
				tmp_rect.x = 766;
				tmp_rect.w = 13;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x56: // V
				tmp_rect.x = 782;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x57: // W
				tmp_rect.x = 795;
				tmp_rect.w = 15;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x58: // X
				tmp_rect.x = 811;
				tmp_rect.w = 13;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x59: // Y
				tmp_rect.x = 827;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x5A: // Z
				tmp_rect.x = 841;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x5B: // [
				tmp_rect.x = 858;
				tmp_rect.w = 8;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x5C: // /
				tmp_rect.x = 873;
				tmp_rect.w = 9;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x5D: // ]
				tmp_rect.x = 888;
				tmp_rect.w = 9;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x5E: // ]
				tmp_rect.x = 903;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x5F: // _
				tmp_rect.x = 915;
				tmp_rect.w = 15;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x60: // `
				tmp_rect.x = 936;
				tmp_rect.w = 7;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x61: // a
				tmp_rect.x = 946;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x62: // b
				tmp_rect.x = 962;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x63: // c
				tmp_rect.x = 976;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x64: // d
				tmp_rect.x = 1;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x65: // e
				tmp_rect.x = 16;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x66: // f
				tmp_rect.x = 34;
				tmp_rect.w = 9;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x67: // g
				tmp_rect.x = 48;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x68: // h
				tmp_rect.x = 62;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x69: // i
				tmp_rect.x = 80;
				tmp_rect.w = 6;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x6A: // j
				tmp_rect.x = 91;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x6B: // k
				tmp_rect.x = 108;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x6C: // l
				tmp_rect.x = 123;
				tmp_rect.w = 6;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x6D: // m
				tmp_rect.x = 136;
				tmp_rect.w = 14;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x6E: // n
				tmp_rect.x = 152;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x6F: // o
				tmp_rect.x = 167;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x70: // p
				tmp_rect.x = 182;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x71: // q
				tmp_rect.x = 197;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x72: // r
				tmp_rect.x = 212;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x73: // s
				tmp_rect.x = 229;
				tmp_rect.w = 9;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x74: // t
				tmp_rect.x = 242;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x75: // u
				tmp_rect.x = 258;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x76: // v
				tmp_rect.x = 272;
				tmp_rect.w = 10;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x77: // w
				tmp_rect.x = 285;
				tmp_rect.w = 14;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x78: // x
				tmp_rect.x = 301;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x79: // y
				tmp_rect.x = 318;
				tmp_rect.w = 12;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;
			case 0x7A: // z
				tmp_rect.x = 332;
				tmp_rect.w = 11;
				tmp_rect.h = 19;
				tmp_rect.y = 19 + 39*type;
				SDL_BlitSurface( m_pFontList, &tmp_rect, pScreen, &rect);
				rect.x += tmp_rect.w;
				break;

		}

	}

	return 0;
}
