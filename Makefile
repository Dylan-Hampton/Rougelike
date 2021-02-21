dungeon: dungeon.c heap.c heap.h
	gcc -Wall -Werror -ggdb3 -g dungeon.c heap.c -o dungeon

clean:
	rm -f dungeon
