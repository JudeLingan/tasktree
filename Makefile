all: 
	gcc -Wall -Wextra -lsqlite3 -g -o tasktree main.c tasktree.h tasktree.c

clean:
	rm -f tasktree
