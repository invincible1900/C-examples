#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

long sign_extend(long data, int width){
	int shift = sizeof(long) * 8 - width;
	return data << shift >> shift;
}

long sign_extend_safe(long data, int width){
	if(width > 64 || width < 0){
		exit(-1);
	}

	if(width == 64){
		return data;	
	}

	if((data >> (width-1)) & 0x1){
		// negative
		return data | ~((1UL << width) - 1);
	}else{
		// positive
		return data & ((1UL << width) - 1);
	}
}

int test_sign_extend(){
	printf("0x%lx\n", sign_extend(0x8, 4));
	printf("0x%lx\n", sign_extend(0x8, 5));
	printf("0x%lx\n", sign_extend_safe(0x8, 4));
	printf("0x%lx\n", sign_extend_safe(0x8, 5));
	
	for(int i = 0; i<64; i++){
		printf("0x%lx, ", sign_extend(0x1UL << i, i + 1));
		printf("0x%lx\n", sign_extend_safe(0x1UL << i, i + 1));
	}
}

int test_sub(){
	long base = 0xffff000000000010;
	long off = 0x10;	
	long unsigned_off = 0xfffffffffffffff0;
	printf("off: %lx\n", off);
	printf("off: %lx\n", -off);
	printf("unsigned off: %lx\n", unsigned_off);
	printf("base - off: %lx\n", base - off);
	printf("base + unsigned off: %lx\n", base + unsigned_off);
	printf("base + sign_extend(0x10): %lx\n", base + sign_extend(0x10, 5));
}

int main(){
	test_sign_extend();
	test_sub();
}
