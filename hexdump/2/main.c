/****************************************************************
 * 
 * Desc: hexdump ，附带 ascii 码
 *
 * Author: not me
 *
 **************************************************************/
#include <stdio.h>
#include <string.h>

void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

int main(){
	char buf[40];
	memset(buf, 0, sizeof(buf));
	*(unsigned long *)buf = 0x6f6c6c6548;
	hexdump(buf, sizeof(buf));
}

/*****************************************************************************

$ make
gcc main.c

$ ./a.out
48 65 6C 6C 6F 00 00 00  00 00 00 00 00 00 00 00  |  Hello...........
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |  ................
00 00 00 00 00 00 00 00                           |  ........

*****************************************************************************/
