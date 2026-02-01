#include "Entity.h"
#include "Game.hpp"
Entity::Entity(int x, int y) : cx(x), cy(y), dx(0), dy(0), xr(0), yr(0), xx(0), yy(0) // stoopid boring warning from VS
{
	tex.loadFromFile("res/adventurer-Sheet.png");
	sprite = new sf::Sprite(tex);
	sprite->setOrigin(sf::Vector2f(25, 37));
	sprite->setPosition(sf::Vector2f(x, y));
	sprite->setScale(sf::Vector2f(1, 1));
	createAnim("idle", 0, 0, 50, 37, 4, 0.15f, true);
	createAnim("run", 0, 37, 50, 37, 6, 0.10f, true);
	createAnim("jump", 0, 74, 50, 37, 2, 0.15f, false);
	createAnim("fall", 100, 74, 50, 37, 2, 0.15f, true);
	createAnim("crouch", 0, 111, 50, 37, 4, 0.15f, true);
	createAnim("attack", 0, 185, 50, 37, 6, 0.08f, false);
	play("idle");
}
void Entity::createAnim(std::string name, int startX, int startY, int w, int h, int count, float speed, bool loop)
{
	Anim a;
	a.speed = speed;
	a.loop = loop;
	for (int i = 0; i < count; i++) {
		a.frames.push_back(sf::IntRect(startX + i * w, startY, w, h));
	}
	anims[name] = a;
}
void Entity::play(std::string name)
{
	if (currentAnim == name) return;

	if (anims.find(name) == anims.end()) return;

	currentAnim = name;
	frameIndex = 0;
	frameTimer = 0.0f;

	if (sprite) sprite->setTextureRect(anims[currentAnim].frames[0]);
}
void Entity::tickAnim(double dt)
{
	if (currentAnim == "") return;

	Anim& a = anims[currentAnim];
	frameTimer += (float)dt;

	if (frameTimer >= a.speed) {
		frameTimer -= a.speed;
		frameIndex++;

		if (frameIndex >= a.frames.size()) {
			if (a.loop) {
				frameIndex = 0;
			}
			else {
				frameIndex = (int)a.frames.size() - 1;
			}
		}

		if (sprite) sprite->setTextureRect(a.frames[frameIndex]);
	}

	if (dx > 0.1f) flipX = false;
	if (dx < -0.1f) flipX = true;

	if (sprite) {
		float absScaleX = std::abs(sprite->getScale().x);
		sprite->setScale(flipX ? -absScaleX : absScaleX, sprite->getScale().y);
	}
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


	float visualX = xx + (C::GRID_SIZE / 2.0f);
	float visualY = yy + C::GRID_SIZE;

	sprite->setPosition(sf::Vector2f(visualX, visualY));

	if (!game.isWall(cx, cy + 1) && std::abs(dy) > 0.0f) {
		if (dy < 0) play("jump");
		else play("fall");
	}
	else if (std::abs(dx) > 0.5f) {
		play("run");
	}
	else {
		play("idle");
	}

	tickAnim(dt);
}

void Entity::Draw(sf::RenderWindow& win)
{
	win.draw(*sprite);
}