#include "Enemy.h"

Enemy::Enemy(int x, int y) : Entity(x, y)
{
	sprite->setColor(sf::Color(255, 100, 100, 100));
	speed = 10;
}

void Enemy::UpdateAI(Game& game, double dt, Entity* player)
{
	float diffX = player->cx + player->xr - (cx + xr);

	if (std::abs(diffX) > 0.5f)
	{
		if (diffX > 0)
		{
			dx = speed;
			sprite->setScale(1, 1);
		}
		else
		{
			dx = -speed;
			sprite->setScale(-1, 1);
		}
	}
	else
	{
		dx = 0;
	}
	Entity::Update(game, dt);
}