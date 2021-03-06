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
#include <iostream>
#include <fstream> //ifstream and ofstream
#include <string>
#include <vector>

//Constants
#define DUNGEON_ROW 21
#define DUNGEON_COL 80
#define DUNGEON_FOW 5
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
#define BIT_PASS    0x10 
#define BIT_PICKUP  0x20 
#define BIT_DESTROY 0x40
#define BIT_UNIQ    0x80
#define BIT_BOSS    0x100

//Struct defs
class room_t {
  public:
  int x_pos;
  int y_pos;
  int x_width;
  int y_height;
};

class pc_t {
  public:
  int x_pos;
  int y_pos;
};

class dice_t {
  public:
    int base;
    int dice;
    int sides;
};

  
class npc_t {
  public:
  std::string name;
  std::string desc;
  int color[8];
  int speed;
  int hp;
  dice_t dam;
  int rarity;
  int x_pos;
  int y_pos;
  int characteristics;
  char type; //symbol
  int generated = 0;
};

class npc_desc_t {
  public:
    std::string name;
    std::string desc;
    int color[8];
    dice_t speed;
    dice_t hp;
    dice_t dam;
    int ability;
    char symb;
    int rarity;

int dice_roll(dice_t d) {
  int sum = 0;
  for (int i = 0; i < d.dice; i++) {
    sum += (rand() % d.sides) + 1; 
  }
  return sum + d.base;
}

  npc_t generate_npc() {
    npc_t output;
    output.name = name;
    output.desc = desc;
    for (int i = 0; i < 8; i++) {
      output.color[i] = color[i];
    }
    output.speed = dice_roll(speed);
    output.hp = dice_roll(hp);
    output.dam = dam;
    output.characteristics = ability;
    output.rarity = rarity;
    output.type = symb;
    return output;
  }
};

class item_desc_t {
  public:
  std::string name;
  std::string desc;
  std::string type;
  int color[8];
  dice_t weight;
  dice_t hit;
  dice_t dam;
  dice_t attr;
  dice_t val;
  dice_t dodge;
  dice_t def;
  dice_t speed;
  std::string art;
  int rarity;
};

class character_t {
  public:
  int sn;
  int x_pos;
  int y_pos;
  int speed;
  int turn;
  npc_t *npc;
  pc_t *pc;
  int is_pc;
  int is_alive;
};

class stair_t {
  public:
  int x_pos;
  int y_pos;
  int direction;
};

class monster_path_t {
  public:
  heap_node_t *hn; //heap node pointer
  int pos[2]; //r,c
  int cost; 
};

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
    heap_t *h, int num_ent, int *alive_ent);

//dungeon.c
int star_movement(int *alive_ent); // moves the star around on the screen 
void update_monster_list(int num_ent); //updates the list of alive monsters
void spawn_new_dungeon(int num_rooms, int *num_mon, int *alive_ent, int *num_ent); //creates a new dungeon with new layout
void print_monster_list(int num_ent, int scroll); //prints full screen monster list
void update_fow(); 
void print_dist_map(int dist_map[DUNGEON_ROW][DUNGEON_COL]); //Prints out the distance map
void print_dungeon(int player_other_action, int toggle);//Prints out the dungeon using ncurses
void print_dungeon_terminal(); //Prints out the dungeon using terminal output
void set_layout(); //sets the layout of dungeon
void set_dungeon(); //Initializes all cells to rock(space)
void create_rooms(int *num_rooms); //Creates 6 random rooms of random but minimum size
void create_paths(int *num_rooms); //Creates paths between each room
void create_stairs(); //Creates one stair that goes up and one that goes down
void create_player(); //Creates the player and places them in the highest room corner
void set_hardness(); //sets the hardness of the rocks in the dungeon
void create_entities(int num_rooms, int *num_monsters); // creates the monsters and player
char get_monster_type(int n); // gets monster type based on number n
void parse_monsters();
void parse_dice(std::string temp, int dice[3]);
std::string get_colors(int i, int toggle);
std::string get_abilities(int i);
void print_monster_desc();
void print_item_desc();
void parse_items();
int dice_roll(dice_t d);
dice_t make_dice(int temp[3]);


//player_movement.c
int teleport_player(int row, int col, int num_ent, int *alive_ent, character_t *entities[DUNGEON_ROW][DUNGEON_COL],
		int dungeon_display[DUNGEON_ROW][DUNGEON_COL], pc_t *pc, heap_t *entities_heap, int dungeon_layout[DUNGEON_ROW][DUNGEON_COL], int dungeon_fow[DUNGEON_ROW][DUNGEON_COL]);  // moves the player in direction
int move_player(char direction, int num_ent, int *alive_ent, character_t *entities[DUNGEON_ROW][DUNGEON_COL],
		int dungeon_display[DUNGEON_ROW][DUNGEON_COL], pc_t *pc, heap_t *entities_heap, int dungeon_layout[DUNGEON_ROW][DUNGEON_COL]);  // moves the player in direction
void interact_stair(char up_or_down, int *num_ent, int *alive_ent, int dungeon_layout[DUNGEON_ROW][DUNGEON_COL], int num_rooms, pc_t pc); // goes up or down the stairs

#endif
