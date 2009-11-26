#ifndef __FONT_HH__
#define __FONT_HH__

class Font
{
public:
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

	virtual void draw(int x, int y, int w, int h) = 0;

	virtual void draw(int x, int y, int w, int h, int r, int g, int b) = 0;
};

#endif /* __FONT_HH__ */
