#include <unistd.h>
#include <stdio.h>

/***************************
access
0-检查文件是否存在
1-检查文件是否可运行
2-检查文件是否可写访问
4-检查文件是否可读访问
6-检查文件是否可读/写访问
*****************************/
int file_exists(char *filename)
{
   return (access(filename, 0) == 0);
}

int main(){

        if(file_exists("Makefile")){
                printf("Good\n");
        }

        return 0;

}
