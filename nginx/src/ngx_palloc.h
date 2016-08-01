#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include "ngx_config.h"
#include "ngx_core.h"


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef struct ngx_pool_s        ngx_pool_t;
typedef struct ngx_log_s         ngx_log_t;
typedef struct ngx_chain_s       ngx_chain_t;


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;/* 清理函数 */
    void                 *data;/* 传给函数的数据 */
    ngx_pool_cleanup_t   *next;/* 下一个清理对象 */
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next; /* 下一个大块区域 */
    void                 *alloc;/* 空间的起始地址 */
};


typedef struct {
    u_char               *last;/* 可用空间的起始地址 */
    u_char               *end;/* 可用空间的结束地址 */
    ngx_pool_t           *next;/* 下一个内存池 */
    ngx_uint_t            failed;/* 匹配失败的次数 */
} ngx_pool_data_t;


struct ngx_pool_s {
    ngx_pool_data_t       d;
    size_t                max; /* 用于衡量申请的size是否为large块 */
    ngx_pool_t           *current;/* 下一次分配空间时开始匹配的内存池 */
    ngx_chain_t          *chain;/* 缓冲区链 */
    ngx_pool_large_t     *large;/* 大的数据块，大小超过了max */
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};


ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);

#endif /* _NGX_PALLOC_H_INCLUDED_ */
