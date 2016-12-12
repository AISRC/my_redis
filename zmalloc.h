#ifndef __ZMALLOC_H__
#define __ZMALLOC_H__

#include <malloc.h>

#define zmalloc malloc
#define zfree free
#define zrealloc realloc
#define zcalloc(x) calloc(x, 1)
#define zstrdup strdup

#endif
