#pragma once

#include <vector>

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include "sys.hpp"

#include "Particle.hpp"
#include "ParticleMan.hpp"

using namespace sf;

class Entity;
class Enemy;
class HotReloadShader;
class Game {
public:
	sf::RenderWindow* win = nullptr;

	sf::RectangleShape				bg;
	HotReloadShader* bgShader = nullptr;

	sf::Texture						tex;

	bool							closing = false;

	std::vector<sf::Vector2i>		walls;
	std::vector<sf::RectangleShape> wallSprites;

	ParticleMan beforeParts;
	ParticleMan afterParts;
	Entity* player;
	std::vector<Enemy*> enemies;
	sf::VertexArray laserBeam{ sf::Lines };
	float laserTimer = 0.0f;
	float shakeTimer = 0.0f;
	float shakeStrength = 0.0f;
	float hitStopTimer = 0.0f;
	Game(sf::RenderWindow* win);

	void cacheWalls();

	void processInput(sf::Event ev);
	bool wasPressed = false;
	void pollInput(double dt);
	void onSpacePressed();

	void addShake(float amount) { shakeTimer = 0.2f; shakeStrength = amount; }
	void freeze(float duration) { hitStopTimer = duration; }
	void update(double dt);

	void draw(sf::RenderWindow& win);

	bool isWall(int cx, int cy);
	void im();

	void saveLevel(const char* filename);
	void loadLevel(const char* filename);
};

std::vector<sf::Vector2i> Bresenham(int x0, int y0, int x1, int y1);