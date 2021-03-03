#include "heap.h"
#include "dungeon.h"

//prototypes
static int32_t monster_path_cmp(const void *key, const void *with);
//void generate_nonTunnel_dist_map(uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL], int monster_dist[DUNGEON_ROW][DUNGEON_COL], int pc_x, int pc_y);
//void generate_tunnel_dist_map(uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL], int monster_dist[DUNGEON_ROW][DUNGEON_COL], int pc_x, int pc_y);

//returns + if 1st bigger, - if second bigger, 0 if equal
//Most of this code is borrowed from Sheaffer
static int32_t monster_path_cmp(const void *key, const void *with) {
  return ((monster_path_t *) key)->cost - ((monster_path_t *) with)->cost;
}

static int32_t character_move_cmp(const void *key, const void *with) {
  return ((character_t *) key)->turn - ((character_t *) with)->turn;
}

heap_t generate_entities_heap(int num_mon, character_t *entities[DUNGEON_ROW][DUNGEON_COL])
{
  heap_t h;

  heap_init(&h, character_move_cmp, NULL);
  //Filling the heap with all entities
  for (int r = 0 ; r < DUNGEON_ROW; r++) {
    for (int c = 0 ; c < DUNGEON_COL; c++) {
      if (entities[r][c] != NULL) {
        heap_insert(&h, entities[r][c]);
      }
    }  
  }  
  return h;
}
/**
   
dungeon_layout is to remember what tile the entities are standing on
dungeon_display is what the current dungeon looks like
dungeon_hardness is //TODO in future will use to chip away at rocks monsters tunnel through
entities keeps track of all the entities
tunnel is tunneling map distances to player
nontunneling is nontunnelting map distances to player

 */
//Moves least turn character
int next_turn(int dungeon_layout[DUNGEON_ROW][DUNGEON_COL],
	       int dungeon_display[DUNGEON_ROW][DUNGEON_COL],
	       uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL],
	       character_t *entities[DUNGEON_ROW][DUNGEON_COL],
	       int tunnel[DUNGEON_ROW][DUNGEON_COL],
	       int nontunnel[DUNGEON_ROW][DUNGEON_COL],
	       heap_t *h)
{
  character_t *c = heap_remove_min(h);
  c->turn += (1000 / c->speed);
  if(c->is_alive == 1)
    {
      //printf("Speed: %d  Turn: %d\n  x: %d  y: %d\n", c->speed, c->turn, c->x_pos, c->y_pos);
      int smart = 0;
      int tele = 0;
      int tun = 0;
      int erat = 0;
      //initializing map 
      int map[DUNGEON_ROW][DUNGEON_COL];
      for (int r = 0; r < DUNGEON_ROW; r++) {
	for (int c = 0; c < DUNGEON_COL; c++) {
	  map[r][c] = nontunnel[r][c];
	}
      }
      //setting characteristics
      if (!c->is_pc && c->npc->characteristics & BIT_SMART) {
	smart = 1;
      } if (!c->is_pc && c->npc->characteristics & BIT_TELE) {
	tele = 1;
      } if (!c->is_pc && c->npc->characteristics & BIT_TUN) {
	tun = 1;
      } if (!c->is_pc && c->npc->characteristics & BIT_ERAT) {
	erat = 1;
      }

      if (tun) {
	for (int r = 0; r < DUNGEON_ROW; r++) {
	   for (int c = 0; c < DUNGEON_COL; c++) {
	    map[r][c] = tunnel[r][c];
	  }
        }
      }
      
      if(erat && (rand() % 2 == 0))
      {
	
      }
      else
      {
	if (smart) {
	  //use line of sight and remembers where it last saw them
	}
	//set map to tunneling if tun is true  
	//update character position

	if (tele) {
	  int min = INT_MAX;
	  int min_y;
	  int min_x;
	  //finding min move
	  for (int i = -1; i <= 1; i++) {
	    for (int j = -1; j <= 1; j++) {
	      if(c->y_pos + i >= 0 && c->y_pos + i < DUNGEON_ROW && c->x_pos + j >= 0 && c->x_pos + j < DUNGEON_COL) {
	        if (map[c->y_pos + i][c->x_pos + j] < min) {
		  min = map[c->y_pos + i][c->x_pos + j];
		  min_y = c->y_pos + i;
		  min_x = c->x_pos + j;
		}
	      }
	    }
	  }
	  //killing any monsters standing on next tile
	  if (entities[min_y][min_x] != NULL) {
	    entities[min_y][min_x]->is_alive = 0;
	    dungeon_display[min_y][min_x] = dungeon_layout[min_y][min_x];
	    
	    if(entities[min_y][min_x]->is_pc)
	    {
	      return -1;
	    }
	  }	  
      
	  //moving monster to next tile and replacing current display tile r,c with underneath tile
	  //if monster breaks rock and moves
	  if(dungeon_display[min_y][min_x] == TILE_ROCK && (dungeon_hardness[min_y][min_x] - 85) < 0)
	  {
	    dungeon_hardness[min_y][min_x] = 0;
	    dungeon_layout[min_y][min_x] = TILE_CORR;

	    dungeon_display[min_y][min_x] = dungeon_display[c->y_pos][c->x_pos];
	    dungeon_display[c->y_pos][c->x_pos] = dungeon_layout[c->y_pos][c->x_pos];
	    c->y_pos = min_y;
	    c->x_pos = min_x;
	  } //monster digs, may or may not break rock, but doesn't move to corridor
	  else if(dungeon_display[min_y][min_x] == TILE_ROCK && (dungeon_hardness[min_y][min_x] - 85) >= 0)
	  {
	    dungeon_hardness[min_y][min_x] -= 85;
	    if(dungeon_hardness[min_y][min_x] == 0)
	    {
	      dungeon_layout[min_y][min_x] = TILE_CORR;
	    }
	  } //if moving to already room / corridor / stairs
	  else if(dungeon_hardness[min_y][min_x] == 0)
	  {
	    dungeon_display[min_y][min_x] = dungeon_display[c->y_pos][c->x_pos];
	    dungeon_display[c->y_pos][c->x_pos] = dungeon_layout[c->y_pos][c->x_pos];
	    c->y_pos = min_y;
	    c->x_pos = min_x;
	  }
	  else
	  {
	    printf("ERROR");
	  }
	}
      }                
  }

  heap_insert(h, c);
  
  if(c->is_pc)
  {
    return 1;
  }
  return 0;
}

