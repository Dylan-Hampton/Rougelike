dungeon: dungeon.c dungeon.h heap.c heap.h path_finding.c player_movement.c
	gcc -Wall -Werror -ggdb3 -g path_finding.c dungeon.c heap.c player_movement.c -o dungeon -lncurses

clean:
	rm -f dungeon
