#ifndef __UTILS_H__
#define __UTILS_H__

#define BUG_ON(cond)

#define panic(x...) do \
  { \
    fprintf(stderr, "============Translator panic===========\n"); \
    fprintf(stderr, x); \
    fprintf(stderr, "=======================================\n"); \
    exit(1); \
  } while(0)

#define panic_if(cond, x...) \
  do { if ((cond)) panic(x); } while(0)

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

  return out;
}


#define xsnprintf(buf, size, fmt, x...) do { \
    int r = snprintf(buf, size, fmt, x); \
    panic_if(r < 0 || r >= (int)(size), "snprintf failed for %s with %d\n", fmt, r); \
} while(0)

#endif /* __UTILS_H__ */
