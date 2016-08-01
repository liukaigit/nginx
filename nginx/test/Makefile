all:nginx

nginx:ngx.c ../src/ngx_alloc.c ../src/ngx_palloc.c
	gcc -o nginx ngx.c ../src/ngx_alloc.c ../src/ngx_palloc.c -I ../src
clean:
	rm nginx
