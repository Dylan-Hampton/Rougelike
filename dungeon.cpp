#include "dungeon.h"

using namespace std;

//Globals
int dungeon_tunnel_map[DUNGEON_ROW][DUNGEON_COL]; // distance map for tunneling monsters
int dungeon_non_tunnel_map[DUNGEON_ROW][DUNGEON_COL]; // distance map for non-tunneling monsters
int dungeon_layout[DUNGEON_ROW][DUNGEON_COL];
int dungeon_fow[DUNGEON_ROW][DUNGEON_COL];
int dungeon_star[DUNGEON_ROW][DUNGEON_COL];
int dungeon_display[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for outputting text 
uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for hardness level  hardness goes from 0 - 255 (0 meaning room or corridor, 255 meaning immutable)
room_t *rooms;
pc_t pc;
stair_t *upstairs;
stair_t *downstairs;
character_t *entities[DUNGEON_ROW][DUNGEON_COL];
heap_t entities_heap;
int num_ent;
int num_rooms;
npc_t *monster_list;
// stores all monster descs
vector<npc_desc_t> monster_descriptions;
vector<item_desc_t> item_descriptions;

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int num_mon = 10;
  num_rooms = 0;
  char player_next_move;
  int alive_ent;

  if(argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      if(!strcmp(argv[i], "--nummon") || !strcmp(argv[i], "-m"))
      {
        num_mon = atoi(argv[++i]);
      }
    }
  }

  upstairs = (stair_t *) malloc(sizeof(stair_t));
  downstairs = (stair_t *) malloc(sizeof(stair_t));
  rooms = (room_t *) malloc(MIN_ROOMS * sizeof(room_t));
  num_ent = num_mon + 1;
  alive_ent = num_ent;
  spawn_new_dungeon(num_rooms, &num_mon, &alive_ent, &num_ent);
  monster_list = (npc_t *) malloc(num_ent * sizeof(npc_t));

  free(upstairs);
  free(downstairs);
  free(rooms);
  initscr();
  noecho();
  raw();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_BLACK, COLOR_WHITE, COLOR_BLACK);
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);

  generate_nonTunnel_dist_map(dungeon_hardness, dungeon_non_tunnel_map, pc.x_pos, pc.y_pos);
  generate_tunnel_dist_map(dungeon_hardness, dungeon_tunnel_map, pc.x_pos, pc.y_pos);

  int pc_state = 1;
  num_ent = num_mon + 1;
  int pc_other_action = 0; // 1 = killed monster -1 = hit a wall 0 = default
  int is_in_mon_list = 0, was_mon_list = 0;
  int scroll = 0;
  int fow_toggle = 0;
  update_fow();
  parse_monsters();
  parse_items();

  while(1)
  {
    if (is_in_mon_list == 0 && was_mon_list == 0) {
      // sets the monster lists and takes all the turns until it is the player turn
      generate_tunnel_dist_map(dungeon_hardness, dungeon_tunnel_map, pc.x_pos, pc.y_pos);
      generate_nonTunnel_dist_map(dungeon_hardness, dungeon_non_tunnel_map, pc.x_pos, pc.y_pos);
      pc_state = next_turn(dungeon_layout, dungeon_display,
          dungeon_hardness, entities, dungeon_tunnel_map,
          dungeon_non_tunnel_map, &entities_heap, num_ent, &alive_ent);
    }


    if((pc_state > 0 && num_ent > 1) || was_mon_list)
    {
      if (is_in_mon_list) {
        print_monster_list(alive_ent, scroll);
        int input = getch();
        if (input == 'Q') {
          //quits game
          endwin();
          break;
        } else if (input == 27) { //27 is esc key
          //exits monster list
          was_mon_list = 1;
          scroll = 0;
          is_in_mon_list = 0;
        } else if (input == KEY_DOWN && scroll < alive_ent && alive_ent > DUNGEON_ROW && DUNGEON_ROW + scroll < alive_ent) {
          //scrolls monster list up
          scroll++;
        } else if (input == KEY_UP && scroll > 0) {
          //scrolls monster list down
          scroll--;
        }	
      } else {
        // if in regular player loop
        was_mon_list = 0;
        print_dungeon(pc_other_action, fow_toggle);
        player_next_move = getch();
        if(player_next_move == 'g') {
          int star = star_movement(&alive_ent);
          if (star) {
            endwin();
            break;
          }
        } else if (player_next_move == 'f' || player_next_move == 'F') {
          fow_toggle = (fow_toggle + 1) % 2;
        } else if (player_next_move == 'Q') {
          endwin();
          break;
        } else if (player_next_move == 'm') {
          //go to monster list
          is_in_mon_list = 1;
        } else if (player_next_move == '>' || player_next_move == '<') {
          //go up or down stairs
          interact_stair(player_next_move, &num_ent, &alive_ent, dungeon_layout, num_rooms, pc);
        } else {
          //move pc to specified place
          pc_other_action = move_player(player_next_move, num_ent, &alive_ent, entities, dungeon_display, &pc, &entities_heap, dungeon_layout);
          update_fow();
          if (pc_other_action < 0) {
            was_mon_list = 1;
          }
        }
        update_monster_list(num_ent);
    }    
  } 

  if(pc_state > 0 && alive_ent == 1)
  {
    endwin();
    print_dungeon_terminal();
    printf("                      _.--.    .--._\n");
    printf("                    .'  .'      '.  '.\n");
    printf("                   ;  .'    /\\    '.  ;\n");
    printf("                   ;  '._,-/  \\-,_.`  ;\n");
    printf("                   \\  ,`  / /\\ \\  `,  /\n");
    printf("                    \\/    \\/  \\/    \\/\n");
    printf("                    ,=_    \\/\\/    _=,\n");
    printf("                    |  '_   \\/   _'  |\n");
    printf("                    |_   ''-..-''   _|\n");
    printf("                    | '-.        .-' |\n");
    printf("                    |    '\\    /'    |\n");
    printf("                    |      |  |      |\n");
    printf("            ___     |      |  |      |     ___\n");
    printf("        _,-',  ',   '_     |  |     _'   ,'  ,'-,_\n");
    printf("      _(  \\  \\   \\'=--'-.  |  |  .-'--='/   /  /  )_\n");
    printf("    ,'  \\  \\  \\   \\      '-'--'-'      /   /  /  /  '.\n");
    printf("   !     \\  \\  \\   \\                  /   /  /  /     !\n");
    printf("   :      \\  \\  \\   \\                /   /  /  /      :\n");
    printf("\n                       PLAYER WINS!\n");
    break;
  }
  else if(pc_state < 0)
  {
    endwin();
    print_dungeon_terminal();
    printf("                            ,-.\n");
    printf("       ___,---.__          /'|`\\          __,---,___\n");
    printf("    ,-'    \\`    `-.____,-'  |  `-.____,-'    //    `-.\n");
    printf("  ,'        |           ~'\\     /`~           |        `.\n");
    printf(" /      ___//              `. ,'          ,  , \\___      \\\n");
    printf("|    ,-'   `-.__   _         |        ,    __,-'   `-.    |\n");
    printf("|   /          /\\_  `   .    |    ,      _/\\          \\   |\n");
    printf("\\  |           \\ \\`-.___ \\   |   / ___,-'/ /           |  /\n");
    printf(" \\  \\           | `._   `\\\\  |  //'   _,' |           /  /\n");
    printf("  `-.\\         /'  _ `---'' , . ``---' _  `\\         /,-'\n");
    printf("     ``       /     \\    ,='/ \\`=.    /     \\       ''\n");
    printf("             |__   /|\\_,--.,-.--,--._/|\\   __|\n");
    printf("             /  `./  \\\\`\\ |  |  | /,//' \\,'  \\\n");
    printf("            /   /     ||--+--|--+-/-|     \\   \\\n");
    printf("           |   |     /'\\_\\_\\ | /_/_/`\\     |   |\n");
    printf("            \\   \\__, \\_     `~'     _/ .__/   /\n");
    printf("             `-._,-'   `-._______,-'   `-._,-'\n");
    printf("\n                       MONSTERS WIN!\n");
    break;
  }
}

