#pragma once

#include "Entity.h"

class Drone : public Entity {
public:
	Drone(int x, int y);

	void UpdateLogic(Game& game, double dt, Entity* player);

	float angle = 0.0f;
	float orbitSpeed = 3.0f;
	float radius = 50.0f;
	float lerpSpeed = 5.0f;
};