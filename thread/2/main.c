#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//线程函数
void *test(void *ptr)
{
    int i;
    for(i=0;i<8;i++)
    {
        printf("the pthread running ,count: %d\n",i);
        sleep(1); 
    }

}


int main(void)
{
    pthread_t pId;
    int i,ret;
    //创建子线程，线程id为pId
    ret = pthread_create(&pId,NULL,test,NULL);

    if(ret != 0)
    {
        printf("create pthread error!\n");
        exit(1);
    }

    for(i=0;i < 5;i++)
    {
        printf("main thread running ,count : %d\n",i);
        sleep(1);
    }

    printf("main thread will exit when pthread is over\n");
    //等待线程pId的完成
    pthread_join(pId,NULL);
    printf("main thread  exit\n");

    return 0;

}