return 0;
}

int star_movement(int *alive_ent) {
  int output = 0;
  int star_row = pc.y_pos;
  int star_col = pc.x_pos;
  print_dungeon(0, 1);
  mvwaddch(stdscr, star_row + 1, star_col, '*');
  int x_direction = 0;
  int y_direction = 0;
  char direction = getch();
  int random_x = (rand() % (DUNGEON_COL - 2)) + 1;
  int random_y = (rand() % (DUNGEON_ROW - 2)) + 1;
  while (direction != 'g') {
    if (direction == 'r') {
      star_row = random_y;
      star_col = random_x;
      break;
    }
    switch (direction) {
      case '1':// down left
      case 'b':
        x_direction = -1;
        y_direction = 1;
        break;
      case '2':// down
      case 'j':
        y_direction = 1;
        x_direction = 0;
        break;
      case '3':// down right
      case 'n':
        x_direction = 1;
        y_direction = 1;
        break;
      case '4':// left
      case 'h':
        x_direction = -1;
        y_direction = 0;
        break;
      case '6':// right
      case 'l':
        y_direction = 0;
        x_direction = 1;
        break;
      case '7':// up left
      case 'y':
        x_direction = -1;
        y_direction = -1;
        break;
      case '8':// up
      case 'k':
        x_direction = 0;
        y_direction = -1;
        break;
      case '9':// up right
      case 'u':
        x_direction = 1;
        y_direction = -1;
        break;
      default:
        break;
    }
    if (star_row + y_direction >= 0 && star_col + x_direction >= 0 && star_row + y_direction < DUNGEON_ROW && star_col + x_direction < DUNGEON_COL) {
      star_row += y_direction;
      star_col += x_direction;
      print_dungeon(0, 1);
    } else {
      print_dungeon(-1, 1);
    }  
    mvwaddch(stdscr, star_row + 1, star_col, '*');
    direction = getch();
  }
  if (star_row + y_direction >= 0 && star_col + x_direction >= 0 && star_row + y_direction < DUNGEON_ROW && star_col + x_direction < DUNGEON_COL) {
    output = teleport_player(star_row, star_col, num_ent, alive_ent, entities, dungeon_display, &pc, &entities_heap, dungeon_layout, dungeon_fow);
    update_fow();
    print_dungeon(0, 0);
  } else {
    print_dungeon(-1, 1);
  }
  return output;
}

void update_monster_list(int num_ent) {
  free(monster_list);
  monster_list = (npc_t *) malloc(num_ent * sizeof(npc_t));
  character_t *temp[num_ent - 1];
  int index = 0;
  for(int i = 0; i < num_ent - 1; i++)
  {
    temp[i] = (character_t *) heap_remove_min(&entities_heap);	  

    if(temp[i]->is_alive && !(temp[i]->is_pc))
    {
      npc_t npc;
      npc.x_pos = temp[i]->npc->x_pos;
      npc.y_pos = temp[i]->npc->y_pos;
      npc.characteristics = temp[i]->npc->characteristics;
      npc.type = temp[i]->npc->type;
      monster_list[index++] = npc;
    }
  } //reinserts temp into heap
  for(int i = 0; i < num_ent - 1; i++)
  {
    heap_insert(&entities_heap, temp[i]);
  }
}

// makes a new dungeon (used for going upstairs and downstairs)
void spawn_new_dungeon(int num_rooms, int *num_mon, int *alive_ent, int *num_ent) {
  //clears the entities array
  for (int r = 0; r < DUNGEON_ROW; r++) {
    for (int c = 0; c < DUNGEON_COL; c++) {
      entities[r][c] = NULL;
      dungeon_fow[r][c] = TILE_ROCK;
    }
  }
  set_dungeon();
  create_rooms(&num_rooms);
  create_stairs();
  create_paths(&num_rooms);
  set_layout();
  create_player();
  create_entities(num_rooms, num_mon);
  *num_ent = *num_mon + 1;
  *alive_ent = *num_ent;
  set_hardness();
  entities_heap = generate_entities_heap(*num_mon, entities);
}

