#include <stdio.h>
#include <errno.h>
#include <string.h>

/*********************************
$ ./a.out 
[-] main:43
[-] main:44, Invalid argument

$ man errno
$ man strerror
 
*********************************/

#define SAFE_1(expr) \
	while(1) { \
		if(expr < 0){ \
			printf("[-] %s:%d\n", __func__, __LINE__); \
		} \
		break; \
	}


#define SAFE_2(expr) \
	while(1) { \
		ret = expr; \
		if(ret < 0){ \
			printf("[-] %s:%d, %s\n", __func__, __LINE__, strerror(-ret)); \
		} \
		break; \
	}

int foo(){
	return -1;
}

int bar(){
	return -EINVAL;
}

int main(){
	int ret = 0;
	SAFE_1(foo());
	SAFE_2(bar());
}


