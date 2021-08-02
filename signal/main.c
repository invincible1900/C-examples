#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sig_handler(int sig){
        printf("sig: %d\n", sig);
        return;
}

int main(){
	printf("pid: %d\n", getpid());
        for(int i = SIGHUP; i<= SIGRTMAX; i++){
                signal(i, sig_handler);
        }

        while(1){
                sleep(1);
        }
}