// updates the fog of war
void update_fow() {
  for (int r = -DUNGEON_FOW; r <= DUNGEON_FOW; r++) {
    for (int c = -DUNGEON_FOW; c <= DUNGEON_FOW; c++) {
      if (pc.y_pos + r >= 0 && pc.x_pos + c >= 0 && pc.y_pos + r < DUNGEON_ROW && pc.x_pos + c < DUNGEON_COL) {
        dungeon_fow[pc.y_pos + r][pc.x_pos + c] = dungeon_display[pc.y_pos + r][pc.x_pos + c];
      }
    }
  }
}

void create_entities(int num_rooms, int *num_monsters) {
  int seq = 0;
  //intializing player and placing into entities
  character_t *player = (character_t *) malloc(sizeof(character_t));
  player->sn = seq++;
  player->x_pos = pc.x_pos;
  player->y_pos = pc.y_pos;
  player->speed = 10;
  player->turn = 0;
  player->is_pc = 1;
  player->pc = &pc;
  player->is_alive = 1;
  entities[pc.y_pos][pc.x_pos] = player;
  //finds which room the player is in so no monster spawn there
  int player_room = 0;
  for (int room = 0; room < num_rooms; room++)
  {
    for (int x = rooms[room].x_pos; x < rooms[room].x_pos + rooms[room].x_width; x++)
    {
      for (int y = rooms[room].y_pos; y < rooms[room].y_pos + rooms[room].y_height; y++)
      {
        if (x == pc.x_pos && y == pc.y_pos)
        {
          player_room = room; 
        }
      }
    }
  }

  //places num_monsters randomly within the rooms (if no more space is available none are placed)
  int attempts = 0;
  int spawned_mon = 0;
  while (*num_monsters > 0 && attempts < 1000)
  {
    for (int room = 0; room < num_rooms; room++)
    {
      if (room != player_room)
      {
        int x = rooms[room].x_pos + (rand() % rooms[room].x_width);
        int y = rooms[room].y_pos + (rand() % rooms[room].y_height);
        if (dungeon_display[y][x] == TILE_FLOOR && *num_monsters > 0)
        {
          //initializing monster
          dungeon_display[y][x] = 10;
          int size = monster_descriptions.size();
          int rand_desc;
          int rand_rarity;
          for (int i = 0; i < 2000; i++) {
            rand_desc = rand() % size;
            npc_t current_mon = monster_descriptions[rand_desc].generate_npc();
            if ((current_mon.characteristics & BIT_UNIQ && current_mon.generated == 0
                ) || !(current_mon.characteristics & BIT_UNIQ)) {
                rand_rarity = rand() % 100;
                if (rand_rarity < current_mon.rarity) {
                  current_mon.generated++;
                  current_mon.x_pos = x;
                  current_mon.y_pos = y;
                  character_t *monster = (character_t *) malloc(sizeof(character_t));
                  monster->sn = seq++;
                  monster->x_pos = x;
                  monster->y_pos = y;
                  monster->turn = 0;
                  monster->is_pc = 0;
                  monster->npc = &current_mon;
                  monster->is_alive = 1;
                  entities[y][x] = monster;
                  spawned_mon++;
                  (*num_monsters)--;
                }
            }
          }
/*
          npc_t *npc = (npc_t *) malloc(sizeof(npc_t));
          switch (mon_type % 4) {
            case 0:
              monster->speed = 5;
              break;
            case 1:
              monster->speed = 10;
              break;
            case 2:
              monster->speed = 15;
              break;
            case 3:
              monster->speed = 20;
              break;
          }
*/
        }
      }
    }
    attempts++;
  }

  *num_monsters = spawned_mon;
}

//returns respective monster character based on number
char get_monster_type(int n) {
  switch(n)
  {
    case 0:
      return '0';
      break;
    case 1:
      return '1';
      break;
    case 2:
      return '2';
      break;
    case 3:
      return '3';
      break;
    case 4:
      return '4';
      break;
    case 5:
      return '5';
      break;
    case 6:
      return '6';
      break;
    case 7:
      return '7';
      break;
    case 8:
      return '8';
      break;
    case 9:
      return '9';
      break;
    case 10:
      return 'a';
      break;
    case 11:
      return 'b';
      break;
    case 12:
      return 'c';
      break;
    case 13:
      return 'd';
      break;
    case 14:
      return 'e';
      break;
    case 15:
      return 'f';
      break;
  }
  return 'Z';
}

void set_layout() {
  for (int r = 0; r < DUNGEON_ROW; r++)
  {
    for (int c = 0; c < DUNGEON_COL; c++)
    {
      if (dungeon_display[r][c] == TILE_PC) {
        dungeon_layout[r][c] = TILE_FLOOR;
      } else {
        dungeon_layout[r][c] = dungeon_display[r][c];
      }
    }
  }
}

void set_hardness() { //sets any non-rock to zero hardness
  for (int r = 0; r < DUNGEON_ROW; r++)
  {
    for (int c = 0; c < DUNGEON_COL; c++)
    {
      if (dungeon_display[r][c] != TILE_ROCK) {
        dungeon_hardness[r][c] = 0;
      }
    }
  }
}

void create_player() {
  int player = 1;
  // creates the player in the highest room corner
  for (int y = 0; y < DUNGEON_ROW; y++)
  {
    for (int x = 0; x < DUNGEON_COL; x++)
    {
      if (dungeon_display[y][x] == TILE_FLOOR && player)
      {
        dungeon_display[y][x] = TILE_PC;
        dungeon_hardness[y][x] = 0;
        player = 0;
        pc.x_pos = x;
        pc.y_pos = y;
      }
    }
  }	
}