//Most of this code is borrowed from Sheaffer
void generate_nonTunnel_dist_map(uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL], int monster_dist[DUNGEON_ROW][DUNGEON_COL], int pc_x, int pc_y)
{
  static monster_path_t path[DUNGEON_ROW][DUNGEON_COL], *p;
  static uint32_t initialized = 0;
  heap_t h;
  int r = 0, c = 1;
  uint32_t x, y;

  //Initalize heap with positions
  if (!initialized)
  {
    for (y = 0; y < DUNGEON_ROW; y++)
    {
      for (x = 0; x < DUNGEON_COL; x++)
      {
        path[y][x].pos[r] = y;
        path[y][x].pos[c] = x;
      }
    }
    initialized = 1;
  }

  //Initial heap costs to infinity
  for (y = 0; y < DUNGEON_ROW; y++)
  {
    for (x = 0; x < DUNGEON_COL; x++)
    {
      path[y][x].cost = INT_MAX;
    }
  }

  //Initialize pc to start
  path[pc_y][pc_x].cost = 0;

  heap_init(&h, monster_path_cmp, NULL);
  //Adding any valid moves to the heap
  for (y = 0; y < DUNGEON_ROW; y++)
  {
    for (x = 0; x < DUNGEON_COL; x++)
    {
      if ((dungeon_hardness[y][x] == 0) && !(y > DUNGEON_ROW - 1 || y < 0) && !(x > DUNGEON_COL - 1 || x < 0))
      {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h)))
  {
    p->hn = NULL;

    //Above node (y-1)
    if ((path[p->pos[r] - 1][p->pos[c]    ].hn) &&
        (path[p->pos[r] - 1][p->pos[c]    ].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] - 1][p->pos[c]    ].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] - 1]
          [p->pos[c]    ].hn);
    }
    //Left node (x-1)
    if ((path[p->pos[r]    ][p->pos[c] - 1].hn) &&
        (path[p->pos[r]    ][p->pos[c] - 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r]    ][p->pos[c] - 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r]    ]
          [p->pos[c] - 1].hn);
    }
    //Right node (x+1)
    if ((path[p->pos[r]    ][p->pos[c] + 1].hn) &&
        (path[p->pos[r]    ][p->pos[c] + 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r]    ][p->pos[c] + 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r]    ]
          [p->pos[c] + 1].hn);
    }
    //Down node (y+1)
    if ((path[p->pos[r] + 1][p->pos[c]    ].hn) &&
        (path[p->pos[r] + 1][p->pos[c]    ].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] + 1][p->pos[c]    ].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] + 1]
          [p->pos[c]    ].hn);
    }
    //Top left corner (y-1,x-1)
    if ((path[p->pos[r] - 1][p->pos[c] - 1].hn) &&
        (path[p->pos[r] - 1][p->pos[c] - 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] - 1][p->pos[c] - 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] - 1]
          [p->pos[c] - 1].hn);
    }
    //Bottom left corner (y+1,x-1)
    if ((path[p->pos[r] + 1][p->pos[c] - 1].hn) &&
        (path[p->pos[r] + 1][p->pos[c] - 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] + 1][p->pos[c] - 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] + 1]
          [p->pos[c] - 1].hn);
    }
    //Top right corner (y-1,x+1)
    if ((path[p->pos[r] - 1][p->pos[c] + 1].hn) &&
        (path[p->pos[r] - 1][p->pos[c] + 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] - 1][p->pos[c] + 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] - 1]
          [p->pos[c] + 1].hn);
    }
    //Bottom right corner (y+1,x+1)
    if ((path[p->pos[r] + 1][p->pos[c] + 1].hn) &&
        (path[p->pos[r] + 1][p->pos[c] + 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] + 1][p->pos[c] + 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] + 1]
          [p->pos[c] + 1].hn);
    }
  }
  for (int r = 0; r < DUNGEON_ROW; r++) {
    for (int c = 0; c < DUNGEON_COL; c++) {
      monster_dist[r][c] = path[r][c].cost;
    }
  }
}

