#ifndef __UTILS_H__
#define __UTILS_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_ttf.h>

class Font;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

#define BUG_ON(cond)

#define panic(x...) do \
  { \
    fprintf(stderr, "=============PANIC PANIC PANIC===========\n"); \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); fprintf(stderr, x); \
    fprintf(stderr, "=========================================\n"); \
    assert(0); \
    exit(1); \
  } while(0)

#define warning(x...) do \
  { \
    fprintf(stderr, "==============WARNING WARNING============\n"); \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); fprintf(stderr, x); \
    fprintf(stderr, "=========================================\n"); \
  } while(0)

#define panic_if(cond, x...) \
  do { if ((cond)) panic(x); } while(0)

#define warning_if(cond, x...) \
  do { if ((cond)) warning(x); } while(0)

static inline char *xstrdup(const char *s)
{
  char *out = strdup(s);

  panic_if(!out, "strdup failed");

  return out;
}

static inline void *xmalloc(size_t sz)
{
  void *out = malloc(sz);

  panic_if(!out, "malloc failed");
  memset(out, 0, sz);

  return out;
}

static inline void *xrealloc(void *ptr, size_t sz)
{
  void *out = realloc(ptr, sz);

  panic_if(!out, "malloc failed");

  return out;
}

#define xsnprintf(buf, size, fmt, x...) do { \
    int r = snprintf(buf, size, fmt, x); \
    panic_if(r < 0 || r >= (int)(size), "snprintf failed for %s with %d\n", fmt, r); \
} while(0)

TTF_Font *read_and_alloc_font(const char *path, int pt_size);

bool ext_matches_list(const char *name, const char **exts);

const char **get_file_list(const char *base_dir, const char *exts[]);

void *sdl_surface_to_png(SDL_Surface *src, size_t *out_sz);

SDL_Surface *sdl_surface_from_data(void *data, size_t sz);

void highlight_background(SDL_Surface *where, Font *font,
		SDL_Surface *bg_left, SDL_Surface *bg_middle, SDL_Surface *bg_right,
		int x, int y, int w, int h);

const char *ip_to_str(uint8_t *ip_in);

const char *region_to_str(int region);

SDL_Surface *sdl_surface_8bit_copy(SDL_Surface *src);

#endif /* __UTILS_H__ */
