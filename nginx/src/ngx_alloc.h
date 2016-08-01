#ifndef _NGX_ALLOC_H_INCLUDED_
#define _NGX_ALLOC_H_INCLUDED_


#include "ngx_config.h"
#include "ngx_core.h"


#define NGX_HAVE_POSIX_MEMALIGN 1


void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);


#define ngx_free          free
#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)
#define ngx_memset(buf, c, n)     (void) memset(buf, c, n)


#if (NGX_HAVE_POSIX_MEMALIGN || NGX_HAVE_MEMALIGN)
void *ngx_memalign(size_t alignment, size_t size, ngx_log_t *log);
#else
#define ngx_memalign(alignment, size, log)  ngx_alloc(size, log)
#endif


extern ngx_uint_t  ngx_pagesize;


#endif /* _NGX_ALLOC_H_INCLUDED_ */
