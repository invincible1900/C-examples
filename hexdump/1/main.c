#include <stdio.h>
#include <stdint.h>
#include <string.h>

void hexdump(char *buf, int len){
        int i;
        for(i = 0; i < len; i++){
                printf("%02x ", buf[i]);
                if( (i+1) % 8 == 0){
                        printf("\t");
                }
                if( (i+1) % 16 == 0){
                        printf("\n");
                }
        }
}

int main()
{
	char buf[100];
	memset(buf, 'a', 100);
	hexdump(buf, 100);
   return 0;
}

/* 
output:

	61 61 61 61 61 61 61 61 	61 61 61 61 61 61 61 61 	
	61 61 61 61 61 61 61 61 	61 61 61 61 61 61 61 61 	
	61 61 61 61 61 61 61 61 	61 61 61 61 61 61 61 61 	
	61 61 61 61 61 61 61 61 	61 61 61 61 61 61 61 61 	
	61 61 61 61 61 61 61 61 	61 61 61 61 61 61 61 61 	
	61 61 61 61 61 61 61 61 	61 61 61 61 61 61 61 61 	
	61 61 61 61 

 * */
