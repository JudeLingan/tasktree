all: 
	gcc -Wall -Wextra -lsqlite3 -g -o tasktree main.c util.h util.c

clean:
	rm -f tasktree
