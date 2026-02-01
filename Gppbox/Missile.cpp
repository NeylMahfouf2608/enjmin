#include "Missile.h"
#include "C.hpp"
#include <cmath>

static const float PI = 3.14159265f;

float lerpAngle(float a, float b, float t) {
	float diff = b - a;
	while (diff < -180) diff += 360;
	while (diff > 180) diff -= 360;
	return a + diff * t;
}

Missile::Missile(float startX, float startY) : x(startX), y(startY) {
	if (!tex.loadFromFile("res/missile.png")) {
	}
	sprite.setTexture(tex);

	sf::Vector2u size = tex.getSize();
	sprite.setOrigin(size.x / 2.0f, size.y / 2.0f);

	sprite.setPosition(x, y);
	sprite.setScale(0, 0);

	angle = -90.0f + (rand() % 90 - 45);

	speed = 500.f;
}

void Missile::Update(double dt, std::vector<Enemy*>& enemies) {
	lifeTime += dt;

	if (lifeTime < launchDuration) {
		speed -= speed * 5.0f * dt;
	}
	else {
		if (speed < maxSpeed) speed += 600.0f * dt;

		Enemy* target = nullptr;
		float minDist = 999999.0f;

		for (Enemy* e : enemies) {
			float dx = (e->cx + e->xr) * C::GRID_SIZE - x;
			float dy = (e->cy + e->yr) * C::GRID_SIZE - y;
			float distSq = dx * dx + dy * dy;

			if (distSq < minDist) {
				minDist = distSq;
				target = e;
			}
		}

		if (target) {
			float targetX = (target->cx + target->xr) * C::GRID_SIZE;
			float targetY = (target->cy + target->yr) * C::GRID_SIZE;

			float dx = targetX - x;
			float dy = targetY - y;

			float targetAngle = std::atan2(dy, dx) * 180.0f / PI;

			angle = lerpAngle(angle, targetAngle, 5.0f * (float)dt);
		}
	}

	sprite.setRotation(angle);

	float rad = angle * PI / 180.0f;
	x += std::cos(rad) * speed * (float)dt;
	y += std::sin(rad) * speed * (float)dt;

	sprite.setPosition(x, y);
	sprite.setScale(scale, scale);
}

void Missile::Draw(sf::RenderWindow& win) {
	if (sprite.getTexture() == nullptr) {
		sf::RectangleShape r(sf::Vector2f(20, 5));
		r.setOrigin(0, 2.5f);
		r.setPosition(x, y);
		r.setRotation(angle);
		r.setFillColor(sf::Color::Cyan);
		r.setScale(scale, scale);
		win.draw(r);
	}
	else {
		win.draw(sprite);
	}
}