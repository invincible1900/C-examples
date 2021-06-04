all: main.c
	cp libwow.so /usr/lib
	gcc main.c -o main -I include -lwow 

clean:
	rm -f main 
	rm -f /usr/lib/libwow.so
