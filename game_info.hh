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
	GameInfo(char *name = NULL, char *author = NULL, SDL_Surface *image = NULL);

	~GameInfo();

	void resetDefaults();

	/** Returns an allocated dump structure */
	struct game_info *dump(size_t *out_sz);

	/** Fill in this game info object from a structure */
	void fromDump(struct game_info *data, size_t sz);

protected:
	char *name;
	char *author;
	SDL_Surface *screenshot;
};

#endif /*__GAME_INFO_HH__ */
