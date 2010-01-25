#ifndef __FONT_HH__
#define __FONT_HH__

#include <SDL.h>

class Font
{
public:
	virtual ~Font()
	{
	}

	virtual int getHeight(const char *str) = 0;

	virtual int getHeight(const char c)
	{
		char buf[2] = {c, '\0'};

		return this->getHeight(buf);
	}

	virtual int getWidth(const char *str) = 0;

	virtual int getWidth(const char c)
	{
		char buf[2] = {c, '\0'};

		return this->getWidth(buf);
	}

	virtual void draw(SDL_Surface *where, const char *msg,
			int x, int y, int w, int h) = 0;
};

#endif /* __FONT_HH__ */
