#include <dlfcn.h>
#include <stdio.h>

void (*wow)(void);

int main(){
	void *h;
	h = dlopen("./libwow.so", RTLD_NOW | RTLD_GLOBAL);
	if(!h){
		printf("error dlopen\n");
		return -1;
	}

	wow = dlsym(h, "wow");
	if(dlerror() != NULL){
		printf("error dlsym\n");
		return -1;
	}

	wow();

	dlclose(h);
	dlerror();

	return 0;
}
