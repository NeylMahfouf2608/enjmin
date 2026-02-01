#include "Drone.h"
#include "Game.hpp"
#include <cmath>

Drone::Drone(int x, int y) : Entity(x, y) {
	if (tex.loadFromFile("res/missile.png")) {
		sprite = new sf::Sprite(tex);
		sprite->setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);

		xx = (float)x * C::GRID_SIZE;
		yy = (float)y * C::GRID_SIZE;
		sprite->setPosition(xx, yy);
	}
}

void Drone::UpdateLogic(Game& game, double dt, Entity* player) {
	angle += orbitSpeed * (float)dt;

	float playerX = (player->cx + player->xr) * C::GRID_SIZE;
	float playerY = (player->cy + player->yr) * C::GRID_SIZE - 20.0f;

	float targetX = playerX + std::cos(angle) * radius;
	float targetY = playerY + std::sin(angle) * radius;

	float dx = targetX - xx;
	float dy = targetY - yy;
	float distSq = dx * dx + dy * dy;

	if (distSq > 300.0f * 300.0f) {
		xx = targetX;
		yy = targetY;
	}

	int px = (int)(playerX / C::GRID_SIZE);
	int py = (int)(playerY / C::GRID_SIZE);
	int tx = (int)(targetX / C::GRID_SIZE);
	int ty = (int)(targetY / C::GRID_SIZE);

	auto line = Bresenham(px, py, tx, ty);
	for (auto& p : line) {
		if (game.isWall(p.x, p.y)) {

			targetX = (float)p.x * C::GRID_SIZE + C::GRID_SIZE / 2.0f;
			targetY = (float)p.y * C::GRID_SIZE + C::GRID_SIZE / 2.0f;

			targetX = stoopid::lerp(targetX, playerX, 0.2f);
			targetY = stoopid::lerp(targetY, playerY, 0.2f);
			break;
		}
	}

	float smooth = lerpSpeed * (float)dt;
	xx = stoopid::lerp(xx, targetX, smooth);
	yy = stoopid::lerp(yy, targetY, smooth);

	sprite->setPosition(xx, yy);
	sprite->setRotation(angle * 50.0f);
}