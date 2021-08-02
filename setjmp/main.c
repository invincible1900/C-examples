#include <setjmp.h>
#include <stdio.h>
jmp_buf env;

void A(){
    printf("Now A\n");
    longjmp(env,1); //这里跳回到if中继续执行
}

void B(){
    printf("Now B\n");
}

int main()
{
    if (!setjmp(env)) //setjmp初次返回0,第二次回来返回的就是1
        A();
    else
        B();
    return 1;
}

/************
>> 执行结果:
Now A
Now B
************/

