#pragma once
#include "SFML/Graphics.hpp"
#include "C.hpp"
#include <map>

static const float FRICTION = 10.0f;
static const float GRAVITY = 60.0f;
static const float MAX_FALL_SPEED = 25.0f;

struct Anim {
	std::vector<sf::IntRect> frames;
	float speed;
	bool loop;
};

class Game;
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
	void createAnim(std::string name, int startX, int startY, int w, int h, int count, float speed, bool loop = true);
	void play(std::string name);
	void tickAnim(double dt);

	virtual void Update(Game& game, double dt);
	void Draw(sf::RenderWindow& win);
	sf::Sprite* sprite;

protected:
	sf::Texture tex;

	std::map<std::string, Anim> anims;
	std::string currentAnim = "";
	int frameIndex = 0;
	float frameTimer = 0.0f;
	bool flipX = false;
};
