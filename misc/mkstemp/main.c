/*****************************************************************************************
 * Desc: mkstemp 创建临时文件示例
 *
 * Author: X++D
 *
*****************************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(){
        int fd;

        char temp_path[24];

        strcpy(temp_path, "my.XXXXXX");

        fd = mkstemp(temp_path);

        write(fd, "Hello\n", strlen("Hello\n")+1);

        close(fd);
}

/*****************************************************************************************
OUTPUT: 

$ make
$ ls
a.out  main.c  Makefile
$ ./a.out
$ ls
a.out  main.c  Makefile  my.enMQ45
$ cat my.enMQ45
Hello

*****************************************************************************************/

/*****************************************************************************************
ATTENTION:

        int mkstemp(char *template);

        The last six characters of template must be "XXXXXX" and these are replaced with
        a  string  that  makes  the filename  unique.

        Since it will be modified, template must not be a string constant, but should be
        declared as a character array.

*****************************************************************************************/
