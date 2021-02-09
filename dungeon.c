#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//Constants
#define DUNGEON_ROW 21
#define DUNGEON_COL 80
#define numRooms    6

void print_dungeon(); //Prints out the dungeon
void set_dungeon(); //Initializes all cells to rock(space)
void create_rooms(); //Creates 6 random rooms of random but minimum size
void create_paths(); //Creates paths between each room
void create_stairs(); //Creates one stair that goes up and one that goes down

//Globals -I'm pretty sure we are allowed to use?

int dungeon[DUNGEON_ROW][DUNGEON_COL];//Used ints to make defining hardness easier later


int main(int argc, char *argv[]) {

  srand(time(NULL));
  int room_x_coord[6];
  int room_y_coord[6];
  

  set_dungeon();
  create_rooms(&room_x_coord, &room_y_coord);
  create_stairs();
  create_paths(&room_x_coord, &room_y_coord);
  print_dungeon();

  return 0;
}

void create_stairs() {
 int upperstair = 1;
 for (int y = 0; y < DUNGEON_ROW; y++)    
        {
                for (int x = 0; x < DUNGEON_COL; x++) 
                {
                 if (dungeon[y][x] == 1 && upperstair)
   {
    dungeon[y + 1][x + 1] = 3;
    upperstair = 0;
   }
                }
        }
 int lowerstair = 1;
        for (int y =  DUNGEON_ROW; y >= 0; y--)
        {
                for (int x = DUNGEON_COL; x >= 0; x--)
                {
                        if (dungeon[y][x] == 1 && lowerstair)
                        {
                                dungeon[y - 1][x - 1] = 4;
                                lowerstair = 0;
                        }
                }
        }

}

// creates the paths between rooms
void create_paths(int room_x_coord[numRooms], int room_y_coord[numRooms]){
  //find closest room based on center euclidean distance ie sqrt((x2-x1)^2+(y2-y1)^2)
  for(int i = 1; i < numRooms; i++) {
	int y = room_y_coord[0], x = room_x_coord[0];
	  while(y != room_y_coord[i]) {
		if (room_y_coord[i] > y) {
			y++;
		}
		else {
			y--;
		}
		if (dungeon[y][x] == 0) {
			dungeon[y][x] = 2;
		}

	  }
	  while(x != room_x_coord[i]) {
                if (room_x_coord[i] > x) {
                        x++;
                }
                else {
                        x--;
                }
                if (dungeon[y][x] == 0) {
                        dungeon[y][x] = 2;
                }

          }

  }
}

void create_rooms(int room_x_coord[6], int room_y_coord[6]){
  // int totalRooms = (rand() % 3) + 6;
  int roomCount = 0;
  int attempts = 0;

 while(roomCount != numRooms && attempts < 2000)
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
       else if(dungeon[y][x] != 0 || dungeon[y + y_dim][x] != 0 ||
   dungeon[y][x+x_dim] != 0 || dungeon[y+y_dim][x+x_dim] != 0)
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
        if(dungeon[i][x_coord-1] != 0 || dungeon[y_coord-1][j] != 0 ||
    dungeon[(y_coord+y_dim)+1][j] != 0 || dungeon[i][(x_coord + x_dim)+1] != 0)
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
  dungeon[r][c] = 1;
       }
     }

     room_y_coord[roomCount] = y_coord + (y_dim / 2);
     room_x_coord[roomCount] = x_coord + (x_dim / 2);
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
  //Change to spaces
  for(int i = 0; i < DUNGEON_ROW; i++)
  {
    for(int j = 0; j < DUNGEON_COL; j++)
    {
      dungeon[i][j] = 0;
    }
  }
}

//May have to change these values for "hardness" in future assignments
//A 0 == rock(space)
//A 1 == room floor('.')
//A 2 == corridor('#')
//A 3 == upstairs('>')
//A 4 == downstairs('<')
void print_dungeon(){

  for(int r = 0; r < DUNGEON_ROW; r++)
  {
    for(int c = 0; c < DUNGEON_COL; c++)
    {

      switch(dungeon[r][c])
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

 //E for error
        default:
   printf("E");
   break;
      }
      
    }
    printf("\n");
  }
 
}

