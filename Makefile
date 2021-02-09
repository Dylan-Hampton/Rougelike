dungeon: dungeon.c
	gcc -Wall -Werror -ggdb3 dungeon.c -o dungeon

clean:
	rm -f dungeon
