#include "Entity.h"

Entity::Entity(int x, int y) : cx(x), cy(y), dx(0), dy(0), xr(0), yr(0), xx(0), yy(0) // stoopid boring warning from VS
{
	tex.loadFromFile("res/char.png");
	sprite = new sf::Sprite(tex);
	sprite->setOrigin(sf::Vector2f(0.5 * C::GRID_SIZE, 1 * C::GRID_SIZE));
	sprite->setPosition(sf::Vector2f(x, y));
	sprite->setScale(sf::Vector2f(1, 1));
}
void Entity::Update(Game& game, double dt)
{
	if (std::abs(dx) > 0.01f) {
		dx -= dx * FRICTION * dt;
	}
	else {
		dx = 0.0f;
	}

	float moveX = dx * dt;
	float nextXr = xr + moveX;
	int direction = (dx > 0 ? 1 : -1);

	if (game.isWall(cx + direction, cy) || game.isWall(cx + direction, cy - 1)) {
		dx = 0;
		moveX = 0;
	}

	xr += moveX;

	while (xr > 1.0f) { xr -= 1.0f; cx++; }
	while (xr < 0.0f) { xr += 1.0f; cx--; }

	dy += GRAVITY * dt;

	if (dy > MAX_FALL_SPEED) dy = MAX_FALL_SPEED;

	float moveY = dy * dt;

	if (dy > 0) {
		if (game.isWall(cx, cy + 1)) {
			if (yr + moveY > 0.0f) {
				dy = 0;
				yr = 0;
			}
			else {
				yr += moveY;
			}
		}
		else {
			yr += moveY;
		}
	}
	else if (dy < 0 && game.isWall(cx, cy - 1)) {
		dy = 0;
	}
	else {
		yr += moveY;
	}
	while (yr > 1.0f) { yr -= 1.0f; cy++; }
	while (yr < 0.0f) { yr += 1.0f; cy--; }

	xx = (cx + xr) * C::GRID_SIZE;
	yy = (cy + yr) * C::GRID_SIZE;

	sprite->setPosition(sf::Vector2f(xx, yy));
}

void Entity::Draw(sf::RenderWindow& win)
{
	win.draw(*sprite);
}