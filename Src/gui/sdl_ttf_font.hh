#include <SDL.h>
#include <SDL_ttf.h>

#include <utils.hh>
#include "font.hh"

#ifndef __SDL_TTF_FONT_HH__
#define __SDL_TTF_FONT_HH__

class Font_TTF : public Font
{
public:
	Font_TTF(TTF_Font *font,
			int r, int g, int b)
	{
		this->clr = (SDL_Color){r, g, b};
		this->font = font;
	}

	~Font_TTF()
	{
		TTF_CloseFont(this->font);
	}

	int getHeight(const char *str)
	{
		int tw, th;

		TTF_SizeText(this->font, str, &tw, &th);

		return th;
	}

	int getWidth(const char *str)
	{
		int tw, th;

		TTF_SizeText(this->font, str, &tw, &th);

		return tw;
	}

	void draw(SDL_Surface *where, const char *msg,
			int x, int y, int w, int h)
	{
		SDL_Surface *p;
		SDL_Rect dst;

		p = TTF_RenderText_Blended(this->font, msg, this->clr);
		panic_if(!p, "TTF error for '%s': %s\n", msg, TTF_GetError());

		dst = (SDL_Rect){x, y, w, h};

		SDL_BlitSurface(p, NULL, where, &dst);
		SDL_FreeSurface(p);
	}

protected:
	TTF_Font *font;
	SDL_Color clr;
};

#endif
