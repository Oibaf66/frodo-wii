#include <arpa/inet.h>
#include <SDL.h>
#include <SDL_image.h>

#include "game_info.hh"
#include "utils.hh"

GameInfo::GameInfo()
{
	this->name = NULL;
	this->author = NULL;
	this->screenshot = NULL;
}

GameInfo::~GameInfo()
{
	this->resetDefaults();
}

void GameInfo::resetDefaults()
{
	free(this->name);
	free(this->author);
	SDL_FreeSurface(this->screenshot);

	this->name = NULL;
	this->author = NULL;
	this->screenshot = NULL;
}

struct game_info *GameInfo::dump(size_t *out_sz)
{
	size_t total_sz = sizeof(struct game_info);
	size_t png_sz;
	struct game_info *out;
	void *png_data;

	if (this->screenshot)
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
	out->dummy = 0;
	out->author_off = 0; /* Starts AFTER the header */
	out->name_off = out->author_off + strlen(this->author) + 1;
	out->screenshot_off = out->name_off + strlen(this->name) + 1;

	memcpy(out->data + out->author_off, this->author, strlen(this->author) + 1);
	memcpy(out->data + out->name_off, this->name, strlen(this->name) + 1);
	memcpy(out->data + out->screenshot_off, png_data, png_sz);

	out->sz = htonl(out->sz);
	out->author_off = htons(out->author_off);
	out->name_off = htons(out->name_off);
	out->screenshot_off = htons(out->screenshot_off);

	return out;
}

void GameInfo::fromDump(struct game_info *gi, size_t sz)
{
	SDL_RWops *rw;

	gi->sz = ntohl(gi->sz);
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

	return;

bail_out:
	this->resetDefaults();
}
