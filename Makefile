flags = -Wall -Wextra -lsqlite3 -g
modules = util.h util.c tasktree.h tasktree.c

all: 
	gcc $(flags) -o tasktree main.c $(modules) $(libs)

test:
	gcc $(flags) -o test test.c $(modules) $(libs)
	valgrind ./test
	rm test

clean:
	rm -f tasktree test
