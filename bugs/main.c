// 移位运算潜在溢出的bug
#include <stdio.h>

#define MASK_64 1<<63
#define MASK_64_OK 1lu<<63

//#define MASK_32 1<<32
//#define MASK_32_OK 1<<32

int func1(){
	long ff = 0x8000000000000000;
	if(ff & MASK_64){
		printf("True\n");
			
	}else{
		printf("False\n");	
	}

}

int func2(){
	long ff = 0x8000000000000000;
	if(ff & MASK_64_OK){
		printf("True\n");
			
	}else{
		printf("False\n");	
	}
}

int main(){
	func1();
	func2();
}