void create_stairs() {
  int upperstair = 1;
  //adds upper staircase highest room corner
  for (int y = 0; y < DUNGEON_ROW; y++)    
  {
    for (int x = 0; x < DUNGEON_COL; x++)
    {
      if (dungeon_display[y][x] == TILE_FLOOR && upperstair)
      {
        dungeon_display[y + 1][x + 1] = TILE_UP;
        dungeon_hardness[y + 1][x + 1] = 0;
        upperstair = 0;
        upstairs[0].x_pos = x + 1;
        upstairs[0].y_pos = y + 1;
        upstairs[0].direction = 1;
      }
    }
  }
  int lowerstair = 1;
  //adds lower staircase to lowest room corner
  for (int y = DUNGEON_ROW - 1; y >= 0; y--)
  {
    for (int x = DUNGEON_COL - 1; x >= 0; x--)
    {
      if (dungeon_display[y][x] == TILE_FLOOR && lowerstair)
      {
        dungeon_display[y - 1][x - 1] = TILE_DOWN;
        dungeon_hardness[y - 1][x - 1] = 0;
        lowerstair = 0;
        downstairs[0].x_pos = x - 1;
        downstairs[0].y_pos = y - 1;
        downstairs[0].direction = 0;
      }
    }
  }
}

// creates the paths between rooms
void create_paths(int *num_rooms){

  //connects all rooms to intial room
  for(int i = 1; i < *num_rooms; i++)
  {
    int room_center_x = rooms[i].x_pos + (rooms[i].x_width / 2);
    int room_center_y = rooms[i].y_pos + (rooms[i].y_height / 2);
    int x = rooms[0].x_pos + (rooms[0].x_width / 2);
    int y = rooms[0].y_pos + (rooms[0].y_height / 2);

    //goes up / down to room[i] y coordinate 
    while(y != room_center_y)
    {
      if (room_center_y > y)
      {
        y++;
      }  
      else
      {
        y--;
      }

      //sets rock to corridor
      if (dungeon_display[y][x] == TILE_ROCK && dungeon_display[y][x+1] != TILE_CORR && dungeon_display[y][x-1] != TILE_CORR)
      {
        dungeon_display[y][x] = TILE_CORR;
        dungeon_hardness[y][x] = 0;
      }
    }

    //goes left / right to room[i] x coordinate
    while(x != room_center_x)
    {
      if (room_center_x > x)
      {
        x++;
      }
      else
      {
        x--;
      }

      //sets rock to corridor
      if (dungeon_display[y][x] == TILE_ROCK && dungeon_display[y+1][x] != TILE_CORR && dungeon_display[y-1][x] != TILE_CORR)
      {
        dungeon_display[y][x] = TILE_CORR;
        dungeon_hardness[y][x] = TILE_ROCK;
      }
    }
  }
}

void create_rooms(int *num_rooms){
  // int totalRooms = (rand() % 3) + 6;
  int roomCount = 0;
  int attempts = 0;
  *num_rooms = MIN_ROOMS;

  while(roomCount != MIN_ROOMS && attempts < 2000)
  {
    int x_coord = rand() % DUNGEON_COL; //0-79
    int y_coord = rand() % DUNGEON_ROW; //0-20
    int x_dim = (rand() % 4) + 4; //4-7 can change if want
    int y_dim = (rand() % 4) + 3; //3-6 can change if want
    int placement_successful = 1; //Can't get boolean to work? But works like bool

    for(int r = -1; r <= 1; r++)
    {
      for(int c = -1; c <= 1; c++)
      {
        int y = y_coord+r;
        int x = x_coord+c;

        //Used to check if out of bounds
        if((y > DUNGEON_ROW - 1 || y < 0) || (x > DUNGEON_COL - 1 || x < 0) || y+y_dim > DUNGEON_ROW - 1 || x+x_dim > DUNGEON_COL - 1)
        {
          placement_successful = 0;
        }

        //Checks each corner for overlap
        else if(dungeon_display[y][x] != TILE_ROCK || dungeon_display[y + y_dim][x] != TILE_ROCK ||
            dungeon_display[y][x+x_dim] != TILE_ROCK || dungeon_display[y+y_dim][x+x_dim] != TILE_ROCK)
        {
          placement_successful = 0;
        }

        //Checks if two rooms are touching by checking the perimeter of room
        else
        {
          for(int j = x_coord; j <= x_coord + x_dim; j++)
          {
            for(int i = y_coord; i <= y_coord + y_dim; i++)
            {
              if(dungeon_display[i][x_coord-1] != TILE_ROCK || dungeon_display[y_coord-1][j] != TILE_ROCK ||
                  dungeon_display[(y_coord+y_dim)+1][j] != TILE_ROCK || dungeon_display[i][(x_coord + x_dim)+1] != TILE_ROCK)
              {
                placement_successful = 0;
              }
            }
          }
        }
      }
    }

    //Sets cells if place is available
    if(placement_successful)
    {
      for(int r = y_coord; r < y_coord + y_dim; r++)
      {
        for(int c = x_coord; c < x_coord + x_dim; c++)
        {
          dungeon_display[r][c] = TILE_FLOOR;
          dungeon_hardness[r][c] = 0;
        }
      }

      rooms[roomCount].x_width = x_dim;
      rooms[roomCount].y_height = y_dim;
      rooms[roomCount].x_pos = x_coord;
      rooms[roomCount].y_pos = y_coord;
      roomCount++;
      attempts = 0;
    }

    //Ends if too many failed attempts in a row
    else
    {
      attempts++; 
    }
  }
}

void set_dungeon(){
  //Initializes dungeon, sets all rock display with randomized hardness factors
  for(int y = 0; y < DUNGEON_ROW; y++)
  {
    for(int x = 0; x < DUNGEON_COL; x++)
    {
      dungeon_display[y][x] = TILE_ROCK;
      dungeon_hardness[y][x] = (rand() % 254) + 1; //1-254
    }
  }
}

