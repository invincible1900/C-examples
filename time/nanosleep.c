/*
NANOSLEEP(2)              Linux Programmer's Manual             NANOSLEEP(2)
NAME         top
       nanosleep - high-resolution sleep
SYNOPSIS         top
       #include <time.h>
       int nanosleep(const struct timespec *req, struct timespec *rem);
       ...

DESCRIPTION         top
       nanosleep() suspends the execution of the calling thread until either
       at least the time specified in *req has elapsed, or ...
       ...
       The structure timespec is used to specify intervals of time with
       nanosecond precision.  It is defined as follows:

           struct timespec {
               time_t tv_sec;        // seconds 
               long   tv_nsec;       // nanoseconds
           };

       The value of the nanoseconds field must be in the range 0 to
       999999999.

       ...

RETURN VALUE         top
       On successfully sleeping for the requested interval, nanosleep()
       returns 0.  

       If the call is interrupted by a signal handler or
       encounters an error, then it returns -1, with errno set to indicate
       the error.

*/
// nanosecond是纳秒的意思 1秒 = 10^9纳秒

#include <time.h>

int main(){
  struct timespec currentTime;
  currentTime.tv_sec=5;
  currentTime.tv_nsec=1000000000-1;
  nanosleep(&currentTime, NULL);
}

/*
$ time ./nanosleep_demo

real  0m6.016s
...
*/









