#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <png.h>
#include <sys/stat.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <sysdeps.h>
#include <C64.h>
#include <Network.h>

#include "gui/font.hh"
#include "utils.hh"

TTF_Font *read_and_alloc_font(const char *path, int pt_size)
{
	TTF_Font *out;
	SDL_RWops *rw;
	Uint8 *data;
	FILE *fp = fopen(path, "r");
	size_t r;

	if (!fp) {
		fprintf(stderr, "Could not open font %s\n", path);
		return NULL;
	}
	data = (Uint8*)xmalloc(1 * 1024*1024);
	r = fread(data, 1, 1 * 1024 * 1024, fp);
	if (r == 0 || ferror(fp))
	{
		free(data);
		return NULL;
	}
	rw = SDL_RWFromMem(data, 1 * 1024 * 1024);
	if (!rw)
	{
		fprintf(stderr, "Could not create RW: %s\n", SDL_GetError());
		free(data);
		return NULL;
	}
	out = TTF_OpenFontRW(rw, 1, pt_size);
	if (!out)
	{
		fprintf(stderr, "TTF: Unable to create font %s (%s)\n",
				path, TTF_GetError());
	}
	fclose(fp);

	return out;
}



static int cmpstringp(const void *p1, const void *p2)
{
	const char *p1_s = *(const char**)p1;
	const char *p2_s = *(const char**)p2;

	/* Put directories first and handle some special cases */
	if (*p1_s == '[' && *p2_s != '[')
		return -1;
	if (*p1_s != '[' && *p2_s == '[')
		return 1;
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}

/* Return true if name ends with ext (for filenames) */
static bool ext_matches(const char *name, const char *ext)
{
	int len = strlen(name);
	int ext_len = strlen(ext);

	if (len <= ext_len)
		return false;
	return (strcmp(name + len - ext_len, ext) == 0);
}

bool ext_matches_list(const char *name, const char **exts)
{
	for (const char **p = exts; *p; p++)
	{
		if (ext_matches(name, *p))
			return true;
	}

	return false;
}

const char **get_file_list(const char *base_dir, const char *exts[])
{
	DIR *d = opendir(base_dir);
	const char **file_list;
	int cur = 0;
	struct dirent *de;
	int cnt = 16;

	if (!d)
		return NULL;

	file_list = (const char**)malloc(cnt * sizeof(char*));
	file_list[cur++] = strdup("None");
	file_list[cur] = NULL;

	for (de = readdir(d);
	de;
	de = readdir(d))
	{
		char buf[255];
		struct stat st;

		snprintf(buf, 255, "%s/%s", base_dir, de->d_name);
		if (stat(buf, &st) < 0)
			continue;
		if (S_ISDIR(st.st_mode))
		{
			char *p;
			size_t len = strlen(de->d_name) + 4;

			/* We don't need the current dir */
			if (strcmp(de->d_name, ".") == 0)
				continue;
			p = (char*)malloc( len );
			snprintf(p, len, "[%s]", de->d_name);
			file_list[cur++] = p;
			file_list[cur] = NULL;
		}
		else if (ext_matches_list(de->d_name, exts))
		{
			char *p;

			p = strdup(de->d_name);
			file_list[cur++] = p;
			file_list[cur] = NULL;
		}

		if (cur > cnt - 2)
		{
			cnt = cnt + 32;
			file_list = (const char**)realloc(file_list, cnt * sizeof(char*));
			if (!file_list)
				return NULL;
		}
	}
	closedir(d);
        qsort(&file_list[1], cur-1, sizeof(const char *), cmpstringp);

        return file_list;
}


/* PNG writing */
struct  png_write_user_struct
{
	size_t sz;
	void *data;
};

static void user_write_fn(png_structp png_ptr, png_bytep bytes, png_size_t sz)
{
	struct png_write_user_struct *out = (struct png_write_user_struct *)png_ptr->io_ptr;

	out->data = xrealloc(out->data, out->sz + sz);
	memcpy((uint8_t*)out->data + out->sz, bytes, sz);
	out->sz += sz;
}


static int png_colortype_from_surface(SDL_Surface *surface)
{
	int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */

	if (surface->format->palette)
		colortype |= PNG_COLOR_MASK_PALETTE;

	return colortype;
}


static void png_user_warn(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: warning: %s\n", str);
}


static void png_user_error(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: error: %s\n", str);
}

extern SDL_Color sdl_palette[PALETTE_SIZE];

