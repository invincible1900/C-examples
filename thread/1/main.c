// https://www.educative.io/edpresso/how-to-create-a-simple-thread-in-c

/*
man pthread_create

       #include <pthread.h>

       int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);

       The new thread terminates in one of the following ways:

       * It calls pthread_exit(3), specifying an exit status value that is available to another thread in the  same
         process that calls pthread_join(3).

       * It returns from start_routine().  This is equivalent to calling pthread_exit(3) with the value supplied in
         the return statement.

       * It is canceled (see pthread_cancel(3)).

       * Any of the threads in the process calls exit(3), or the main thread performs a return from  main().   This
         causes the termination of all threads in the process.

man pthread_exit
       #include <pthread.h>

       void pthread_exit(void *retval);

*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>

// Global variable:
int data = 0xaabb;
int ret = 0;

void* reader(void* p){
	while(data < 0xbb00){
		printf("[D] reader[%d]: %x\n", *(int *)p, data);
	}
	ret = *(int *)p;
	pthread_exit(&ret);
}


void* writer(void* p){
	while(data < 0xbb00){
		data += 1;
		printf("[D] writer: %x\n", data);
	}
	ret = 100;
	pthread_exit(&ret);

}

int main(void){
  pthread_t id[3];
  int num[2] = { 0, 1 };
  int * _ret;

  pthread_create(&id[0], NULL, writer, NULL);
  pthread_create(&id[1], NULL, reader, &num[0]);
  pthread_create(&id[2], NULL, reader, &num[1]);
    
  pthread_join(id[0], (void**)&_ret);
  printf("[D] main: %p, %p, %i\n", _ret, &ret, *_ret);

  pthread_join(id[1], (void**)&_ret);
  printf("[D] main: %p, %p, %i\n", _ret, &ret, *_ret);

  pthread_join(id[2], (void**)&_ret);
  printf("[D] main: %p, %p, %i\n", _ret, &ret, *_ret);


}
