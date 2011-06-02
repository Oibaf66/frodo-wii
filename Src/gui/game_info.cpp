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
#define VERSION_MAGIC  VERSION(2)

struct game_info
{
	/* These two MUST stay the same */
	uint32_t sz;
	uint16_t version_magic;
	uint16_t flags;

	uint16_t author_off;
	uint16_t name_off;
	uint16_t screenshot_off; /* In PNG format */
	uint16_t filename_off;
	uint16_t score;
	uint16_t year;
	uint8_t genre;
	uint8_t players;
	uint16_t creator_off;
	uint16_t musician_off;
	uint16_t graphics_artist_off;
	uint8_t data[]; /* 4-byte aligned */
};

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
	uint8_t genre;
	uint8_t players;
	uint16_t creator_off;
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
	src->creator_off = ntohs(src->creator_off);
	src->graphics_artist_off = ntohs(src->graphics_artist_off);
}


static bool from_v0(GameInfo *dst, struct game_info_v0 *p)
{
	demarshal_v0(p);

	dst->publisher = xstrdup((char*)p->data + p->author_off);
	dst->name = xstrdup((char*)p->data + p->name_off);
	dst->filename = xstrdup((char*)p->data + p->filename_off);
	dst->score = p->score;
	dst->year = 1984;
	dst->graphics_artist = xstrdup(" ");
	dst->musician = xstrdup(" ");
	dst->creator = xstrdup(" ");
	dst->genre = GENRE_UNKNOWN;
	dst->players = 1;

	dst->screenshot = sdl_surface_from_data(p->data + p->screenshot_off,
			p->sz - p->screenshot_off);
	if (!dst->screenshot)
		return false;

	return true;
}

static bool from_v1(GameInfo *dst, struct game_info_v1 *p)
{
	demarshal_v1(p);

	dst->publisher = xstrdup((char*)p->data + p->author_off);
	dst->name = xstrdup((char*)p->data + p->name_off);
	dst->filename = xstrdup((char*)p->data + p->filename_off);
	dst->score = p->score;
	dst->year = p->year;
	dst->graphics_artist = xstrdup(" ");
	dst->musician = xstrdup(" ");
	dst->creator = xstrdup(" ");
	dst->genre = GENRE_UNKNOWN;
	dst->players = 1;

	dst->screenshot = sdl_surface_from_data(p->data + p->screenshot_off,
			p->sz - p->screenshot_off);
	if (!dst->screenshot)
		return false;

	return true;
}

static bool from_v2(GameInfo *dst, struct game_info_v2 *p)
{
	demarshal_v2(p);

	dst->publisher = xstrdup((char*)p->data + p->author_off);
	dst->name = xstrdup((char*)p->data + p->name_off);
	dst->filename = xstrdup((char*)p->data + p->filename_off);
	dst->score = p->score;
	dst->year = p->year;
	dst->graphics_artist = xstrdup((char*)p->data + p->graphics_artist_off);
	dst->musician = xstrdup((char*)p->data + p->musician_off);
	dst->creator = xstrdup((char*)p->data + p->creator_off);
	dst->genre = p->genre;
	dst->players = p->players;

	dst->screenshot = sdl_surface_from_data(p->data + p->screenshot_off,
			p->sz - p->screenshot_off);
	if (!dst->screenshot)
		return false;

	return true;
}


GameInfo::GameInfo(const char *filename,
		const char *name, const char *publisher,
		const char *creator, const char *graphics_artist,
		const char *musician, SDL_Surface *image)
{
	this->filename = xstrdup(filename);
	if (strcmp(name, " ") == 0)
		this->name = xstrdup(filename);
	else
		this->name = xstrdup(name);
	this->publisher = xstrdup(publisher);
	this->creator = xstrdup(creator);
	this->graphics_artist = xstrdup(graphics_artist);
	this->musician = xstrdup(musician);
	this->graphics_artist = xstrdup(graphics_artist);
	this->creator = xstrdup(creator);
	this->musician = xstrdup(musician);
	this->screenshot = image;
	this->genre = GENRE_UNKNOWN;
	this->players = 1;
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
	this->publisher = xstrdup(gi->publisher);
	this->filename = xstrdup(gi->filename);
	this->graphics_artist = xstrdup(gi->graphics_artist);
	this->creator = xstrdup(gi->creator);
	this->musician = xstrdup(gi->musician);
	this->screenshot = NULL;
	this->players = gi->players;
	this->score = gi->score;
	this->year = gi->year;
	this->genre = gi->genre;

	if (gi->screenshot)
	//	this->screenshot = SDL_DisplayFormat(gi->screenshot);
	this->screenshot = sdl_surface_8bit_copy(gi->screenshot);
}

