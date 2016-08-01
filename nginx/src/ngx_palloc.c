
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include "ngx_config.h"
#include "ngx_core.h"


static void *ngx_palloc_block(ngx_pool_t *pool, size_t size);
static void *ngx_palloc_large(ngx_pool_t *pool, size_t size);

/* 创建内存池 */
ngx_pool_t *
ngx_create_pool(size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;
	ngx_pagesize = getpagesize();
	
    p = ngx_memalign(NGX_POOL_ALIGNMENT, size, log);//申请size大小内存，按16字节对齐。
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;
    p->log = log;

    return p;
}

/* 销毁内存池链 */
void
ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;
	
	/* 1.调用用户注册的清理函数 */
    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }

	/* 2.清理大内存块链 */
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

	/* 3.清理内存池链 */
    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_free(p);

        if (n == NULL) {
            break;
        }
    }
}

//回收用户数据内存
void
ngx_reset_pool(ngx_pool_t *pool)
{
    ngx_pool_t        *p;
    ngx_pool_large_t  *l;
	
	//释放大内存块的用户内存
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }
    pool->large = NULL;
	
	//重置内存池的last指针
    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    }
}


/* 从内存池分配size大小空间，返回数据存储的首地址 */
void *
ngx_palloc(ngx_pool_t *pool, size_t size)
{
    u_char      *m;
    ngx_pool_t  *p;

    if (size <= pool->max) {
        p = pool->current;/* 真正开始匹配的内存池 */

        do {
            m = ngx_align_ptr(p->d.last, NGX_ALIGNMENT);//地址对齐

			/* 分配了未使用的空间 */
            if ((size_t) (p->d.end - m) >= size) {
                p->d.last = m + size;
                return m;
            }
			
            p = p->d.next;
        } while (p);

        return ngx_palloc_block(pool, size);
    }
	/* 如果size与max大，则分配更大空间的内存池 */
    return ngx_palloc_large(pool, size);
}

/* 与ngx_palloc功能相同，但不需要地址对齐 */
void *
ngx_pnalloc(ngx_pool_t *pool, size_t size)
{
    u_char      *m;
    ngx_pool_t  *p;

    if (size <= pool->max) {

        p = pool->current;

        do {
            m = p->d.last;

            if ((size_t) (p->d.end - m) >= size) {
                p->d.last = m + size;

                return m;
            }

            p = p->d.next;

        } while (p);

        return ngx_palloc_block(pool, size);
    }

    return ngx_palloc_large(pool, size);
}

/* 创建一个新内存池，返回数据存储的首地址 */
static void *
ngx_palloc_block(ngx_pool_t *pool, size_t size)
{
    u_char      *m;
    size_t       psize;
    ngx_pool_t  *p, *new, *current;

    psize = (size_t) (pool->d.end - (u_char *) pool);

	/* 创建一个与pool同样大小的内存池 */
    m = ngx_memalign(NGX_POOL_ALIGNMENT, psize, pool->log);
    if (m == NULL) {
        return NULL;
    }

    new = (ngx_pool_t *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    new->d.last = m + size;

    current = pool->current;

	/* 设置匹配失败的次数，以及下一次开始匹配的内存池 */
	//如果某个内存池的匹配失败次数超过4次，则使用其下一个内存池作为
	//下一次匹配的开始
    for (p = current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            current = p->d.next;
        }
    }

	/* 将新的内存池添加到内存池链的尾部 */
    p->d.next = new;

    pool->current = current ? current : new;

    return m;
}

/* 创建一个大内存块,返回数据存储的首地址 */
static void *
ngx_palloc_large(ngx_pool_t *pool, size_t size)
{
    void              *p;
    ngx_uint_t         n;
    ngx_pool_large_t  *large;
	//分配数据存储空间
    p = ngx_alloc(size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

	/*�������е�large���ݿ��в����Ƿ���ڿտ飬�������������ڴ�ֱ�ӹ��ϣ����������ڴ��׵�ַ*/
    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {/* 查找没有使用的大块内存池 */
            large->alloc = p;
            return p;
        }

		/* 只检测前面5块(0-4) */
        if (n++ > 3) {
            break;
        }
    }

	/*����ǰ4��large����û���ҵ��տ飬����������һ��large����������ڴ棬������large����ͷ��*/
	/* 在内存池中创建ngx_pool_large_t对象 */
    large = ngx_palloc(pool, sizeof(ngx_pool_large_t));
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

	/* 将新创建的内存池加入到链(头部)中 */
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

/* 分配一个size字节大小内存块，按alignment对齐，保存到内存池的large上*/
/*�������������ͬ�n��gx_palloc_large���������ڿ������벻ͬ���뷽ʽ���ڴ�����һ��Ĭ��ϵͳλ��*/
void *
ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    ngx_pool_large_t  *large;

    p = ngx_memalign(alignment, size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    large = ngx_palloc(pool, sizeof(ngx_pool_large_t));
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

/* 释放large链的数据内存 */
ngx_int_t
ngx_pfree(ngx_pool_t *pool, void *p)
{
    ngx_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            ngx_free(l->alloc);
            l->alloc = NULL;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}

/* 在内存池中申请空间，同时清零*/
void *
ngx_pcalloc(ngx_pool_t *pool, size_t size)
{
    void *p;

    p = ngx_palloc(pool, size);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}