void print_dist_map(int dist_map[DUNGEON_ROW][DUNGEON_COL]){
  for (int r = 0; r < DUNGEON_ROW; r++)
  {
    for (int c = 0; c < DUNGEON_COL; c++)
    {
      if (r == pc.y_pos && c == pc.x_pos) {
        printf("@");
      } else if (dist_map[r][c] <= 0){
        printf("X");
      } else if (dist_map[r][c] != INT_MAX) {
        printf("%d", (dist_map[r][c] % 10));		
      } else if (dungeon_hardness[r][c] == 0) { //janky but works
        printf("X");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
}

//prints the dungeon with ncurses
void print_dungeon(int player_other_action, int toggle) {
  //initializing the screen and turning on keyboard input
  clear();
  int dungeon_temp[DUNGEON_ROW][DUNGEON_COL];
  for (int r = 0; r < DUNGEON_ROW; r++) {
    for (int c = 0; c < DUNGEON_COL; c++) {
      if (toggle) {
        dungeon_temp[r][c] = dungeon_display[r][c];
      } else {
        dungeon_temp[r][c] = dungeon_fow[r][c];
      }
    }
  }
  if (player_other_action == -1) {
    mvprintw(0, 1, "There is a wall in the way!"); 
  } else if (player_other_action > 0) {
    mvprintw(0, 1, "Slayed a monster!"); 
  }
  char currentchar;
  for (int r = 0; r < DUNGEON_ROW; r++) {
    for (int c = 0; c < DUNGEON_COL; c++) {
      switch(dungeon_temp[r][c])
      {
        case TILE_ROCK:
          currentchar = ' ';
          break;
        case TILE_FLOOR:
          currentchar = '.';
          break;

        case TILE_CORR:
          currentchar = '#';
          break;

        case TILE_DOWN:
          currentchar = '>';
          break;

        case TILE_UP:
          currentchar = '<';
          break;

        case TILE_PC:
          currentchar = '@';
          break;

          //Monster 10-25 (n - 10 for type)
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
          currentchar = get_monster_type(dungeon_temp[r][c] - 10);
          break;

        default:
          currentchar = '!';
          break;
      }
      mvwaddch(stdscr, r + 1, c, currentchar);
    }  
  }  
  refresh();
}

// prints the monster list
void print_monster_list(int alive_ent, int scroll) {
  clear();
  int y_dir;
  int x_dir;
  const char *y_string;
  const char *x_string;
  char adj[][18] = {"the normal", "the smart", "the stinky", "the shaggy", "the slow",
    "the vile", "the giant", "the powerful", "the dumb", "the bloodthirsty",
    "the dangerous", "the greedy", "the terrifying", "the tenacious", "the grotesque", "the savage"};

  if(alive_ent > DUNGEON_ROW && DUNGEON_ROW + scroll < alive_ent)
  {
    alive_ent = scroll + DUNGEON_ROW;
  }

  for (int r = scroll; r < alive_ent - 1; r++) {
    y_dir = pc.y_pos - monster_list[r].y_pos;
    x_dir = pc.x_pos - monster_list[r].x_pos;
    if (y_dir < 0) {
      y_string = "south";
      y_dir *= -1;
    } else {
      y_string = "north";
    }
    if (x_dir < 0) {
      x_string = "east";
      x_dir *= -1;
    } else {
      x_string = "west";
    }
    if (y_dir == 0) {
      mvprintw(r - scroll, 1, "%3d| %16s %c, %2d %s", r, adj[monster_list[r].characteristics], monster_list[r].type, x_dir, x_string);
    } if (x_dir == 0) {
      mvprintw(r - scroll, 1, "%3d| %16s %c, %2d %s", r, adj[monster_list[r].characteristics], monster_list[r].type, y_dir, y_string);
    } else {
      mvprintw(r - scroll, 1, "%3d| %16s %c, %2d %s and %2d %s", r, adj[monster_list[r].characteristics], monster_list[r].type, y_dir, y_string, x_dir, x_string); 
    }
  }
  refresh();
}

// 0 == rock(space)
// 1 == room floor('.')
// 2 == corridor('#')
// 3 == downstairs('>')
// 4 == upstairs('<')
// 5 == player character('@')
// 6 == monster('type')
void print_dungeon_terminal(){

  for(int r = 0; r < DUNGEON_ROW; r++)
  {
    for(int c = 0; c < DUNGEON_COL; c++)
    {
      switch(dungeon_display[r][c])
      {
        case TILE_ROCK:
          printf(" ");
          break;
        case TILE_FLOOR:
          printf(".");
          break;

        case TILE_CORR:
          printf("#");
          break;

        case TILE_DOWN:
          printf(">");
          break;

        case TILE_UP:
          printf("<");
          break;

        case TILE_PC:
          printf("@");
          break;

          //Monster 10-25 (n - 10 for type)
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
          printf("%c", get_monster_type(dungeon_display[r][c] - 10));
          break;

        default:
          printf("Error");
          break;
      }

    }
    printf("\n");
  }
}

void parse_monsters()
{
  string monFilePath = strcat(getenv("HOME"),"/.rlg327/monster_desc.txt");
  ifstream monFile(monFilePath);
  // temp is current string being worked on
  string temp;
  // following are monster characteristics that are grabbed from the monster desc
  string name;
  string desc;
  int color[8];
  int speed[3];
  int hp[3];
  int dam[3];
  int ability;
  char symb;
  int rarity;
  // counter is number of characteristics parsed currently
  int counter = 0;
  getline(monFile, temp);

  if(temp == "RLG327 MONSTER DESCRIPTION 1")
  {
    while(monFile.peek() != EOF) //goes until end of file
    {
      getline(monFile, temp);
      if(temp == "BEGIN MONSTER")
      {
        //cout << "\n" << endl;
        // resets all attributes after each new desc 
        ability = 0;
        desc = "";
        counter = 0;
        for(int i = 0; i < 8; i++)
        {
          color[i] = -1;
        }

        while(temp != "END")
        {
          getline(monFile, temp);
          // gets the monster name
          if(temp.length() > 4 && temp.substr(0,4) == "NAME")
          {
            counter++;
            name = temp.substr(5);
            //cout << name << endl;
          }
          // gets the monster symbol
          if(temp.length() > 4 && temp.substr(0,4) == "SYMB") 
          {
            counter++;
            symb = temp[5];
            //cout << symb << endl;
          }
          // gets the monster rarity
          if(temp.length() > 4 && temp.substr(0,4) == "RRTY") 
          {
            counter++;
            rarity = atoi(temp.substr(5).c_str());
            //cout << rarity << endl;
          }
          // uses parse_dice to get monster hp
          if(temp.length() > 2 && temp.substr(0,2) == "HP")
          {
            counter++;
            parse_dice(temp.substr(3), hp);
            //cout << hp[0] << hp[1] << hp[2] << endl;
          }
          // uses parse_dice to get monster damage
          if(temp.length() > 3 && temp.substr(0,3) == "DAM")
          {
            counter++;
            parse_dice(temp.substr(4), dam);
            //cout << temp.substr(0,3) << temp.substr(4) << endl;
          }
          // uses parse_dice to get monster speed
          if(temp.length() > 5 && temp.substr(0,5) == "SPEED") 
          {
            counter++;
            parse_dice(temp.substr(6), speed);
            //cout << temp.substr(0,5) << temp.substr(6) << endl;
          }
          // parses through line word by word, and sets to equivalent integer
          if(temp.length() > 5 && temp.substr(0,5) == "COLOR")
          {
            counter++;
            int i = 6;
            int lasti = i;
            int colorIndex = 0;
            int length = temp.length();
            // gets characters until whitespace
            while (i < length) {
              while(temp[i] != ' ' && i < length){
                i++;
              }

              if(temp.substr(lasti, i - lasti) == "BLACK"){
                color[colorIndex++] = 0;
              }
              else if(temp.substr(lasti, i - lasti) == "RED"){
                color[colorIndex++] = 1;
              }
              else if(temp.substr(lasti, i - lasti) == "GREEN"){
                color[colorIndex++] = 2;
              }
              else if(temp.substr(lasti, i - lasti) == "YELLOW"){
                color[colorIndex++] = 3;
              }
              else if(temp.substr(lasti, i - lasti) == "BLUE"){
                color[colorIndex++] = 4;
              }
              else if(temp.substr(lasti, i - lasti) == "MAGENTA"){
                color[colorIndex++] = 5;
              }
              else if(temp.substr(lasti, i - lasti) == "CYAN"){
                color[colorIndex++] = 6;
              }
              else if(temp.substr(lasti, i - lasti) == "WHITE"){
                color[colorIndex++] = 7;
              }
              else {
                color[colorIndex++] = -1;
              }
              i++;
              lasti = i;
            }
            //cout << temp.substr(0,5) << temp.substr(6) << endl;
          }
          // parses through line word by word, and sets ability to appropriate binary number
          if(temp.length() > 4 && temp.substr(0,4) == "ABIL")
          {
            counter++;
            int i = 5;
            int lasti = i;
            int length = temp.length();
            // gets characters till encounters whitespace
            while (i < length) {
              while(temp[i] != ' ' && i < length){
                i++;
              }

              if(temp.substr(lasti, i - lasti) == "SMART"){
                ability = ability|BIT_SMART;
              }
              else if(temp.substr(lasti, i - lasti) == "TELE"){
                ability = ability|BIT_TELE;
              }
              else if(temp.substr(lasti, i - lasti) == "TUNNEL"){
                ability = ability|BIT_TUN;
              }
              else if(temp.substr(lasti, i - lasti) == "ERRATIC"){
                ability = ability|BIT_ERAT;
              }
              else if(temp.substr(lasti, i - lasti) == "PASS"){
                ability = ability|BIT_PASS;
              }
              else if(temp.substr(lasti, i - lasti) == "PICKUP"){
                ability = ability|BIT_PICKUP;
              }
              else if(temp.substr(lasti, i - lasti) == "DESTROY"){
                ability = ability|BIT_DESTROY;
              }
              else if(temp.substr(lasti, i - lasti) == "UNIQ"){
                ability = ability|BIT_UNIQ;
              }
              else if(temp.substr(lasti, i - lasti) == "BOSS"){
                ability = ability|BIT_BOSS;
              }
              else {
                ability = -1;
              }
              i++;
              lasti = i;
            }
            //cout << temp.substr(0,4) << temp.substr(5) << endl;
          }
          // reads the next lines(77 chars max) until it encounters a singular ".", appends into desc string
          if(temp == "DESC")
          {
            counter++;
            getline(monFile, temp);
            while (temp != ".") {
              desc += "\n" + temp.substr(0, 77);
              getline(monFile, temp);
            }
            desc = desc.substr(1);
            //cout << desc << endl;
          }
        }
        // if all characteristics are not set
        if (counter != 9) {
          cout << "Error: Invalid Monster Description" << endl;
        } else {
          npc_desc_t m;
          m.speed = make_dice(speed);
          m.hp = make_dice(hp);
          m.dam = make_dice(dam);

          m.name = name;
          m.desc = desc;
          for(int i = 0; i < 8; i++)
          {
            m.color[i] = color[i];
          }

          m.ability = ability;
          m.symb = symb;
          m.rarity = rarity;

          monster_descriptions.push_back(m);
          //cout << monster_descriptions.size() << endl;
        }
      }
    }
  }
  else
  {
    cout << "Error: Invalid File Format" << endl;
  }
}

void parse_dice(std::string temp, int dice[3])
{
  int i = 0, j = 0;
  while(temp[i] != '+')
  {
    i++;
  }
  i++;
  dice[0] = atoi(temp.substr(0, i - 1).c_str());
  j = i;
  while(temp[j] != 'd')
  {
    j++;
  }
  j++;

  dice[1] = atoi(temp.substr(i, j - i - 1).c_str());
  dice[2] = atoi(temp.substr(j).c_str());
}

void print_monster_desc()
{
  int size = monster_descriptions.size();
  cout << "\n" << endl;
  for(int i = 0; i < size; i++)
  {
    cout << monster_descriptions[i].name << endl;
    cout << monster_descriptions[i].desc << endl;

    cout << monster_descriptions[i].symb << endl;

    cout << get_colors(i, 1) << endl;

    cout << monster_descriptions[i].speed.base << "+"
      << monster_descriptions[i].speed.dice << "d"
      << monster_descriptions[i].speed.sides << endl;

    cout << get_abilities(i) << endl;

    cout << monster_descriptions[i].hp.base << "+"
      << monster_descriptions[i].hp.dice << "d"
      << monster_descriptions[i].hp.sides << endl;

    cout << monster_descriptions[i].dam.base << "+"
      << monster_descriptions[i].dam.dice << "d"
      << monster_descriptions[i].dam.sides << endl;

    cout << monster_descriptions[i].rarity << endl;
    cout << "\n" << endl;
  }
}

void parse_items()
{
  string objFilePath = strcat(getenv("HOME"),"/.rlg327/object_desc.txt");
  ifstream objFile(objFilePath);
  // temp is current string being worked on
  string temp;
  // following are monster characteristics that are grabbed from the monster desc
  string name;
  string desc;
  string type;
  int color[8];
  int weight[3];
  int hit[3];
  int dam[3];
  int attr[3];
  int val[3];
  int dodge[3];
  int def[3];
  int speed[3];
  string art;
  int rarity;

  // counter is number of characteristics parsed currently
  int counter = 0;
  getline(objFile, temp);
  cout << temp << endl;

  if(temp == "RLG327 OBJECT DESCRIPTION 1")
  {
    while(objFile.peek() != EOF) //goes until end of file
    {
      getline(objFile, temp);
      if(temp == "BEGIN OBJECT")
      {
        // resets all attributes after each new desc 
        desc = "";
        counter = 0;
        for(int i = 0; i < 8; i++)
        {
          color[i] = -1;
        }

        while(temp != "END")
        {
          getline(objFile, temp);
          // gets the object name
          if(temp.length() > 4 && temp.substr(0,4) == "NAME")
          {
            counter++;
            name = temp.substr(5);
            //cout << name << endl;
          }
          // gets the item type
          if(temp.length() > 4 && temp.substr(0,4) == "TYPE") 
          {
            counter++;
            type = temp.substr(5);
            //cout << symb << endl;
          }
          // gets the object rarity
          if(temp.length() > 4 && temp.substr(0,4) == "RRTY") 
          {
            counter++;
            rarity = atoi(temp.substr(5).c_str());
            //cout << rarity << endl;
          }
          // uses parse_dice to get object weight
          if(temp.length() > 6 && temp.substr(0,6) == "WEIGHT")
          {
            counter++;
            parse_dice(temp.substr(7), weight);
            //cout << hp[0] << hp[1] << hp[2] << endl;
          }
          // uses parse_dice to get item damage
          if(temp.length() > 3 && temp.substr(0,3) == "DAM")
          {
            counter++;
            parse_dice(temp.substr(4), dam);
            //cout << temp.substr(0,3) << temp.substr(4) << endl;
          }
          // uses parse_dice to get item speed
          if(temp.length() > 5 && temp.substr(0,5) == "SPEED") 
          {
            counter++;
            parse_dice(temp.substr(6), speed);
            //cout << temp.substr(0,5) << temp.substr(6) << endl;
          }
          // parses through line word by word, and sets to equivalent integer
          if(temp.length() > 5 && temp.substr(0,5) == "COLOR")
          {
            counter++;
            int i = 6;
            int lasti = i;
            int colorIndex = 0;
            int length = temp.length();
            // gets characters until whitespace
            while (i < length) {
              while(temp[i] != ' ' && i < length){
                i++;
              }

              if(temp.substr(lasti, i - lasti) == "BLACK"){
                color[colorIndex++] = 0;
              }
              else if(temp.substr(lasti, i - lasti) == "RED"){
                color[colorIndex++] = 1;
              }
              else if(temp.substr(lasti, i - lasti) == "GREEN"){
                color[colorIndex++] = 2;
              }
              else if(temp.substr(lasti, i - lasti) == "YELLOW"){
                color[colorIndex++] = 3;
              }
              else if(temp.substr(lasti, i - lasti) == "BLUE"){
                color[colorIndex++] = 4;
              }
              else if(temp.substr(lasti, i - lasti) == "MAGENTA"){
                color[colorIndex++] = 5;
              }
              else if(temp.substr(lasti, i - lasti) == "CYAN"){
                color[colorIndex++] = 6;
              }
              else if(temp.substr(lasti, i - lasti) == "WHITE"){
                color[colorIndex++] = 7;
              }
              else {
                color[colorIndex++] = -1;
              }
              i++;
              lasti = i;
              //cout << temp.substr(6) << endl;
            }

          }
          // uses parse_dice to get item art
          if(temp.length() > 3 && temp.substr(0,3) == "HIT")
          {
            counter++;
            parse_dice(temp.substr(4), hit);
          }
          // uses parse_dice to get item attributes
          if(temp.length() > 4 && temp.substr(0,4) == "ATTR")
          {
            counter++;
            parse_dice(temp.substr(5), attr);
          }
          // uses parse_dice to get item dodge
          if(temp.length() > 5 && temp.substr(0,5) == "DODGE")
          {
            counter++;
            parse_dice(temp.substr(6), dodge);
          }
          // uses parse_dice to get item defense
          if(temp.length() > 3 && temp.substr(0,3) == "DEF")
          {
            counter++;
            parse_dice(temp.substr(4), def);
          }
          // uses parse_dice to get item art
          if(temp.length() > 3 && temp.substr(0,3) == "VAL")
          {
            counter++;
            parse_dice(temp.substr(4), val);
          }
          // gets the object artifact status
          if(temp.length() > 3 && temp.substr(0,3) == "ART")
          {
            counter++;
            art = temp.substr(4);
          }
          // reads the next lines(77 chars max) until it encounters a singular ".", appends into desc string
          if(temp == "DESC")
          {
            counter++;
            getline(objFile, temp);
            while (temp != ".") {
              desc += "\n" + temp.substr(0, 77);
              getline(objFile, temp);
            }
            desc = desc.substr(1);
            //cout << desc << endl;
          }
        }
        // if all characteristics are not set
        if (counter != 14) {
          cout << "Error: Invalid Object Description" << endl;
        } else {
          item_desc_t it;
          it.type = type;
          it.weight = make_dice(weight);
          it.hit = make_dice(hit);
          it.dam = make_dice(dam);
          it.def = make_dice(def);
          it.attr = make_dice(attr);
          it.val = make_dice(val);
          it.dodge = make_dice(dodge);
          it.speed = make_dice(speed);	  
          it.name = name;
          it.desc = desc;
          it.art = art;
          it.rarity = rarity;

          for(int i = 0; i < 8; i++)
          {
            it.color[i] = color[i];
          }


          item_descriptions.push_back(it);
          //cout << monster_descriptions.size() << endl;
        }
      }
    }
  }
  else
  {
    cout << "Error: Invalid File Format" << endl;
  }
}

void print_item_desc()
{
  int size = item_descriptions.size();
  cout << "\n" << endl;
  for(int i = 0; i < size; i++)
  {
    cout << item_descriptions[i].name << endl;
    cout << item_descriptions[i].desc << endl;

    cout << item_descriptions[i].type << endl;

    cout << get_colors(i, 0) << endl;

    cout << item_descriptions[i].speed.base << "+"
      << item_descriptions[i].speed.dice << "d"
      << item_descriptions[i].speed.sides << endl;

    cout << item_descriptions[i].weight.base << "+"
      << item_descriptions[i].weight.dice << "d"
      << item_descriptions[i].weight.sides << endl;

    cout << item_descriptions[i].attr.base << "+"
      << item_descriptions[i].attr.dice << "d"
      << item_descriptions[i].attr.sides << endl;

    cout << item_descriptions[i].val.base << "+"
      << item_descriptions[i].val.dice << "d"
      << item_descriptions[i].val.sides << endl;

    cout << item_descriptions[i].dodge.base << "+"
      << item_descriptions[i].dodge.dice << "d"
      << item_descriptions[i].dodge.sides << endl;

    cout << item_descriptions[i].def.base << "+"
      << item_descriptions[i].def.dice << "d"
      << item_descriptions[i].def.sides << endl;

    cout << item_descriptions[i].art << endl;

    cout << item_descriptions[i].hit.base << "+"
      << item_descriptions[i].hit.dice << "d"
      << item_descriptions[i].hit.sides << endl;

    cout << item_descriptions[i].dam.base << "+"
      << item_descriptions[i].dam.dice << "d"
      << item_descriptions[i].dam.sides << endl;

    cout << item_descriptions[i].rarity << endl;
    cout << "\n" << endl;
  }
}

dice_t make_dice(int temp[3])
{
  dice_t temp_dice;
  temp_dice.base = temp[0];
  temp_dice.dice = temp[1];
  temp_dice.sides = temp[2];
  return temp_dice;
}

std::string get_colors(int i, int toggle)
{
  string ret;
  if (toggle){
    for(int j = 0; j < 8; j++)
    {
      switch(monster_descriptions[i].color[j])
      {
        case 0:
          ret += "BLACK ";
          break;
        case 1:
          ret += "RED ";
          break;
        case 2:
          ret += "GREEN ";
          break;
        case 3:
          ret += "YELLOW ";
          break;
        case 4:
          ret += "BLUE ";
          break;
        case 5:
          ret += "MAGENTA ";
          break;
        case 6:
          ret += "CYAN ";
          break;
        case 7:
          ret += "WHITE ";
          break;
        default:
          break;
      }
    }
  }
  else {
    for(int j = 0; j < 8; j++)
    {
      switch(item_descriptions[i].color[j])
      {
        case 0:
          ret += "BLACK ";
          break;
        case 1:
          ret += "RED ";
          break;
        case 2:
          ret += "GREEN ";
          break;
        case 3:
          ret += "YELLOW ";
          break;
        case 4:
          ret += "BLUE ";
          break;
        case 5:
          ret += "MAGENTA ";
          break;
        case 6:
          ret += "CYAN ";
          break;
        case 7:
          ret += "WHITE ";
          break;
        default:
          break;
      }
    }
  }

  return ret;
}

std::string get_abilities(int i)
{
  string ret;
  //cout << monster_descriptions[i].ability << endl;
  if(monster_descriptions[i].ability & BIT_SMART){
    ret += "SMART ";
  } if(monster_descriptions[i].ability & BIT_TELE){
    ret += "TELE ";
  } if(monster_descriptions[i].ability & BIT_TUN){
    ret += "TUNNEL ";
  } if(monster_descriptions[i].ability & BIT_ERAT){
    ret += "ERRATIC ";
  } if(monster_descriptions[i].ability & BIT_PASS){
    ret += "PASS ";
  } if(monster_descriptions[i].ability & BIT_PICKUP){
    ret += "PICKUP ";
  } if(monster_descriptions[i].ability & BIT_DESTROY){
    ret += "DESTROY ";
  } if(monster_descriptions[i].ability & BIT_UNIQ){
    ret += "UNIQUE ";
  } if(monster_descriptions[i].ability & BIT_BOSS){
    ret += "BOSS ";
  }
  return ret;
}