//Most of this code is borrowed from Sheaffer
void generate_tunnel_dist_map(uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL], int monster_dist[DUNGEON_ROW][DUNGEON_COL], int pc_x, int pc_y)
{
  static monster_path_t path[DUNGEON_ROW][DUNGEON_COL], *p;
  static uint32_t initialized = 0;
  heap_t h;
  int r = 0, c = 1;
  uint32_t x, y;

  //Initalize heap with positions
  if (!initialized)
  {
    for (y = 0; y < DUNGEON_ROW; y++)
    {
      for (x = 0; x < DUNGEON_COL; x++)
      {
        path[y][x].pos[r] = y;
        path[y][x].pos[c] = x;
      }
    }
    initialized = 1;
  }

  //Initial heap costs to infinity
  for (y = 0; y < DUNGEON_ROW; y++)
  {
    for (x = 0; x < DUNGEON_COL; x++)
    {
      path[y][x].cost = INT_MAX;
    }
  }

  //Initialize pc to start
  path[pc_y][pc_x].cost = 0;

  heap_init(&h, monster_path_cmp, NULL);
  //Adding any valid moves to the heap
  for (y = 0; y < DUNGEON_ROW; y++)
  {
    for (x = 0; x < DUNGEON_COL; x++)
    {
      if ((dungeon_hardness[y][x] != 255) && !(y > DUNGEON_ROW - 1 || y < 0) && !(x > DUNGEON_COL - 1 || x < 0))
      {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h)))
  {
    p->hn = NULL;

    //Above node (y-1)
    if ((path[p->pos[r] - 1][p->pos[c]    ].hn) &&
        (path[p->pos[r] - 1][p->pos[c]    ].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] - 1][p->pos[c]    ].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] - 1]
          [p->pos[c]    ].hn);
    }
    //Left node (x-1)
    if ((path[p->pos[r]    ][p->pos[c] - 1].hn) &&
        (path[p->pos[r]    ][p->pos[c] - 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r]    ][p->pos[c] - 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r]    ]
          [p->pos[c] - 1].hn);
    }
    //Right node (x+1)
    if ((path[p->pos[r]    ][p->pos[c] + 1].hn) &&
        (path[p->pos[r]    ][p->pos[c] + 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r]    ][p->pos[c] + 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r]    ]
          [p->pos[c] + 1].hn);
    }
    //Down node (y+1)
    if ((path[p->pos[r] + 1][p->pos[c]    ].hn) &&
        (path[p->pos[r] + 1][p->pos[c]    ].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] + 1][p->pos[c]    ].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] + 1]
          [p->pos[c]    ].hn);
    }
    //Top left corner (y-1,x-1)
    if ((path[p->pos[r] - 1][p->pos[c] - 1].hn) &&
        (path[p->pos[r] - 1][p->pos[c] - 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] - 1][p->pos[c] - 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] - 1]
          [p->pos[c] - 1].hn);
    }
    //Bottom left corner (y+1,x-1)
    if ((path[p->pos[r] + 1][p->pos[c] - 1].hn) &&
        (path[p->pos[r] + 1][p->pos[c] - 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] + 1][p->pos[c] - 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] + 1]
          [p->pos[c] - 1].hn);
    }
    //Top right corner (y-1,x+1)
    if ((path[p->pos[r] - 1][p->pos[c] + 1].hn) &&
        (path[p->pos[r] - 1][p->pos[c] + 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] - 1][p->pos[c] + 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] - 1]
          [p->pos[c] + 1].hn);
    }
    //Bottom right corner (y+1,x+1)
    if ((path[p->pos[r] + 1][p->pos[c] + 1].hn) &&
        (path[p->pos[r] + 1][p->pos[c] + 1].cost >
         p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85))) {
      path[p->pos[r] + 1][p->pos[c] + 1].cost =
        p->cost + 1 + (dungeon_hardness[p->pos[r]][p->pos[c]] / 85);
      heap_decrease_key_no_replace(&h, path[p->pos[r] + 1]
          [p->pos[c] + 1].hn);
    }
  }
  for (int r = 0; r < DUNGEON_ROW; r++) {
    for (int c = 0; c < DUNGEON_COL; c++) {
      monster_dist[r][c] = path[r][c].cost;
    }
  }
}
