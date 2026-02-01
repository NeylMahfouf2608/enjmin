#pragma once
#include "Entity.h"

class Game;
class Enemy : public Entity {
public:
	float speed;
	int hp = 10;
	int maxHp = 10;

	bool isDead = false;
	bool isOnGroundDead = false;
	float deadTimer = 0.0f;

	void TakeDamage(int amount, Game& game);
	Enemy(int x, int y);

	void UpdateAI(Game& game, double dt, Entity* player);

};