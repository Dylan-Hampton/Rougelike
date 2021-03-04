Carver Bartz  cjbartz@iastate.edu
Dylan Hampton dhampton@iastate.edu
Ben Nguyen    btnguyen@iastate.edu

This program generates a basic dungeon model according to the assignment 1.04 specifications.
Our solution generates monsters randomly within the any of rooms of the dungeon except for the players room.
After generating all the entities, it places them all within a heap and pulls the entitiy with the minimum turn,
if the entity pulled is not the player, it makes a move based on its characteristics, smart or telepathic monsters
use either the tunneling or nontunneling Dijkstra map based on if it has the tunneling characteristic, and dumb or erratic
monsters move randomly (erratic only 50% of the time). If it encounters another entitity on the tile it moves to, it murders them.
The game ends if either the player or all monsters die.
