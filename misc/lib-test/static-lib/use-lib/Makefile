.PHONY: static
all: main.c
	gcc main.c -o main -I include -L lib -lwow

static: main.c
	gcc -static main.c -o main-static -I include -L lib -lwow

