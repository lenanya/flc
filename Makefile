all: build/flc

CFLAGS = -I./include

build/flc: src/main.c include/flc.h build/flc.o
	cc src/main.c $(CFLAGS) build/flc.o -I./include -o build/flc

build/flc.o: src/flc.c include/flc.h 
	cc src/flc.c $(CFLAGS) -c -o build/flc.o

run: build/flc
	./build/flc

clean: 
	rm build/*