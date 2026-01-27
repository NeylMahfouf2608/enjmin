#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
	float speed;
	Enemy(int x, int y);

	void UpdateAI(Game& game, double dt, Entity* player);

};