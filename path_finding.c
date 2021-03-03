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

//Moves least turn character
void next_turn(int monster_dist[DUNGEON_ROW][DUNGEON_COL], heap_t *h)
{
  character_t *c = heap_remove_min(h);
  printf("Speed: %d  Turn: %d\n  x: %d  y: %d\n", c->speed, c->turn, c->x_pos, c->y_pos);
  c->turn += (1000 / c->speed);
  //update character position
  heap_insert(h, c);
  //heapify
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
