all: build/flc build/generation

CFLAGS = -I./include -ggdb

build/flc: src/main.c include/flc.h
	cc src/main.c $(CFLAGS) -I./include -o build/flc

build/generation.S: build/flc
	./build/flc > ./build/generation.S 

build/generation: build/generation.S 
	cc build/generation.S -no-pie -o build/generation

build/test: src/test.S
	cc src/test.S -o build/test -no-pie -ggdb

clean: 
	rm build/*