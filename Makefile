dungeon: dungeon.c
	gcc -Wall -Werror -ggdb3 dungeon.c -lm -o dungeon

clean:
	rm -f dungeon
