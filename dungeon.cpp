#include "dungeon.h"

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

  //print_dungeon(0);
  generate_nonTunnel_dist_map(dungeon_hardness, dungeon_non_tunnel_map, pc.x_pos, pc.y_pos);
  generate_tunnel_dist_map(dungeon_hardness, dungeon_tunnel_map, pc.x_pos, pc.y_pos);

  int pc_state = 1;
  num_ent = num_mon + 1;
  int pc_other_action = 0; // 1 = killed monster -1 = hit a wall 0 = default
  int is_in_mon_list = 0, was_mon_list = 0;
  int scroll = 0;
  int fow_toggle = 0;
  update_fow();

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
        /*
           } else {
        //Teleporting
        player_next_move = getch();
        if(player_next_move == 'g'){
        is_teleporting = 0;
        }
        else if(player_next_move == 'r'){
        //Random position
        int y_rand_teleport = rand() % 21;
        int x_rand_teleport = rand() % 80;
        entities[y_rand_teleport][x_rand_teleport] = entities[pc.y_pos][pc.x_pos];
        entities[pc.y_pos][pc.x_pos] = NULL;
        dungeon_display[y_rand_teleport][x_rand_teleport] = dungeon_display[pc.y_pos][pc.x_pos];
        dungeon_display[pc.y_pos][pc.x_pos] = dungeon_layout[pc.y_pos][pc.x_pos];
        pc.x_pos = x_rand_teleport;
        pc.y_pos = y_rand_teleport;
        is_teleporting = 0;
        }
        }

         */
    }    
  } 

  else if(pc_state > 0 && alive_ent == 1)
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
    dungeon_display[star_row][star_col] = dungeon_layout[star_row][star_col];
    dungeon_fow[star_row][star_col] = dungeon_layout[star_row][star_col];
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
  for (int r = -3; r <= 3; r++) {
    for (int c = -3; c <= 3; c++) {
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
        int mon_type = (rand() % 16);
        int x = rooms[room].x_pos + (rand() % rooms[room].x_width);
        int y = rooms[room].y_pos + (rand() % rooms[room].y_height);
        if (dungeon_display[y][x] == TILE_FLOOR && *num_monsters > 0)
        {
          //initializing monster
          dungeon_display[y][x] = 10 + mon_type;
          npc_t *npc = (npc_t *) malloc(sizeof(npc_t));
          npc->x_pos = x;
          npc->y_pos = y;
          npc->characteristics = mon_type;
          npc->type = get_monster_type(mon_type);
          character_t *monster = (character_t *) malloc(sizeof(character_t));
          monster->sn = seq++;
          monster->x_pos = x;
          monster->y_pos = y;
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
          monster->turn = 0;
          monster->is_pc = 0;
          monster->npc = npc;
          monster->is_alive = 1;
          entities[y][x] = monster;
          spawned_mon++;
          (*num_monsters)--;
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
