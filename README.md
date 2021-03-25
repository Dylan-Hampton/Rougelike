Carver Bartz  cjbartz@iastate.edu
Dylan Hampton dhampton@iastate.edu
Ben Nguyen    btnguyen@iastate.edu

This program generates a basic dungeon model according to the assignment 1.06 specifications.
Our implementation for fog of war is another dungeon copy that only gets updated in a 3x3 area around the player when the player moves, it remembers previously seen tiles / monsters. Our implementation of teleportation creates a star on top of the player, and teleports the player to wherever the star is, and if it teleports onto a monster, the player dies instantly. After pressing g, then r, the player will teleport to a random location. We changed all the structs to classes, and we also converted all c files to cpp files (except for heap.c)
