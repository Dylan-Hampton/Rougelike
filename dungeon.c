#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

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
int dungeon_hardness[DUNGEON_ROW][DUNGEON_COL]; //dungeon map for hardness level  hardness goes from 0 - 255 (0 meaning room or corridor, 255 meaning immutable)
room_t rooms[MAX_ROOMS];
pc_t pc;
stair_t *upstairs;
stair_t *downstairs;

int main(int argc, char *argv[]) {

  srand(time(NULL));
  int load, save = 0;
  int readErr,writeErr = 0;
	int *num_rooms;
  //check for save or load switches
  if(argc > 1)
  {
    for(int i = 1; i < argc; i++) //should i just make this 2 if statements with 2 args?
    {
      if(!strcmp(argv[i], "--save"))  //strcmp returns 0 (which in an if statement means false) if matching
      {	
	save = 1;
      }
      if(!strcmp(argv[i], "--load"))
      {
	load = 1;
      }
    }
  }
  
  set_dungeon();
  
  if (save == 1)
  {
    writeErr = save_dungeon(&num_rooms); // save dungeon to file
  }
  if (load == 1)
  {
    readErr = load_dungeon(&num_rooms); // load from file
  }
  else
  {
    create_rooms(&num_rooms);
    create_stairs();
    create_player();
    create_paths(&num_rooms);
  }
	if(writeErr == -1 || readErr == -1) //error handling
  {
    return -1;
  }	
	print_dungeon();
  
  return 0;
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
		}
      }
    }
  int lowerstair = 1;
  //adds lower staircase to lowest room corner
  for (int y =  DUNGEON_ROW; y >= 0; y--)
  {
    for (int x = DUNGEON_COL; x >= 0; x--)
    {
      if (dungeon_display[y][x] == 1 && lowerstair)
      {
	dungeon_display[y - 1][x - 1] = 4;
	dungeon_hardness[y + 1][x + 1] = 0;
	lowerstair = 0;
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
    int x = rooms[0].x_pos + (rooms[0].x_width / 2), y = rooms[0].y_pos + (rooms[0].y_height / 2);;

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
//A 3 == upstairs('>')
//A 4 == downstairs('<')
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

int save_dungeon(int *num_rooms)
{	
  FILE *f;
  char fileMarker[] = "RLG327-S2021";
	uint16_t num_upstair = sizeof(upstairs)/sizeof(upstairs[0]);
	uint16_t num_downstair = sizeof(downstairs)/sizeof(downstairs[0]);
  uint32_t fileVersion = 0, fileSize = 1708 + (num_downstair * 2) + (num_upstair * 2) + (*num_rooms * 4);
  uint16_t uNumRooms = *num_rooms;
  if(!(f = fopen(strcat(getenv("HOME"),"/.rlg327/dungeon/RLG327-S2021"),"w")))
  {
    fprintf(stderr, "Failed to open file for writing\n");
    return -1;
  }

  fwrite(fileMarker, sizeof(char), 12, f); //file-type marker
  fwrite(&fileVersion, sizeof(uint32_t), 1, f); //32 bit uint for file version
  fwrite(&fileSize, sizeof(uint32_t), 1, f); //size of file in bytes (num bytes)
  fwrite(&pc.x_pos, sizeof(uint8_t), 1, f); //write player y pos
  fwrite(&pc.y_pos, sizeof(uint8_t), 1, f); //write player x pos
  fwrite(dungeon_hardness, sizeof(int), DUNGEON_ROW * DUNGEON_COL, f); //writes dungeon hardness array
  fwrite(&uNumRooms, sizeof(uint16_t), 1, f); //writes number of rooms
	for (int i = 0; i < uNumRooms; i++) {
		uint8_t x_pos = rooms[i].x_pos; 
		uint8_t y_pos = rooms[i].y_pos;
		uint8_t x_width = rooms[i].x_width;
		uint8_t y_height = rooms[i].y_height;
		fwrite(&x_pos,sizeof(uint8_t),1,f);
		fwrite(&y_pos,sizeof(uint8_t),1,f);
		fwrite(&x_width,sizeof(uint8_t),1,f);
		fwrite(&y_height,sizeof(uint8_t),1,f);
	}
  fwrite(&num_upstair, sizeof(uint16_t), 1, f);	
	upstairs = malloc(num_upstair * sizeof(stair_t));
	for (int i = 0; i < num_upstair; i++) {
		uint8_t x_pos = upstairs[i].x_pos;
		uint8_t y_pos = upstairs[i].y_pos;
		fwrite(&x_pos, sizeof(uint8_t), 1, f);
		fwrite(&y_pos, sizeof(uint8_t), 1, f);	
	}
	fwrite(&num_downstair, sizeof(uint16_t), 1, f);
  downstairs = malloc(num_downstair * sizeof(stair_t));
	for (int i = 0; i < num_downstair; i++) {
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
  uint32_t fileVersion, fileSize;
  uint16_t uNumRooms;
  if(!(f = fopen(strcat(getenv("HOME"),"/.rlg327/dungeon/RLG327-S2021"),"r")))
  {
    fprintf(stderr, "Failed to open file for reading\n");
    return -1;
  }

  fread(fileMarker, sizeof(char), 12, f); //file-type marker
  fread(&fileVersion, sizeof(uint32_t), 1, f); //file version
  fread(&fileSize, sizeof(uint32_t), 1, f); //size of file in bytes (num bytes)
  fread(&pc.x_pos, sizeof(uint8_t), 1, f); //read player y pos
  fread(&pc.y_pos, sizeof(uint8_t), 1, f); //read player x pos
  fread(dungeon_hardness, sizeof(int), DUNGEON_ROW * DUNGEON_COL, f); //read dungeon hardness array
  fread(&uNumRooms, sizeof(uint16_t), 1, f); //read number of rooms
	for (int i = 0; i < uNumRooms; i++) {
		fread(&rooms[i].x_pos,sizeof(uint8_t),1,f);
		fread(&rooms[i].y_pos,sizeof(uint8_t),1,f);
		fread(&rooms[i].x_width,sizeof(uint8_t),1,f);
		fread(&rooms[i].y_height,sizeof(uint8_t),1,f);
	}
	fread(num_upstair, sizeof(uint16_t), 1, f);	
	upstairs = malloc(*num_upstair * sizeof(stair_t));
	for (int i = 0; i < *num_upstair; i++) {
		fread(&upstairs[i].x_pos, sizeof(uint8_t), 1, f);
		fread(&upstairs[i].y_pos, sizeof(uint8_t), 1, f);	
		upstairs[i].direction = 1;
	}
	fread(num_downstair, sizeof(uint16_t), 1, f);
	downstairs = malloc(*num_downstair * sizeof(stair_t));
	for (int i = 0; i < *num_downstair; i++) {
		fread(&downstairs[i].x_pos, sizeof(uint8_t), 1, f);
		fread(&downstairs[i].y_pos, sizeof(uint8_t), 1, f);	
		downstairs[i].direction = 0;
	}

  printf("%s \n", fileMarker);
  printf("%d \n", fileVersion);
  printf("%d \n", fileSize);
  printf("%d \n", pc.y_pos);
  printf("%d \n", pc.x_pos);
	printf("===============\n");
	for (int r = 0; r < DUNGEON_ROW; 	r++) {
		for (int c = 0; c < DUNGEON_COL; c++) {
			printf("%d ", dungeon_hardness[r][c]);
		}
		printf("\n");
	}
  printf("===============\n");
	for (int i = 0; i < *num_upstair; i++) {
  	printf("%d %d \n", upstairs[i].x_pos, upstairs[i].y_pos);
	}
	for (int i = 0; i < *num_downstair; i++) {
  	printf("%d %d \n", downstairs[i].x_pos, downstairs[i].y_pos);
	}
  
  fclose(f);
  return 0;
}

