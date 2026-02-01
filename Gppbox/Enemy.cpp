#include "Enemy.h"
#include "Game.hpp"

Enemy::Enemy(int x, int y) : Entity(x, y)
{
	sprite->setColor(sf::Color(255, 100, 100, 255));
	speed = 10;
}

void Enemy::UpdateAI(Game& game, double dt, Entity* player)
{
	sf::Color c = sprite->getColor();
	if (c.g < 255 || c.b < 255) {
		c.g = std::min(c.g + 10, 255);
		c.b = std::min(c.b + 10, 255);
		sprite->setColor(c);
	}

	if (!isDead)
	{
		float diffX = player->cx + player->xr - (cx + xr);
		int direction = 0;

		if (std::abs(diffX) > 0.5f) {
			if (diffX > 0) {
				dx = speed;
				direction = 1;
				sprite->setScale(1, 1);
			}
			else {
				dx = -speed;
				direction = -1;
				sprite->setScale(-1, 1);
			}
		}
		else {
			dx = 0;
		}

		if (direction != 0) {
			bool wallAhead = game.isWall(cx + direction, cy);
			bool onGround = game.isWall(cx, cy + 1);

			if (wallAhead && onGround) {
				if (!game.isWall(cx, cy - 1)) {
					dy = -26.0f;
				}
			}
		}
	}
	else {
		if (!isOnGroundDead) {
			sprite->rotate(700.0f * (float)dt);

			if (game.isWall(cx, cy + 1) && dy >= 0) {
				isOnGroundDead = true;

				dy = 0;
				dx = 0;

				sprite->setRotation(90.0f);
				sprite->move(0, 10);
			}
		}
		else {
			deadTimer += (float)dt;

			if (deadTimer > 3.0f) {
				sf::Color c = sprite->getColor();
				if (c.a > 0) c.a -= 1;
				sprite->setColor(c);
			}
		}
	}

	Entity::Update(game, dt);
}

void Enemy::TakeDamage(int amount, Game& game) {
	if (isDead) return;

	hp -= amount;

	sf::Color hitColor = sf::Color::Red;
	hitColor.a = 255;
	sprite->setColor(hitColor);

	game.addBloodEffect(xx, yy);

	if (hp <= 0) {
		hp = 0;
		isDead = true;
		isOnGroundDead = false;

		dy = -20.0f;

		dx = (dx > 0) ? -15.0f : 15.0f;
	}
}