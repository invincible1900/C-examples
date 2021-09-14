#include <stdio.h>

int main(){
	int x = 0xe;
	int y = x << (32 - 4) >> (32 -4);
	int z = x << (32 - 5) >> (32 - 5);
	printf("0x%x, %d\n", x, x);	
	printf("0x%x, %d\n", y, y);	
	printf("0x%x, %d\n", z, z);	
}

/*
SAR: Shift Arithmetic Right

the SAR instruction sets or clears the most significant bit to correspond 
to the sign (most significant bit) of the original value in the destination operand. 

In effect, the SAR instruction fills the empty bit position’s shifted value with the sign of the unshifted value .

(gdb) disassemble main
Dump of assembler code for function main:
...
   0x000055555555465c <+18>:	shl    eax,0x1c
   0x000055555555465f <+21>:	sar    eax,0x1c
...
   0x0000555555554668 <+30>:	shl    eax,0x1b
   0x000055555555466b <+33>:	sar    eax,0x1b
...
End of assembler dump. 


$ make
gcc main.c -g
./a.out
0xe, 14
0xfffffffe, -2
0xe, 14
 * */
