/*
clock_gettime
http://man7.org/linux/man-pages/man2/clock_getres.2.html


gettimeofday
http://man7.org/linux/man-pages/man2/gettimeofday.2.html

struct timeval {
   time_t      tv_sec;     // seconds 
   suseconds_t tv_usec;    // microseconds 
};
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        struct timeval start;
        struct timeval end;
        long time_use = 0;

        gettimeofday(&start,NULL);
        for(int i=0;i<1000000000;i++){
                //...
        }
        gettimeofday(&end,NULL);
        time_use= end.tv_sec - start.tv_sec;
        printf("Done after %lds\n", time_use);

}
