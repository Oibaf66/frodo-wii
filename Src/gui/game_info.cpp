#if defined(GEKKO)
# include <network.h>
#else
 #include <arpa/inet.h>
#endif
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>

#include <utils.hh>

#include "game_info.hh"

#define VERSION_BASE   (0x1978)
#define VERSION(x)     (VERSION_BASE + (x))
/* Current magic */
#define VERSION_MAGIC  VERSION(1)

struct game_info_v0
{
	uint32_t sz;
	uint16_t version_magic;

	uint16_t author_off;
	uint16_t name_off;
	uint16_t screenshot_off; /* In PNG format */
	uint16_t filename_off;
	uint16_t score;
	uint8_t data[]; /* 4-byte aligned */
};

struct game_info_v1
{
	uint32_t sz;
	uint16_t version_magic;
	uint16_t flags;

	uint16_t author_off;
	uint16_t name_off;
	uint16_t screenshot_off; /* In PNG format */
	uint16_t filename_off;
	uint16_t score;
	uint16_t year;
	uint8_t data[]; /* 4-byte aligned */
};

struct game_info_v2
{
	uint32_t sz;
	uint16_t version_magic;
	uint16_t flags;

	uint16_t author_off;
	uint16_t name_off;
	uint16_t screenshot_off; /* In PNG format */
	uint16_t filename_off;
	uint16_t score;
	uint16_t year;
	uint16_t musician_off;
	uint16_t graphics_artist_off;
	uint8_t data[]; /* 4-byte aligned */
};

static void demarshal_v0(struct game_info_v0 *src)
{
	src->sz = ntohl(src->sz);
	src->version_magic = ntohs(src->version_magic);
	src->author_off = ntohs(src->author_off);
	src->name_off = ntohs(src->name_off);
	src->filename_off = ntohs(src->filename_off);
	src->screenshot_off = ntohs(src->screenshot_off);
	src->score = ntohs(src->score);
}


static void demarshal_v1(struct game_info_v1 *src)
{
	demarshal_v0((struct game_info_v0 *)src);
	src->flags = ntohs(src->flags);
	src->year = ntohs(src->year);
}

static void demarshal_v2(struct game_info_v2 *src)
{
	demarshal_v1((struct game_info_v1 *)src);
	src->musician_off = ntohs(src->musician_off);
	src->graphics_artist_off = ntohs(src->graphics_artist_off);
}


static struct game_info *from_v0(struct game_info_v0 *src)
{
	size_t d = sizeof(struct game_info_v1) - sizeof(struct game_info_v0);
	struct game_info *dst;

	printf("Converting v0->v1\n");
	demarshal_v0(src);
	dst = (struct game_info*)xmalloc(src->sz + d);

	dst->sz = src->sz + d;
	dst->version_magic = VERSION_MAGIC;
	dst->flags = 0;
	dst->year = 1982; /* Got to assume something, right :-) */
	dst->score = src->score;

	dst->author_off = src->author_off;
	dst->name_off = src->name_off;
	dst->screenshot_off = src->screenshot_off;
	dst->filename_off = src->filename_off;
	memcpy(dst->data, src->data, src->sz - sizeof(struct game_info_v0));

	return dst;
}

static struct game_info *from_v1(struct game_info_v1 *src)
{
	struct game_info *dst;

	demarshal_v1(src);
	dst = (struct game_info*)xmalloc(src->sz);
	memcpy(dst, src, src->sz);

	return dst;
}

static struct game_info *from_v2(struct game_info_v2 *src)
{
	struct game_info *dst;

	demarshal_v2(src);
	dst = (struct game_info*)xmalloc(src->sz);
	memcpy(dst, src, src->sz);

	return dst;
}


GameInfo::GameInfo(const char *filename,
		const char *name, const char *author,
		const char *musician, const char *graphics_artist,
		SDL_Surface *image)
{
	this->filename = xstrdup(filename);
	if (strcmp(name, " ") == 0)
		this->name = xstrdup(filename);
	else
		this->name = xstrdup(name);
	this->musician = xstrdup(musician);
	this->graphics_artist = xstrdup(graphics_artist);
	this->screenshot = image;
	this->score = 0;
	this->year = 1982;
}

