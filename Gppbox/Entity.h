#pragma once
#include "SFML/Graphics.hpp"
#include "C.hpp"
#include "Game.hpp"

static const float FRICTION = 10.0f;
static const float GRAVITY = 60.0f;
static const float MAX_FALL_SPEED = 25.0f;
class Entity
{

public:
	float xr;
	float yr;

	float xx;
	float yy;
	int cx;
	int cy;
	float dx;
	float dy;
	Entity(int x, int y);
	virtual void Update(Game& game, double dt);
	void Draw(sf::RenderWindow& win);

protected:
	sf::Sprite* sprite;
	sf::Texture tex;
};
