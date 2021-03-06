#include "dungeon.h"

//Globals
int dungeon_tunnel_map[DUNGEON_ROW][DUNGEON_COL]; // distance map for tunneling monsters
int dungeon_non_tunnel_map[DUNGEON_ROW][DUNGEON_COL]; // distance map for non-tunneling monsters
int dungeon_layout[DUNGEON_ROW][DUNGEON_COL];
int dungeon_display[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for outputting text 
uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for hardness level  hardness goes from 0 - 255 (0 meaning room or corridor, 255 meaning immutable)
room_t *rooms;
pc_t pc;
stair_t *upstairs;
stair_t *downstairs;
character_t *entities[DUNGEON_ROW][DUNGEON_COL];
heap_t entities_heap;
int num_entities;

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int load = 0, save = 0, num_mon = 10;//, fps = 4;
  int readErr = 0, writeErr = 0, num_rooms = 0, num_upstair = 0, num_downstair = 0;
  char player_next_move;

  //check for save or load switches
  if(argc > 1)
  {
    for(int i = 1; i < argc; i++) //should i just make this 2 if statements with 2 args?
    {
      if(!strcmp(argv[i], "--save") || !strcmp(argv[i], "-s"))  //strcmp returns 0 (which in an if statement means false) if matching
      {	
        save = 1;
      }
      if(!strcmp(argv[i], "--load") || !strcmp(argv[i], "-l"))
      {
        load = 1;
      }
      if(!strcmp(argv[i], "--nummon") || !strcmp(argv[i], "-m"))
      {
        num_mon = atoi(argv[++i]);
      }
      /*
         if(!strcmp(argv[i], "--fps") || !strcmp(argv[i], "-f"))
         {
         fps = atoi(argv[++i]);
         }
       */
    }
  }

  //load from file
  if (load == 1)
  {
    readErr = load_dungeon(&num_rooms, &num_upstair, &num_downstair); // load from file
    set_layout();
    create_entities(num_rooms, &num_mon); 
  }
  //create our own dungeon
  else
  {
    upstairs = malloc(sizeof(stair_t)); //If we know we are creating our own dungeon we can malloc the stairs with size 1
    downstairs = malloc(sizeof(stair_t));
    rooms = malloc(MIN_ROOMS * sizeof(room_t));
    num_upstair = 1;
    num_downstair = 1;
    num_entities = num_mon + 1;
    spawn_new_dungeon(num_rooms, num_mon);
  }
  //save to file
  if (save == 1)
  { 
    writeErr = save_dungeon(&num_rooms, &num_upstair, &num_downstair); // save dungeon to file
  }
  if(writeErr == -1 || readErr == -1) //error handling
  {
    return -1;
  }

  // print dungeon_display
  print_dungeon();
  // printf("\n");
  // making dist maps and printing them
  // non tunneling
  generate_nonTunnel_dist_map(dungeon_hardness, dungeon_non_tunnel_map, pc.x_pos, pc.y_pos);
  // print_dist_map(dungeon_non_tunnel_map);
  // printf("\n");
  // tunneling
  generate_tunnel_dist_map(dungeon_hardness, dungeon_tunnel_map, pc.x_pos, pc.y_pos);
  // print_dist_map(dungeon_tunnel_map);

  int pc_state = 0;
  //entities_heap = generate_entities_heap(num_mon, entities);
  while(1)
  {
    /**
      moves next entity from queue from heap (besed on lowest turn), returns pc state
      (0 == (monster turn) alive, -1 == player died, 1 == player turn (alive)) 
     **/
    // prints the tunneling map for debugging
//       for (int r = 0; r < DUNGEON_ROW; r++) {
 //      for (int c = 0; c < DUNGEON_COL; c++) {
  //     printf("%2d", dungeon_tunnel_map[r][c]);
   //    }
    //   printf("\n");
     //  }
      // printf("\n\n");
     
    generate_tunnel_dist_map(dungeon_hardness, dungeon_tunnel_map, pc.x_pos, pc.y_pos);
    generate_nonTunnel_dist_map(dungeon_hardness, dungeon_non_tunnel_map, pc.x_pos, pc.y_pos);
    pc_state = next_turn(dungeon_layout, dungeon_display,
        dungeon_hardness, entities, dungeon_tunnel_map,
        dungeon_non_tunnel_map, &entities_heap, (num_mon + 1));

    if(pc_state > 0)
    {
      print_dungeon();
      player_next_move = getch();
      move_player(player_next_move);
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

// moves the player
int move_player(char direction) {
  int x_direction = 0;
  int y_direction = 0;
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
      y_direction = 1;
      break;
    case '4':// left
    case 'h':
      x_direction = -1;
      break;
    case '5':// rest
    case ' ':
    case '.':
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
      printf("Error: wrong char passed into move_player: %c", direction);
      return -1;
      break;
  }
  //checks if movement is valid
  int target_r = pc.y_pos + y_direction;
  int target_c = pc.x_pos + x_direction;
  int target_tile = dungeon_display[target_r][target_c];
  if (target_tile == TILE_ROCK) {
    printf("player movement invalid: %c\n", direction);
    return -1; 
  }
  //kills the monster if there is a monster in target tile
  if (target_tile >= 10) {
    if(entities[target_r][target_c] != NULL)
    {
      character_t *temp[num_entities - 1];
      //pulls out whole heap and checks for murdered entity, if so set is_alive = 0
      for(int i = 0; i < num_entities - 1; i++)
      {
        temp[i] = heap_remove_min(&entities_heap);	  

        if(temp[i]->x_pos == target_c && temp[i]->y_pos == target_r)
        {
          temp[i]->is_alive = 0;
        }
      } //reinserts temp into heap
      for(int i = 0; i < num_entities - 1; i++)
      {
        heap_insert(&entities_heap, temp[i]);
      }
    }
  }
  entities[target_r][target_c] = entities[pc.y_pos][pc.x_pos];
  entities[pc.y_pos][pc.x_pos] = NULL;
  dungeon_display[target_r][target_c] = dungeon_display[pc.y_pos][pc.x_pos];
  dungeon_display[pc.y_pos][pc.x_pos] = dungeon_layout[pc.y_pos][pc.x_pos];
  pc.y_pos = target_r;
  pc.x_pos = target_c;
  return 0;
}

// makes a new dungeon (used for going upstairs and downstairs)
void spawn_new_dungeon(int num_rooms, int num_mon) {
  set_dungeon();
  create_rooms(&num_rooms);
  create_stairs();
  create_paths(&num_rooms);
  set_layout();
  create_player();
  create_entities(num_rooms, &num_mon);
  set_hardness();
  entities_heap = generate_entities_heap(num_mon, entities);
}

void create_entities(int num_rooms, int *num_monsters) {
  int seq = 0;
  //intializing player and placing into entities
  character_t *player = malloc(sizeof(character_t));
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
          npc_t *npc = malloc(sizeof(npc_t));
          npc->x_pos = x;
          npc->y_pos = y;
          npc->characteristics = mon_type;
          npc->type = get_monster_type(mon_type);
          character_t *monster = malloc(sizeof(character_t));
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
void print_dungeon() {
  //initializing the screen and turning on keyboard input
  initscr();
  char currentchar;
  for (int r = 0; r < DUNGEON_ROW; r++) {
    for (int c = 0; c < DUNGEON_COL; c++) {
      switch(dungeon_display[r][c])
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
          currentchar = get_monster_type(dungeon_display[r][c] - 10);
          break;

        default:
          currentchar = '!';
          break;
      }
      mvwaddch(stdscr, r, c, currentchar);
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

int save_dungeon(int *num_rooms, int *num_upstair, int *num_downstair)
{	
  FILE *f;
  char fileMarker[] = "RLG327-S2021";
  uint32_t fileVersion = htobe32(0);
  uint32_t fileSize = htobe32(1708 + ((*num_downstair) * 2) + ((*num_upstair) * 2) + ((*num_rooms) * 4));

  char *home = getenv("HOME");
  char *game_dir = ".rlg327";
  char *save_file = "dungeon";
  char *path = malloc((strlen(home) + strlen(game_dir) + strlen(save_file) + 2 + 1) * sizeof(char));
  sprintf(path, "%s/%s/%s", home, game_dir, save_file);

  if(!(f = fopen(path,"w")))
  {
    fprintf(stderr, "Failed to open file for writing\n");
    return -1;
  }

  fwrite(fileMarker, sizeof(char), 12, f); //file-type marker
  fwrite(&fileVersion, sizeof(uint32_t), 1, f); //32 bit uint for file version
  fwrite(&fileSize, sizeof(uint32_t), 1, f); //size of file in bytes (num bytes)
  fwrite(&pc.x_pos, sizeof(uint8_t), 1, f); //write player y pos
  fwrite(&pc.y_pos, sizeof(uint8_t), 1, f); //write player x pos
  fwrite(dungeon_hardness, sizeof(dungeon_hardness), 1, f); //writes dungeon hardness array
  uint16_t nRoom = htobe16(*num_rooms);
  fwrite(&nRoom, sizeof(nRoom), 1, f); //writes number of rooms

  for (int i = 0; i < *num_rooms; i++) //writes all rooms x,y coord and dimensions
  {
    fwrite(&rooms[i].x_pos, sizeof(uint8_t), 1, f);
    fwrite(&rooms[i].y_pos, sizeof(uint8_t), 1, f);
    fwrite(&rooms[i].x_width, sizeof(uint8_t), 1, f);
    fwrite(&rooms[i].y_height, sizeof(uint8_t), 1, f);
  }

  uint16_t nUp = htobe16(*num_upstair);
  fwrite(&nUp, sizeof(uint16_t), 1, f);	//writes number of upstairs along with x,y pos
  for (int i = 0; i < *num_upstair; i++)
  {
    uint8_t x_pos = upstairs[i].x_pos;
    uint8_t y_pos = upstairs[i].y_pos;
    fwrite(&x_pos, sizeof(uint8_t), 1, f);
    fwrite(&y_pos, sizeof(uint8_t), 1, f);
  }

  uint16_t nDown = htobe16(*num_downstair);
  fwrite(&nDown, sizeof(uint16_t), 1, f); //writes number of downstairs along with x,y pos
  for (int i = 0; i < *num_downstair; i++)
  {
    uint8_t x_pos = downstairs[i].x_pos;
    uint8_t y_pos = downstairs[i].y_pos;
    fwrite(&x_pos, sizeof(uint8_t), 1, f);
    fwrite(&y_pos, sizeof(uint8_t), 1, f);
  }

  fclose(f);
  return 0;
}

int load_dungeon(int *num_rooms, int *num_upstair, int *num_downstair)
{
  FILE *f;
  char fileMarker[12];
  uint32_t version, size;
  uint16_t num_up, num_down, nRooms;

  char *home = getenv("HOME");
  char *game_dir = ".rlg327";
  char *save_file = "dungeon";
  char *path = malloc((strlen(home) + strlen(game_dir) + strlen(save_file) + 2 + 1) * sizeof(char));
  sprintf(path, "%s/%s/%s", home, game_dir, save_file);

  if(!(f = fopen(path,"r")))
  {
    fprintf(stderr, "Failed to open file for reading\n");
    return -1;
  }


  fread(fileMarker, sizeof(char), 12, f); //gets file-type marker
  fread(&version, sizeof(uint32_t), 1, f); //gets file version
  version = be32toh(version);
  fread(&size, sizeof(uint32_t), 1, f); //gets size of file in bytes (num bytes)
  size = be32toh(size);
  fread(&pc.x_pos, sizeof(uint8_t), 1, f); //gets player y pos
  fread(&pc.y_pos, sizeof(uint8_t), 1, f); //gets player x pos
  fread(dungeon_hardness, sizeof(dungeon_hardness), 1, f); //gets dungeon hardness array
  fread(&nRooms, sizeof(uint16_t), 1, f); //gets number of rooms
  *num_rooms = be16toh(nRooms);

  rooms = malloc((*num_rooms) * sizeof(room_t)); //allocate room size
  for (int i = 0; i < *num_rooms; i++) //gets rooms x,y pos and dims
  {
    fread(&rooms[i].x_pos, sizeof(uint8_t), 1, f);
    fread(&rooms[i].y_pos, sizeof(uint8_t), 1, f);
    fread(&rooms[i].x_width, sizeof(uint8_t), 1, f);
    fread(&rooms[i].y_height, sizeof(uint8_t), 1, f);
  }

  fread(&num_up, sizeof(uint16_t), 1, f); //gets number of upstairs
  *num_upstair = be16toh(num_up);
  upstairs = malloc((*num_upstair) * sizeof(stair_t));
  for (int i = 0; i < *num_upstair; i++)  //gets x,y pos for upstairs
  {
    fread(&upstairs[i].x_pos, sizeof(uint8_t), 1, f);
    fread(&upstairs[i].y_pos, sizeof(uint8_t), 1, f);
    upstairs[i].direction = 1;
  }


  fread(&num_down, sizeof(uint16_t), 1, f);  //gets number of downstairs
  *num_downstair = be16toh(num_down);
  downstairs = malloc((*num_downstair) * sizeof(stair_t));
  for (int i = 0; i < *num_downstair; i++)  //gets x,y pos for downstairs
  {
    fread(&downstairs[i].x_pos, sizeof(uint8_t), 1, f);
    fread(&downstairs[i].y_pos, sizeof(uint8_t), 1, f);
    downstairs[i].direction = 0;
  }

  fclose(f);

  //setting loaded dungeon to be printed

  for(int r = 0; r < DUNGEON_ROW; r++) //Place corridors (any extra will be overwritten)
  {
    for(int c = 0; c < DUNGEON_COL; c++)
    {
      if(dungeon_hardness[r][c] == 0)
      {
        dungeon_display[r][c] = 2;
      }
    }
  }

  for(int i = 0; i < *num_rooms; i++) //Place rooms
  {
    int x = rooms[i].x_pos;
    int y = rooms[i].y_pos;
    int x_dim = rooms[i].x_width;
    int y_dim = rooms[i].y_height;   
    for(int r = y; r < y + y_dim; r++)
    {
      for(int c = x; c < x + x_dim; c++)
      {
        dungeon_display[r][c] = 1;
      }
    }
  }

  dungeon_display[pc.y_pos][pc.x_pos] = 5; //Place player
  for(int i = 0; i < *num_downstair; i++) //Place downstairs
  {
    dungeon_display[downstairs[i].y_pos][downstairs[i].x_pos] = 3;
  }

  for(int i = 0; i < *num_upstair; i++) //Place upstairs
  {
    dungeon_display[upstairs[i].y_pos][upstairs[i].x_pos] = 4;
  }

  return 0;
}