GameInfo::GameInfo(GameInfo *gi)
{
	if (!gi)
	{
		this->resetDefaults();
		return;
	}

	this->name = xstrdup(gi->name);
	this->author = xstrdup(gi->author);
	this->filename = xstrdup(gi->filename);
	this->screenshot = NULL;
	this->year = gi->year;

	if (gi->screenshot)
		this->screenshot = SDL_DisplayFormatAlpha(gi->screenshot);
}

GameInfo::~GameInfo()
{
	free((void*)this->name);
	free((void*)this->author);
	free((void*)this->filename);
	free((void*)this->graphics_artist);
	free((void*)this->musician);

	SDL_FreeSurface(this->screenshot);
}

void GameInfo::resetDefaults()
{
	free((void*)this->name);
	free((void*)this->author);
	free((void*)this->filename);
	free((void*)this->graphics_artist);
	free((void*)this->musician);
	SDL_FreeSurface(this->screenshot);

	this->name = xstrdup(" ");
	this->author = xstrdup(" ");
	this->filename = xstrdup("unknown");
	this->musician = xstrdup(" ");
	this->graphics_artist = xstrdup(" ");
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
	total_sz += strlen(this->filename) + 1;

	total_sz += png_sz;
	/* 4-byte align */
	total_sz += 4 - (total_sz & 3);

	/* Allocate everything */
	out = (struct game_info*)xmalloc(total_sz);
	out->sz = total_sz;
	out->score = this->score;
	out->year = this->year;
	out->version_magic = VERSION_MAGIC;

	out->author_off = 0; /* Starts AFTER the header */
	out->name_off = out->author_off + strlen(this->author) + 1;
	out->filename_off = out->name_off + strlen(this->name) + 1;
	out->screenshot_off = out->filename_off + strlen(this->filename) + 1;


	memcpy(out->data + out->author_off, this->author, strlen(this->author) + 1);
	memcpy(out->data + out->name_off, this->name, strlen(this->name) + 1);
	memcpy(out->data + out->filename_off, this->filename, strlen(this->filename) + 1);
	memcpy(out->data + out->screenshot_off, png_data, png_sz);

	/* Marshall it all */
	out->sz = htonl(out->sz);
	out->author_off = htons(out->author_off);
	out->version_magic = htons(out->version_magic);
	out->name_off = htons(out->name_off);
	out->filename_off = htons(out->filename_off);
	out->screenshot_off = htons(out->screenshot_off);
	out->score = htons(out->score);
	out->year = htons(out->year);

	return out;
}

bool GameInfo::fromDump(struct game_info *gi)
{
	struct game_info *p = gi;

	/* Demarshal */
	switch (ntohs(p->version_magic))
	{
	case VERSION(0):
		p = from_v0((struct game_info_v0 *)p); break;
	case VERSION(1):
		p = from_v1((struct game_info_v1 *)p); break;
	case VERSION(2):
		p = from_v2((struct game_info_v2 *)p); break;
	default:
		/* Garbage, let's return */
		warning("game info garbage magic: %2x\n",
				ntohs(p->version_magic));
		return false;
		break;
	}

	this->author = xstrdup((char*)p->data + p->author_off);
	this->name = xstrdup((char*)p->data + p->name_off);
	this->filename = xstrdup((char*)p->data + p->filename_off);
	this->score = p->score;
	this->year = p->year;

	this->screenshot = sdl_surface_from_data(p->data + p->screenshot_off,
			p->sz - p->screenshot_off);
	if (!this->screenshot)
		goto bail_out;
	free(p);

	return true;

bail_out:
	free(p);
	this->resetDefaults();

	return false;
}

GameInfo *GameInfo::loadFromFile(const char *fileName)
{
	struct stat st;
	struct game_info *p;
	GameInfo *out = NULL;
	FILE *fp;

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
	if (strlen(author) == 0)
		author = " ";
	free((void*)this->author);
	this->author = xstrdup(author);
	if (strcmp(author, " ") != 0)
		this->score++;
}

void GameInfo::setYear(uint16_t year)
{
	this->year = year;
	this->score++;
}

void GameInfo::setName(const char *name)
{
	if (strlen(name) == 0)
		name = " ";

	free((void*)this->name);
	this->name = xstrdup(name);
	if (strcmp(name, " ") != 0)
		this->score++;
}

void GameInfo::setScreenshot(SDL_Surface *scr)
{
	SDL_FreeSurface(this->screenshot);
	this->screenshot = scr;
	if (scr != NULL)
		this->score++;
}
