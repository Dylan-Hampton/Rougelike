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

//Globals -I'm pretty sure we are allowed to use?
int dungeon_display[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for outputting text 
uint8_t dungeon_hardness[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for hardness level  hardness goes from 0 - 255 (0 meaning room or corridor, 255 meaning immutable)
room_t *rooms;
pc_t pc;
stair_t *upstairs;
stair_t *downstairs;

int main(int argc, char *argv[]) {

  srand(time(NULL));
  int load = 0, save = 0;
  int readErr = 0, writeErr = 0, num_rooms = 0, num_upstair = 0, num_downstair = 0;
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
    }
  }
  
  if (load == 1)
  {
    readErr = load_dungeon(&num_rooms, &num_upstair, &num_downstair); // load from file
  }
  
  else
  {
    set_dungeon();
    upstairs = malloc(sizeof(stair_t)); //If we know we are creating our own dungeon we can malloc the stairs with size 1
    downstairs = malloc(sizeof(stair_t));
    rooms = malloc(MIN_ROOMS * sizeof(room_t));
    num_upstair = 1;
    num_downstair = 1;
   
    create_rooms(&num_rooms);
    create_stairs();
    create_player();
    create_paths(&num_rooms);
    set_hardness();
  }
  
  if (save == 1)
  { 
    writeErr = save_dungeon(&num_rooms, &num_upstair, &num_downstair); // save dungeon to file
  }
  if(writeErr == -1 || readErr == -1) //error handling
  {
    return -1;
  }	
  print_dungeon();
  
  return 0;
}



void set_hardness() { //sets any non-rock to zero hardness
  for (int r = 0; r < DUNGEON_ROW; r++)
  {
    for (int c = 0; c < DUNGEON_COL; c++)
    {
      if (dungeon_display[r][c] != 0) {
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
      if (dungeon_display[y][x] == 1 && player)
      {
	dungeon_display[y][x] = 5;
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
	if (dungeon_display[y][x] == 1 && upperstair)
	{
	  dungeon_display[y + 1][x + 1] = 3;
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
      if (dungeon_display[y][x] == 1 && lowerstair)
      {
	dungeon_display[y - 1][x - 1] = 4;
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
        if (dungeon_display[y][x] == 0 && dungeon_display[y][x+1] != 2 && dungeon_display[y][x-1] != 2)
	{
	  dungeon_display[y][x] = 2;
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
      if (dungeon_display[y][x] == 0 && dungeon_display[y+1][x] != 2 && dungeon_display[y-1][x] != 2)
      {
	dungeon_display[y][x] = 2;
	dungeon_hardness[y][x] = 0;
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
       else if(dungeon_display[y][x] != 0 || dungeon_display[y + y_dim][x] != 0 ||
	       dungeon_display[y][x+x_dim] != 0 || dungeon_display[y+y_dim][x+x_dim] != 0)
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
		 if(dungeon_display[i][x_coord-1] != 0 || dungeon_display[y_coord-1][j] != 0 ||
		    dungeon_display[(y_coord+y_dim)+1][j] != 0 || dungeon_display[i][(x_coord + x_dim)+1] != 0)
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
	 dungeon_display[r][c] = 1;
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
      dungeon_display[y][x] = 0;
      dungeon_hardness[y][x] = (rand() % 254) + 1; //1-254
    }
  }
}

//A 0 == rock(space)
//A 1 == room floor('.')
//A 2 == corridor('#')
//A 3 == downstairs('>')
//A 4 == upstairs('<')
//A 5 == player character('@')
void print_dungeon(){

  for(int r = 0; r < DUNGEON_ROW; r++)
  {
    for(int c = 0; c < DUNGEON_COL; c++)
    {

      switch(dungeon_display[r][c])
      {
        case 0:
	  printf(" ");
	  break;

        case 1:
	  printf(".");
	  break;

        case 2:
	  printf("#");
	  break;

        case 3:
	  printf(">");
	  break;

        case 4:
	  printf("<");
	  break;
	  
        case 5:
	  printf("@");
	  break;

	  //E for error
        default:
	  printf("E");
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

/*
  printf("Marker: %s \n", fileMarker);
  printf("Version: %d \n", fileVersion);
  printf("File Size: %d \n", fileSize);
  printf("PC x,y: %d,%d\n", pc.x_pos, pc.y_pos);
  printf("===============\n");

  for (int r = 0; r < DUNGEON_ROW; r++)
  {
    for (int c = 0; c < DUNGEON_COL; c++)
    {
      printf("%3d ", dungeon_hardness[r][c]);
    }
    printf("\n");
  }
  printf("===============\n");

  printf("%d \n", uNumRooms);
  for(int i = 0; i < uNumRooms; i++)
  {
    printf("pos x,y: %d,%d ,  dims x,y: %d,%d \n", rooms[i].x_pos, rooms[i].y_pos, rooms[i].x_width, rooms[i].y_height);
  }
  
  printf("Number of upstairs: %d \n", num_up);
  for (int i = 0; i < *num_upstair; i++)
  {
    printf("Upstairs %d: %d,%d \n", i, upstairs[i].x_pos, upstairs[i].y_pos);
    }
  printf("Number of downstairs: %d \n", num_down);
  for (int i = 0; i < *num_downstair; i++)
  {
    printf("Downstairs %d: %d,%d \n", i, downstairs[i].x_pos, downstairs[i].y_pos);
  } */

