#include <arpa/inet.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>

#include "game_info.hh"
#include "utils.hh"

#define VERSION_BASE   (0x1978)
#define VERSION_MAGIC  (VERSION_BASE + 1)

GameInfo::GameInfo(const char *filename,
		const char *name, const char *author,
		SDL_Surface *image)
{
	this->filename = filename;
	this->name = name;
	this->author = author;
	this->screenshot = image;
}

GameInfo::GameInfo(GameInfo *gi)
{
	if (!gi)
	{
		this->filename = NULL;
		this->name = NULL;
		this->author = NULL;
		this->screenshot = NULL;
		return;
	}

	this->name = gi->name ? xstrdup(gi->name) : NULL;
	this->author = gi->author ? xstrdup(gi->author) : NULL;
	this->filename = gi->filename ? xstrdup(gi->filename) : NULL;
	this->screenshot = NULL;

	if (gi->screenshot)
		this->screenshot = SDL_DisplayFormatAlpha(gi->screenshot);
}

GameInfo::~GameInfo()
{
	this->resetDefaults();
}

void GameInfo::resetDefaults()
{
	free((void*)this->name);
	free((void*)this->author);
	SDL_FreeSurface(this->screenshot);

	this->name = NULL;
	this->author = NULL;
	this->screenshot = NULL;
}

struct game_info *GameInfo::dump()
{
	size_t total_sz = sizeof(struct game_info);
	size_t png_sz;
	struct game_info *out;
	void *png_data;

	if (!this->screenshot)
		return NULL;

	/* Convert surface to PNG */
	png_data = sdl_surface_to_png(this->screenshot, &png_sz);
	panic_if(!png_data, "Cannot create PNG from surface\n");

	total_sz += strlen(this->author) + 1;
	total_sz += strlen(this->name) + 1;

	total_sz += png_sz;
	/* 4-byte align */
	total_sz += 4 - (total_sz & 3);

	/* Allocate everything */
	out = (struct game_info*)xmalloc(total_sz);
	out->sz = total_sz;
	out->version_magic = VERSION_MAGIC;
	out->author_off = 0; /* Starts AFTER the header */
	out->name_off = out->author_off + strlen(this->author) + 1;
	out->screenshot_off = out->name_off + strlen(this->name) + 1;

	memcpy(out->data + out->author_off, this->author, strlen(this->author) + 1);
	memcpy(out->data + out->name_off, this->name, strlen(this->name) + 1);
	memcpy(out->data + out->screenshot_off, png_data, png_sz);

	out->sz = htonl(out->sz);
	out->author_off = htons(out->author_off);
	out->version_magic = htons(out->version_magic);
	out->name_off = htons(out->name_off);
	out->screenshot_off = htons(out->screenshot_off);

	return out;
}

bool GameInfo::fromDump(struct game_info *gi)
{
	SDL_RWops *rw;

	gi->sz = ntohl(gi->sz);
	gi->version_magic = ntohs(gi->version_magic);
	gi->author_off = ntohs(gi->author_off);
	gi->name_off = ntohs(gi->name_off);
	gi->screenshot_off = ntohs(gi->screenshot_off);

	this->author = xstrdup((char*)gi->data + gi->author_off);
	this->name = xstrdup((char*)gi->data + gi->name_off);

	rw = SDL_RWFromMem(gi->data + gi->screenshot_off,
			gi->sz - gi->screenshot_off);
	if (!rw)
		goto bail_out;

	this->screenshot = IMG_Load_RW(rw, 0);
	SDL_FreeRW(rw);
	if (!this->screenshot)
		goto bail_out;

	return true;

bail_out:
	this->resetDefaults();

	return false;
}

GameInfo *GameInfo::loadFromFile(const char *fileName)
{
	struct stat st;
	struct game_info *p;
	GameInfo *out = NULL;
	FILE *fp;

	printf("Woho! I'll load %s\n", fileName);
	if (stat(fileName, &st) < 0)
		return NULL;
	if (st.st_size <= (signed)sizeof(struct game_info))
	{
		warning("Size of %s is too small: %d vs minimum %d bytes\n",
				fileName, (int)st.st_size, (signed)sizeof(struct game_info) + 4);
		return NULL;
	}
	if (st.st_size % 4 != 0)
	{
		warning("Size of %s is unreasonable: %d\n", fileName, (int)st.st_size);
		return NULL;
	}

	fp = fopen(fileName, "r");
	if (!fp)
		return NULL;
	p = (struct game_info*)xmalloc(st.st_size);
	if (fread(p, 1, st.st_size, fp) == (unsigned)st.st_size)
	{
		out = new GameInfo();
		if (out->fromDump(p) == false)
		{
			warning("Converting %s to GameInfo failed\n",
					fileName);
			delete out;
			out = NULL;
		}
	}
	else
		warning("Could not read %s\n", fileName);

	free(p);
	fclose(fp);

	return out;
}

void GameInfo::setAuthor(const char *author)
{
	free((void*)this->author);
	this->author = xstrdup(author);
}

void GameInfo::setName(const char *name)
{
	free((void*)this->name);
	this->name = xstrdup(name);
}
