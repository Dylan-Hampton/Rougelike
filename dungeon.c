#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_dungeon(); //Prints out the dungeon
void set_dungeon(); //Initializes all cells to rock(space)
void create_rooms(); //Creates 6 random rooms of random but minimum size

//Globals -I'm pretty sure we are allowed to use?

int dungeon[21][80];//Used ints to make defining hardness easier later


int main(int argc, char *argv[]) {

  srand(time(NULL));
  

  set_dungeon();
  create_rooms();
  print_dungeon();

  return 0;
}

void create_rooms(){
  // int totalRooms = (rand() % 3) + 6;
  int roomcount = 0;
  int attempts = 0;

 while(roomcount != 6 && attempts < 2000)
 {
   int x_coord = rand() % 80; //0-79
   int y_coord = rand() % 21; //0-20
   int x_dim = (rand() % 4) + 4; //4-7 can change if want
   int y_dim = (rand() % 4) + 3; //3-6 can change if want
   int  placement_successful = 1; //Can't get boolean to work? But works like bool

   for(int r = -1; r <= 1; r++)
   {
     for(int c = -1; c <= 1; c++)
     {
       int y = y_coord+r;
       int x = x_coord+c;

       //Used to check if out of bounds
       if((y > 20 || y < 0) || (x > 79 || x < 0) || y+y_dim > 20 || x+x_dim > 80)
       {
	 placement_successful = 0;
       }

       //Checks each corner for overlap
       else if(dungeon[y][x] != 0 || dungeon[y + y_dim][x] != 0 ||
	  dungeon[y][x+x_dim] != 0 || dungeon[y+y_dim][x+x_dim] != 0)
       {
	 placement_successful = 0;
       }

       //Checks if two rooms are touching
       else if (dungeon[y-1][x] != 0 || dungeon[y][x-1] != 0 ||
		dungeon[y-1][x+x_dim] != 0 || dungeon[y][(x+x_dim)+1] != 0 ||
		dungeon[y+y_dim+1][x] != 0 ||dungeon[y+y_dim][x-1] != 0 ||
		dungeon[y+y_dim+1][x+x_dim] != 0 ||dungeon[y+y_dim][(x+x_dim)+1] != 0)
	 {
	   placement_successful = 0;
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
     
       roomcount++;
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
  for(int i = 0; i < 21; i++)
  {
    for(int j = 0; j < 80; j++)
    {
      dungeon[i][j] = 0;
    }
  }
}

//May have to change these values for "hardness" in future assignments
//A 0 == rock(space)
//A 1 == room floor('.')
//A 2 == corridor('#')
void print_dungeon(){

  for(int r = 0; r < 21; r++)
  {
    for(int c = 0; c < 80; c++)
    {
      if(dungeon[r][c] == 0)
      {
	printf(" ");
      }
      else if(dungeon[r][c] == 1)
      {
	printf(".");
      }
      else
      {
	printf("#");
      }
    }
    printf("\n");
  }
 
}
