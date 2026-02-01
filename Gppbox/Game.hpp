#pragma once

#include <vector>

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include <fstream>
#include "sys.hpp"

#include "Particle.hpp"
#include "ParticleMan.hpp"
#include "Tween.h"
#include "Missile.h"
#include "drone.h"

using namespace sf;


struct DialogueLine {
	std::string name;
	std::string text;
	sf::Color color;
};

struct BlockData {
	int x, y;
	int textureID;
	sf::Color color;
};

struct EnemyData {
	int x, y;
	int type;
};

struct ParallaxLayer {
	sf::Texture texture;
	sf::RectangleShape shape;
	sf::Vector2f speedFactor;
};

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

	std::vector<BlockData>		blocks;
	std::vector<EnemyData>		enemiesData;
	std::vector<sf::RectangleShape> wallSprites;
	bool editorMode = false;
	bool inCutscene = true;
	std::vector<DialogueLine> introLines;
	std::vector<ParallaxLayer*> parallaxLayers;
	int currentLineIndex = 0;

	int currentTool = 0;
	int currentTextureID = 0;
	int currentEnemyType = 0;
	float currentColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float charTimer = 0.0f;
	int visibleChars = 0;
	bool lineFinished = false;
	Font font;
	Text text;
	Text name;

	std::vector<Particle*> particles;

	ParticleMan beforeParts;
	ParticleMan afterParts;
	Entity* player;
	Drone* drone;
	std::vector<Enemy*> enemies;
	std::vector<Tween> tweens;
	std::vector<Missile*> missiles;
	sf::View gameView;
	sf::Vector2f defaultViewSize;
	float currentZoom = 0.5f;
	float targetZoom = 0.5f;
	float laserTimer = 0.0f;
	float shakeTimer = 0.0f;
	float shakeStrength = 0.0f;
	float hitStopTimer = 0.0f;
	Game(sf::RenderWindow* win);

	void cacheWalls();

	void initLayer(const char* path, float speedX, float speedY);

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
	void loadScenario(const char* filename);

	void addBloodEffect(float x, float y);
};

std::vector<sf::Vector2i> Bresenham(int x0, int y0, int x1, int y1);

namespace stoopid {
	static float lerp(float a, float b, float t) {
		return a + (b - a) * t;
	}
}