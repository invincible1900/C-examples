#include <unistd.h>
#include <stdio.h>
#include <wait.h>

int main(){
	int ret;
	ret = fork();
	if(ret == 0){
		char * argv[] = {"ls", "-l", NULL};
		execve("/bin/ls", argv, NULL);	
		perror("err");
	}

	ret = fork();
	if(ret == 0){
		execl("/bin/ls", "ls", "-l", NULL);
		perror("err");
	}
	
	wait(NULL);
	wait(NULL);
}

