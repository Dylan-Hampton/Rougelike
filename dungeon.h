#ifndef DUNGEON_H
#define DUNGEON_H

#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <limits.h>
#include <ncurses.h>

//Constants
#define DUNGEON_ROW 21
#define DUNGEON_COL 80
#define TILE_ROCK   0
#define TILE_FLOOR  1
#define TILE_CORR   2
#define TILE_DOWN   3
#define TILE_UP     4
#define TILE_PC     5
#define TILE_MON    6
#define MIN_ROOMS   6
#define MAX_ROOMS   10
#define BIT_SMART   0x1 
#define BIT_TELE    0x2 
#define BIT_TUN     0x4 
#define BIT_ERAT    0x8 

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

typedef struct npc {
  int x_pos;
  int y_pos;
  int characteristics;
  char type;
} npc_t;

typedef struct character {
  int sn;
  int x_pos;
  int y_pos;
  int speed;
  int turn;
  npc_t *npc;
  pc_t *pc;
  int is_pc;
  int is_alive;
} character_t;

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

//Prototypes
//path_finding.c
void generate_nonTunnel_dist_map(uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL], int monster_dist[DUNGEON_ROW][DUNGEON_COL], int pc_x, int pc_y);
void generate_tunnel_dist_map(uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL], int monster_dist[DUNGEON_ROW][DUNGEON_COL], int pc_x, int pc_y);
heap_t generate_entities_heap(int num_mon, character_t *entities[DUNGEON_ROW][DUNGEON_COL]);
int next_turn(int dungeon_layout[DUNGEON_ROW][DUNGEON_COL],
	       int dungeon_display[DUNGEON_ROW][DUNGEON_COL],
	       uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL],
	       character_t *entities[DUNGEON_ROW][DUNGEON_COL],
	       int tunnel[DUNGEON_ROW][DUNGEON_COL],
	       int nontunnel[DUNGEON_ROW][DUNGEON_COL],
	      heap_t *h, int num_ent);

//dungeon.c
void interact_stair(char up_or_down); // goes up or down the stairs 
void spawn_new_dungeon(int num_rooms, int num_mon); //creates a new dungeon with new layout
int move_player(char direction); // moves the player in direction 
void print_dist_map(int dist_map[DUNGEON_ROW][DUNGEON_COL]); //Prints out the distance map
void print_dungeon(); //Prints out the dungeon using ncurses
void print_dungeon_terminal(); //Prints out the dungeon using terminal output
void set_layout(); //sets the layout of dungeon
void set_dungeon(); //Initializes all cells to rock(space)
void create_rooms(); //Creates 6 random rooms of random but minimum size
void create_paths(); //Creates paths between each room
void create_stairs(); //Creates one stair that goes up and one that goes down
void create_player(); //Creates the player and places them in the highest room corner
void set_hardness(); //sets the hardness of the rocks in the dungeon
int save_dungeon(); //Saves dungeon in binary file in the .../.rlg327/dungeon  folder
int load_dungeon(); //Reads dungeon from binary file in the .../.rlg327/dungeon  folder
void create_entities(int num_rooms, int *num_monsters); // creates the monsters and player
char get_monster_type(int n); // gets monster type based on number n

#endif
