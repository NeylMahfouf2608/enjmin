#include "Entity.h"

Entity::Entity(int x, int y) : cx(x), cy(y)
{
	tex.loadFromFile("res/char.png");
	sprite = new sf::Sprite(tex);
	sprite->setOrigin(sf::Vector2f(0.5*C::GRID_SIZE, 1*C::GRID_SIZE));
	sprite->setPosition(sf::Vector2f(x, y));
	sprite->setScale(sf::Vector2f(1, 1));
}
void Entity::Update(Game game)
{
	xx = (cx + xr) * C::GRID_SIZE;
	yy = (cy + yr) * C::GRID_SIZE;

	if(!game.isWall(cx+(dx > 0 ? 1 : -1), cy) && !game.isWall(cx + (dx > 0 ? 1 : -1), cy-1))
	xr += dx;
	while (xr > 1) { xr--; cx++; }
	while (xr < 0) { xr++; cx--; }

	dx *= 0.96;

	if (!game.isWall(cx, cy + (dy > 0 ? 1 : -1)))
	yr += dy;
	while (yr > 1) { cy++; yr--; }
	while (yr < 0) { cy--; yr++; }

	dy *= 0.96;

	sprite->setPosition(sf::Vector2f(xx, yy));
}

void Entity::MoveX(int dx)
{

}

void Entity::Draw(sf::RenderWindow& win)
{
	win.draw(*sprite);
}