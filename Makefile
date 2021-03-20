dungeon: dungeon.cpp dungeon.h heap.c heap.h path_finding.cpp player_movement.cpp
	g++ -Wall -Werror -ggdb3 -g path_finding.cpp dungeon.cpp heap.c player_movement.cpp -o dungeon -lncurses

clean:
	rm -f dungeon
