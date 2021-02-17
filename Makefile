dungeon: dungeon.c
	gcc -Wall -Werror -ggdb3 -g dungeon.c -o dungeon

clean:
	rm -f dungeon
