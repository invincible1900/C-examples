#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#define PAGE_SIZE 4096
void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}


int main(int argc, char *argv[]){
	char path[1024];
	off_t offset = atoi(argv[2]);
	// off_t base = 0x7feccbe00000;
	off_t base = 0x0;
	pid_t pid;
	int mem_fd;
	char buf[PAGE_SIZE * 2];
	
	if(argc < 3){
		return 0;
		// pid = getpid();
	}else{
		pid = atol(argv[1]);
		offset = strtol(argv[2], NULL, 16);
	}

	sprintf(path, "/proc/%u/mem", pid);
	printf("path: %s, offset: %lu\n", path, offset);

	mem_fd = open(path, O_RDONLY);
	ptrace(PTRACE_ATTACH, pid, NULL, NULL);
	waitpid(pid, NULL, 0);
	lseek(mem_fd, base + offset, SEEK_SET);
	read(mem_fd, buf, PAGE_SIZE);
	ptrace(PTRACE_DETACH, pid, NULL, NULL);
	hexdump(buf, PAGE_SIZE);

}
