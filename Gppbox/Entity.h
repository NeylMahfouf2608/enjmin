#pragma once
#include "SFML/Graphics.hpp"
#include "C.hpp"
#include "Game.hpp"


class Entity
{
	sf::Sprite* sprite;
	sf::Texture tex;

	float xr;
	float yr;

	float xx;
	float yy;


public :
	int cx;
	int cy;
	float dx;
	float dy;
	Entity(int x, int y);
	void Update(Game game);
	void Draw(sf::RenderWindow& win);
	void MoveX(int dx);

};
