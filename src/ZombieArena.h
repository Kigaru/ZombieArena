#ifndef zombiearena_h
#define zombiearena_h
#include <SFML/Graphics.hpp>
#include "Zombie.h"
using namespace sf;

int createBackground(VertexArray& rVA, IntRect arena);
Zombie* createHorde(int numZombies, IntRect arena);

#endif