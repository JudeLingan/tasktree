flags = $(shell pkg-config --cflags gtk4) -Wall -Wextra -lsqlite3 -g $(shell pkg-config --libs gtk4)
modules = util.h util.c tasktree.h tasktree.c

all: 
	gcc $(flags) -o tasktree main.c $(modules) $(libs)

test:
	gcc $(flags) -o test test.c $(modules) $(libs)
	valgrind ./test
	rm test

clean:
	rm -f tasktree test
