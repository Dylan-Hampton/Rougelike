Carver Bartz  cjbartz@iastate.edu
Dylan Hampton dhampton@iastate.edu
Ben Nguyen    btnguyen@iastate.edu

This program generates a basic dungeon model according to the assignment 1.07 specifications. Our program reads the file at the specified
file path and then prints out the description of the monster's it reads in. This is done by are parse_monsters method, which finds the
lines that hold the indicators (i.e., "DESC", "DAM", "SPEED", etc) and then saves the necessary information following it. We also have a
parse_dice method that allows us to read in the appropriate information from the dice and save it for the npc. Finally, we make sure that
the monster in the file is a legitimate monster that can be added into our game, by checking if the monster has all the characteristics
necessary to be a valid monster. Once the monster's have been read in are defined by the specifications from the file, we use our
print_monster_desc method to loop through each monster and print their descriptions and characteristics.

