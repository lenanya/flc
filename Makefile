all: build/flc

CFLAGS = -I./include -ggdb -Wall -Wno-format -Wno-unused-variable

build/flc: src/main.c include/flc.h include/lexer.h include/parser.h
	cc src/main.c $(CFLAGS) -I./include -o build/flc
	
clean: 
	rm build/*