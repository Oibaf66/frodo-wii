#ifndef __FONT_H
#define __FONT_H

#include <iostream>
#include <string>
#include <SDL.h>


class Font
{
	SDL_Surface* m_pFontList;
public:
	Font(std::string src_file);
	virtual ~Font(void);
	int ShowText(std::string text, int type, int pos_x, int pos_y, SDL_Surface* pScreen);

        int GetHeight()
          {
            return 18;
          }

        int GetWidth()
          {
            return 14;
          }
};

#endif
