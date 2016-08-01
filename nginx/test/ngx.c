#include<stdio.h>

#include"ngx_config.h"
#include"ngx_core.h"

void print(char *ptr){
	printf("--------------------------\n");
	printf("         %s      \n",ptr);
	printf("--------------------------\n");
}
int main(int argc,char *argv[]){
	ngx_pool_t *p = NULL;
	print("create pool---->5*1024-->p");
	p = ngx_create_pool(5*1024,NULL);   //创建内存池，存储在p
	
	print("print pool");
	printf("p->max = %d\n",p->max);		//内存池允许申请的最大内存
	printf("p = %p,p->d.last = %p,p->d.end = %p\n",p,p->d.last,p->d.end);  //打印数据段首末地址
	
	print("ngx_palloc-2*1024--->ptr1");
	char *ptr1 = ngx_palloc(p,2*1024);      //申请内存池，存储到ptr1
	printf("ptr1 = %p,p->d.last = %p,p->d.end = %p\n",ptr1,p->d.last,p->d.end);
	strcpy(ptr1,"Hello ptr2,I'm ptr1.");

	print("ngx_palloc-2*1024--->ptr2");
	char *ptr2 = ngx_palloc(p,2*1024);      //申请内存池，存储到ptr2
	printf("ptr2 = %p,p->d.last = %p,p->d.end = %p\n",ptr2,p->d.last,p->d.end);
	strcpy(ptr2,ptr1);
	printf("ptr2 = %s\n",ptr2);      		//打印从ptr1拷贝的内容，检查是否正确

	print("ngx_palloc-5*1024---ptr3");
	char *ptr3 = ngx_palloc(p,5*1024);		//申请大块内存
	printf("p = %p,p->d.last = %p,p->d.end = %p\n",p,p->d.last,p->d.end);
	if(p->large)
		printf("p->large = %p,p->large->alloc = %p\n",p->large,p->large->alloc);
	printf("p = %p,p->d.last = %p,p->d.end = %p\n",p,p->d.last,p->d.end);
	print("ngx_palloc-5*1024---ptr4");
	char *ptr4 = ngx_palloc(p,5*1024);
	printf("p = %p,p->d.last = %p,p->d.end = %p\n",p,p->d.last,p->d.end);
	if(p->large)
		printf("p->large = %p,p->large->alloc = %p\n",p->large,p->large->alloc);
	printf("p = %p,p->d.last = %p,p->d.end = %p\n",p,p->d.last,p->d.end);

	print("ngx_palloc-2*1024--->ptr6");
	char *ptr6 = ngx_palloc(p,2*1024);      //申请内存池，存储到ptr6
	printf("ptr6 = %p,p->d.next->d.last = %p,p->d.next->d.end = %p\n",ptr6,p->d.next->d.last,p->d.next->d.end);
	strcpy(ptr6,ptr1);
	printf("ptr6 = %s\n",ptr6);      		//打印从ptr1拷贝的内容，检查是否正确

	if(p->d.next)
		printf("p = %p,p->d.next = %p,p->current = %p\n",p,p->d.next,p->current);

	print("ngx_palloc-512--->ptr7");
	char *ptr7 = ngx_palloc(p,512);      //申请内存池，存储到ptr7
	printf("ptr7 = %p,p->d.last = %p,p->d.end = %p\n",ptr7,p->d.last,p->d.end);
	strncpy(ptr7,ptr1,strlen(ptr1));
	printf("ptr7 = %s\n",ptr7);      		//打印从ptr1拷贝的内容，检查是否正确

	print("failed.");
	if(p->d.next)
		printf("p->d.failed = %d,p->d.next->d.failed = %d\n",p->d.failed,p->d.next->d.failed);

	print("ngx_palloc-2*1024--->ptr8");
	char *ptr8 = ngx_palloc(p,2*1024);      //申请内存池，存储到ptr8
	printf("ptr8 = %p,p->d.next->d.last = %p,p->d.next->d.end = %p\n",ptr8,p->d.next->d.last,p->d.next->d.end);
	strcpy(ptr8,ptr1);
	printf("ptr6 = %s\n",ptr8);      		//打印从ptr1拷贝的内容，检查是否正确


	print("Reset pool");
	ngx_reset_pool(p);
	print("print pool");
	printf("p->max = %d\n",p->max);		//内存池允许申请的最大内存
	printf("p = %p,p->d.last = %p,p->d.end = %p\n",p,p->d.last,p->d.end);  //打印数据段首末地址

	print("ngx_palloc-2*1024--->ptr10");
	char *ptr10 = ngx_pcalloc(p,2*1024);      //申请内存池，但是没有清零，存储到ptr2
	printf("ptr10 = %p,p->d.last = %p,p->d.end = %p\n",ptr10,p->d.last,p->d.end);
	strncpy(ptr10,"Oh,god  ",7);
	printf("ptr10 = %s\n",ptr10);

	print("destroy pool");
	ngx_destroy_pool(p);
	p = NULL;
	return 0;	
}
