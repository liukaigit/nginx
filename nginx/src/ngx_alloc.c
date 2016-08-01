#include "ngx_config.h"
#include "ngx_core.h"


ngx_uint_t  ngx_pagesize;

/* 封装了malloc函数 */
void *
ngx_alloc(size_t size, ngx_log_t *log)
{
    void  *p;

    p = malloc(size);
    if (p == NULL) {
    }

    return p;
}

/* 封装了calloc函数，同时对内存清零 */
void *
ngx_calloc(size_t size, ngx_log_t *log)
{
    void  *p;

    p = ngx_alloc(size, log);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}


#if (NGX_HAVE_POSIX_MEMALIGN)

/* 对posix_memalign封装，分配size大小空间，地址按alignment大小对齐 */	
void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);
    if (0 != err) {
        p = NULL;
    }

    return p;
}

#elif (NGX_HAVE_MEMALIGN)

/* 对memalign封装，分配size大小空间，地址按alignment大小对齐 */	
void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;

    p = memalign(alignment, size);
    if (p == NULL) {
    }

    return p;
}

#endif
