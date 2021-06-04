#include <stdio.h>

#define NONE "\033[m"
#define YELLOW "\033[1;33m"

void wow(){
	printf(YELLOW"Wow ~\n"NONE);
	return;
}
