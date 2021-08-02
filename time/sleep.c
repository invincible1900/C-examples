/*
SLEEP(3)                  Linux Programmer's Manual                 SLEEP(3)

NAME         top
       sleep - sleep for a specified number of seconds

SYNOPSIS         top
       #include <unistd.h>

       unsigned int sleep(unsigned int seconds);

*/
#include <unistd.h>
#include <stdio.h>

int main(){
    for(int i=0;i<10;i++){
        printf("%d\n", i);
        sleep(1);
    }
}
