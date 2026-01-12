#pragma once
#include "SFML/Graphics.hpp"
#include "C.hpp"
#include "Game.hpp"

// Constantes de physique (à tuner selon le
static const float FRICTION = 10.0f;
static const float GRAVITY = 60.0f;
static const float MAX_FALL_SPEED = 25.0f;
class Entity
{
	sf::Sprite* sprite;
	sf::Texture tex;

	float xr;
	float yr;

	float xx;
	float yy;


public:
	int cx;
	int cy;
	float dx;
	float dy;
	Entity(int x, int y);
	void Update(Game& game, double dt);
	void Draw(sf::RenderWindow& win);
};
