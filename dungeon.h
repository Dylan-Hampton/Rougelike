#ifndef DUNGEON_H
#define DUNGEON_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

//Constants
#define DUNGEON_ROW 21
#define DUNGEON_COL 80
#define MIN_ROOMS    6
#define MAX_ROOMS   10

//Prototypes
void print_dungeon(); //Prints out the dungeon
void set_dungeon(); //Initializes all cells to rock(space)
void create_rooms(); //Creates 6 random rooms of random but minimum size
void create_paths(); //Creates paths between each room
void create_stairs(); //Creates one stair that goes up and one that goes down
void create_player(); //Creates the player and places them in the highest room corner
void set_hardness(); //sets the hardness of the rocks in the dungeon
int save_dungeon(); //Saves dungeon in binary file in the .../.rlg327/dungeon  folder
int load_dungeon(); //Reads dungeon from binary file in the .../.rlg327/dungeon  folder

//Struct defs
typedef struct room {
  int x_pos;
  int y_pos;
  int x_width;
  int y_height;
} room_t;

typedef struct pc {
  int x_pos;
  int y_pos;
} pc_t;

typedef struct stair {
  int x_pos;
  int y_pos;
  int direction;
} stair_t;

typedef struct monster_path {
  heap_node_t *hn; //heap node pointer
  int pos[2]; //r,c
  int cost; 
} monster_path_t;

#endif
