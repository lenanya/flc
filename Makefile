all: build/flc

CFLAGS = -I./include -ggdb

build/flc: src/main.c include/flc.h include/lexer.h include/parser.h
	cc src/main.c $(CFLAGS) -I./include -o build/flc

build/helloworld.S: build/flc examples/helloworld.fl
	./build/flc examples/helloworld.fl

build/helloworld: build/helloworld.S 
	cc build/helloworld.S -no-pie -o build/helloworld

build/test: src/test.S
	cc src/test.S -o build/test -no-pie -ggdb

clean: 
	rm build/*