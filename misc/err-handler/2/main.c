// https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html
#include <errno.h>
#include <stdio.h>

int func(int num){
	errno = num;
	return -1;
}

int main(){
	char buf[10];
	for(int i = EPERM; i<= EHWPOISON; i++){
		func(i);
		sprintf(buf, "[%d]", i);
		perror(buf);
	}
}

