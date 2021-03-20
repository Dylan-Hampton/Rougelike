#include "dungeon.h"

// goes up or down the stairs
void interact_stair(char up_or_down, int *num_ent, int *alive_ent, int dungeon_layout[DUNGEON_ROW][DUNGEON_COL], int num_rooms, pc_t pc) {
  int num_mon = *num_ent - 1;
  if ((up_or_down == '<' || up_or_down == '>') 
      && (dungeon_layout[pc.y_pos][pc.x_pos] == TILE_UP
        || dungeon_layout[pc.y_pos][pc.x_pos] == TILE_DOWN))
  {
    spawn_new_dungeon(num_rooms, &num_mon, alive_ent, num_ent);
  }
  else
  {
    //printf("Not on stair tile");
  }
}

// moves the player
int move_player(char direction, int num_ent, int *alive_ent, character_t *entities[DUNGEON_ROW][DUNGEON_COL],
    int dungeon_display[DUNGEON_ROW][DUNGEON_COL], pc_t *pc, heap_t *entities_heap, int dungeon_layout[DUNGEON_ROW][DUNGEON_COL]) {
  int has_killed_monster = 0;
  int x_direction = 0;
  int y_direction = 0;
  int is_resting = 0;
  switch (direction) {
    case '1':// down left
    case 'b':
      x_direction = -1;
      y_direction = 1;
      break;
    case '2':// down
    case 'j':
      y_direction = 1;
      break;
    case '3':// down right
    case 'n':
      x_direction = 1;
      y_direction = 1;
      break;
    case '4':// left
    case 'h':
      x_direction = -1;
      break;
    case '5':// rest
    case ' ':
    case '.':
      is_resting = 1;
      break;
    case '6':// right
    case 'l':
      x_direction = 1;
      break;
    case '7':// up left
    case 'y':
      x_direction = -1;
      y_direction = -1;
      break;
    case '8':// up
    case 'k':
      y_direction = -1;
      break;
    case '9':// up right
    case 'u':
      x_direction = 1;
      y_direction = -1;
      break;
    default:
      //mvprintw(0, 1, "%c", direction);
      return -2;
      break;
  }
  if (is_resting) {
    return 0;
  }
  //checks if movement is valid
  int target_r = pc->y_pos + y_direction;
  int target_c = pc->x_pos + x_direction;
  int target_tile = dungeon_display[target_r][target_c];
  if (target_tile == TILE_ROCK) {
    //printf("player movement invalid: %c\n", direction);
    return -1; 
  }
  //kills the monster if there is a monster in target tile
  if (target_tile >= 10) {
    if(entities[target_r][target_c] != NULL)
    {
      has_killed_monster = 1;
      character_t *temp[num_ent - 1];
      //pulls out whole heap and checks for murdered entity, if so set is_alive = 0
      for(int i = 0; i < num_ent - 1; i++)
      {
        temp[i] = (character_t *) heap_remove_min(entities_heap);	  

        if(temp[i]->x_pos == target_c && temp[i]->y_pos == target_r)
        {
          temp[i]->is_alive = 0;
        }
      } //reinserts temp into heap
      for(int i = 0; i < num_ent - 1; i++)
      {
        heap_insert(entities_heap, temp[i]);
      }
      (*alive_ent)--;
    }
  }
  entities[target_r][target_c] = entities[pc->y_pos][pc->x_pos];
  entities[pc->y_pos][pc->x_pos] = NULL;
  dungeon_display[target_r][target_c] = dungeon_display[pc->y_pos][pc->x_pos];
  dungeon_display[pc->y_pos][pc->x_pos] = dungeon_layout[pc->y_pos][pc->x_pos];
  pc->y_pos = target_r;
  pc->x_pos = target_c;
  if (!has_killed_monster) {
    return 0;
  }
  return 1;
}