GameInfo::~GameInfo()
{
	this->freeAll();
}

void GameInfo::freeAll()
{
	free((void*)this->name);
	free((void*)this->publisher);
	free((void*)this->filename);
	free((void*)this->creator);
	free((void*)this->graphics_artist);
	free((void*)this->musician);

	SDL_FreeSurface(this->screenshot);
}

void GameInfo::resetDefaults()
{
	this->freeAll();

	this->name = xstrdup(" ");
	this->publisher = xstrdup(" ");
	this->filename = xstrdup("unknown");
	this->creator = xstrdup(" ");
	this->musician = xstrdup(" ");
	this->graphics_artist = xstrdup(" ");
	this->screenshot = NULL;

	this->genre = GENRE_UNKNOWN;
	this->players = 1;
	this->score = 0;
	this->year = 1982;
}

void *GameInfo::dump(size_t *out_sz)
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

	total_sz += strlen(this->publisher) + 1;
	total_sz += strlen(this->name) + 1;
	total_sz += strlen(this->filename) + 1;
	total_sz += strlen(this->creator) + 1;
	total_sz += strlen(this->musician) + 1;
	total_sz += strlen(this->graphics_artist) + 1;

	total_sz += png_sz;
	/* 4-byte align */
	total_sz += 4 - (total_sz & 3);

	/* Allocate everything */
	out = (struct game_info*)xmalloc(total_sz);
	out->sz = total_sz;
	out->score = this->score;
	out->year = this->year;
	out->genre = this->genre;
	out->players = this->players;
	out->version_magic = VERSION_MAGIC;

	out->author_off = 0; /* Starts AFTER the header */
	out->name_off = out->author_off + strlen(this->publisher) + 1;
	out->filename_off = out->name_off + strlen(this->name) + 1;
	out->creator_off = out->filename_off + strlen(this->filename) + 1;
	out->musician_off = out->creator_off + strlen(this->creator) + 1;
	out->graphics_artist_off = out->musician_off + strlen(this->musician) + 1;
	out->screenshot_off = out->graphics_artist_off + strlen(this->graphics_artist) + 1;


	memcpy(out->data + out->author_off, this->publisher, strlen(this->publisher) + 1);
	memcpy(out->data + out->name_off, this->name, strlen(this->name) + 1);
	memcpy(out->data + out->filename_off, this->filename, strlen(this->filename) + 1);
	memcpy(out->data + out->creator_off, this->creator, strlen(this->creator) + 1);
	memcpy(out->data + out->musician_off, this->musician, strlen(this->musician) + 1);
	memcpy(out->data + out->graphics_artist_off, this->graphics_artist, strlen(this->graphics_artist) + 1);
	memcpy(out->data + out->screenshot_off, png_data, png_sz);

	*out_sz = out->sz;
	/* Marshall it all */
	out->sz = htonl(out->sz);
	out->author_off = htons(out->author_off);
	out->version_magic = htons(out->version_magic);
	out->name_off = htons(out->name_off);
	out->filename_off = htons(out->filename_off);
	out->creator_off = htons(out->creator_off);
	out->musician_off = htons(out->musician_off);
	out->graphics_artist_off = htons(out->graphics_artist_off);
	out->screenshot_off = htons(out->screenshot_off);
	out->score = htons(out->score);
	out->year = htons(out->year);

	return (void *)out;
}

bool GameInfo::fromDump(struct game_info *gi)
{
	bool ret;

	this->freeAll();

	/* Demarshal and convert */
	switch (ntohs(gi->version_magic))
	{
	case VERSION(0):
		ret = from_v0(this, (struct game_info_v0 *)gi); break;
	case VERSION(1):
		ret = from_v1(this, (struct game_info_v1 *)gi); break;
	case VERSION(2):
		ret = from_v2(this, (struct game_info_v2 *)gi); break;
	default:
		/* Garbage, let's return */
		warning("game info garbage magic: %2x\n",
				ntohs(gi->version_magic));
		return false;
	}

	return ret;
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

void GameInfo::setGeneric(const char **what, const char *who)
{
	if (strlen(who) == 0)
		who = " ";
	free((void*)*what);
	*what = xstrdup(who);
	if (strcmp(who, " ") != 0)
		this->score++;
}

void GameInfo::setAuthor(const char *who)
{
	this->setGeneric(&this->publisher, who);
}

void GameInfo::setCreator(const char *who)
{
	this->setGeneric(&this->creator, who);
}

void GameInfo::setMusician(const char *who)
{
	this->setGeneric(&this->musician, who);
}

void GameInfo::setGraphicsArtist(const char *who)
{
	this->setGeneric(&this->graphics_artist, who);
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
