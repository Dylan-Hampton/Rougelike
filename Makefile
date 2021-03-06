dungeon: dungeon.c dungeon.h heap.c heap.h path_finding.c
	gcc -Wall -Werror -ggdb3 -g path_finding.c dungeon.c heap.c -o dungeon -lncurses

clean:
	rm -f dungeon