/* This is taken from http://encelo.netsons.org/programming/sdl (GPLed) */
void *sdl_surface_to_png(SDL_Surface *surf, size_t *out_sz)
{
	png_structp png_ptr;
	png_infop info_ptr;
	int i, colortype;
	png_bytep *row_pointers;
	png_colorp palette;
	struct png_write_user_struct out;

	if (!surf)
		return NULL;

	out.sz = 0;
	out.data = NULL;

	/* Initializing png structures and callbacks */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, png_user_error, png_user_warn);
	if (png_ptr == NULL) {
		printf("png_create_write_struct error!\n");
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		printf("png_create_info_struct error!\n");
		exit(-1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		exit(-1);
	}

	png_set_write_fn(png_ptr, (void *)&out, user_write_fn, NULL);

	colortype = png_colortype_from_surface(surf);
	png_set_IHDR(png_ptr, info_ptr, surf->w, surf->h, 8, colortype,	PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	if (colortype & PNG_COLOR_MASK_PALETTE)
	{
		/* Set the palette if there is one.  REQUIRED for indexed-color images */
		palette = (png_colorp)png_malloc(png_ptr,
				PNG_MAX_PALETTE_LENGTH * png_sizeof(png_color));

		/* KLUDGE! For some reason, surf->format->palette doesn't work... */
		for (int i = 0; i < PALETTE_SIZE; i++)
		{
			SDL_Color *p = &sdl_palette[i];

			palette[i].red = p->r;
			palette[i].green = p->g;
			palette[i].blue = p->b;
		}
		/* ... Set palette colors ... */
		png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
	}

	/* Writing the image */
	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);

	row_pointers = (png_bytep*) xmalloc(sizeof(png_bytep)*surf->h);
	for (i = 0; i < surf->h; i++)
		row_pointers[i] = (png_bytep)(Uint8 *)surf->pixels + i*surf->pitch;
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);

	/* Cleaning out... */
	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	*out_sz = out.sz;

	return out.data;
}

SDL_Surface *sdl_surface_from_data(void *data, size_t sz)
{
	SDL_RWops *rw;
	SDL_Surface *out;

	rw = SDL_RWFromMem(data, sz);
	if (!rw)
		return NULL;

	out = IMG_Load_RW(rw, 0);
	SDL_FreeRW(rw);

	return out;
}

SDL_Surface *sdl_surface_8bit_copy(SDL_Surface *src)
{
	Uint32 rmask,gmask,bmask,amask;
	SDL_Surface *out;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	if (!src)
		return NULL;
	if (src->format->BitsPerPixel != 8)
		return NULL;

	out = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 8,
			rmask, gmask, bmask, amask);
	if (!out)
		return NULL;
	memcpy(out->pixels, src->pixels, src->h * src->pitch);

	SDL_SetColors(out, sdl_palette, 0, PALETTE_SIZE);

	return out;
}


void highlight_background(SDL_Surface *where, Font *font,
		SDL_Surface *bg_left, SDL_Surface *bg_middle, SDL_Surface *bg_right,
		int x, int y, int w, int h)
{
	SDL_Rect dst;

	/* Can't highlight without images */
	if (!bg_left ||	!bg_middle || !bg_right)
		return;

	int font_height = font->getHeight("X");
	int bg_y_start = y + font_height / 2 -
			bg_left->h / 2;
	int bg_x_start = x - bg_left->w / 3;
	int bg_x_end = x + w - (2 * bg_right->w) / 3;
	int n_mid = (bg_x_end - bg_x_start) / bg_middle->w;

	/* Left */
	dst = (SDL_Rect){bg_x_start, bg_y_start, 0,0};
	SDL_BlitSurface(bg_left, NULL, where, &dst);

	/* Middle */
	for (int i = 1; i < n_mid; i++)
	{
		dst = (SDL_Rect){bg_x_start + i * bg_middle->w, bg_y_start, 0,0};
		SDL_BlitSurface(bg_middle, NULL, where, &dst);
	}
	dst = (SDL_Rect){bg_x_end - bg_middle->w, bg_y_start, 0,0};
	SDL_BlitSurface(bg_middle, NULL, where, &dst);

	/* Right */
	dst = (SDL_Rect){bg_x_end, bg_y_start, 0,0};
	SDL_BlitSurface(bg_right, NULL,	where, &dst);
}

const char *ip_to_str(uint8_t *ip_in)
{
	char *out = (char *)xmalloc(24);
	int ip[4];

	for (int i = 0; i < 4; i++)
	{
		char tmp[3];
		char *endp;

		tmp[0] = ip_in[i * 2];
		tmp[1] = ip_in[i * 2 + 1];
		tmp[2] = '\0';
		ip[i] = strtoul(tmp, &endp, 16);
		panic_if (endp == (const char*)tmp,
			"Could not convert ip to str.\n");
	}
	sprintf(out, "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);

	return out;
}

const char *region_to_str(int region)
{
	switch (region)
	{
	case REGION_EUROPE: return "Europe";
	case REGION_AFRICA: return "Africa";
	case REGION_NORTH_AMERICA: return "North America";
	case REGION_SOUTH_AMERICA: return "South America";
	case REGION_EAST_ASIA: return "East asia";
	case REGION_SOUTH_ASIA: return "South asia";
	case REGION_MIDDLE_EAST: return "Middle east";
	case REGION_OCEANIA: return "Oceania";
	case REGION_ANTARTICA: return "Antartica"; // Likely, yes
	default:
		break;
	}

	return "Unknown";
}
