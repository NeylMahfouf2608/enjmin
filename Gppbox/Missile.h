#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Enemy.h"
#include <vector>

class Missile {
public:
	Missile(float x, float y);
	void Update(double dt, std::vector<Enemy*>& enemies);
	void Draw(sf::RenderWindow& win);

	sf::Sprite sprite;
	sf::Texture tex;

	float x, y;
	float speed = 0.0f;
	float angle = 0.0f;

	float scale = 0.0f;
	bool active = true;

	float lifeTime = 0.0f;
	float launchDuration = 0.4f;
	float maxSpeed = 400.0f;
};