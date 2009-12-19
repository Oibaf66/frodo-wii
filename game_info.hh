#ifndef __GAME_INFO_HH__
#define __GAME_INFO_HH__

#include <SDL.h>

struct game_info
{
	uint32_t sz;
	uint16_t version_magic;
	uint16_t author_off;
	uint16_t name_off;
	uint16_t screenshot_off; /* In PNG format */
	uint8_t data[]; /* 4-byte aligned */
};

class GameInfo
{
public:
	GameInfo(const char *name = NULL, const char *author = NULL, SDL_Surface *image = NULL);

	~GameInfo();

	void resetDefaults();

	/** Returns an allocated dump structure */
	struct game_info *dump();

	/** Fill in this game info object from a structure */
	void fromDump(struct game_info *data);

protected:
	const char *name;
	const char *author;
	SDL_Surface *screenshot;
};

#endif /*__GAME_INFO_HH__ */
