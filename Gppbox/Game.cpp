
#include <imgui.h>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>

#include "C.hpp"
#include "Game.hpp"

#include "HotReloadShader.hpp"
#include "Entity.h"

static int cols = C::RES_X / C::GRID_SIZE;
static int lastLine = C::RES_Y / C::GRID_SIZE - 1;

Game::Game(sf::RenderWindow* win) {
	this->win = win;
	bg = sf::RectangleShape(Vector2f((float)win->getSize().x, (float)win->getSize().y));

	bool isOk = tex.loadFromFile("res/bg_stars.png");
	if (!isOk) {
		printf("ERR : LOAD FAILED\n");
	}
	bg.setTexture(&tex);
	bg.setSize(sf::Vector2f(C::RES_X, C::RES_Y));

	bgShader = new HotReloadShader("res/bg.vert", "res/bg.frag");

	for (int i = 0; i < C::RES_X / C::GRID_SIZE; ++i)
		walls.push_back(Vector2i(i, lastLine));

	walls.push_back(Vector2i(0, lastLine - 1));
	walls.push_back(Vector2i(0, lastLine - 2));
	walls.push_back(Vector2i(0, lastLine - 3));

	walls.push_back(Vector2i(cols - 1, lastLine - 1));
	walls.push_back(Vector2i(cols - 1, lastLine - 2));
	walls.push_back(Vector2i(cols - 1, lastLine - 3));

	walls.push_back(Vector2i(cols >> 2, lastLine - 2));
	walls.push_back(Vector2i(cols >> 2, lastLine - 3));
	walls.push_back(Vector2i(cols >> 2, lastLine - 4));
	walls.push_back(Vector2i((cols >> 2) + 1, lastLine - 4));
	cacheWalls();
	player = new Entity(10, 10);
}

void Game::cacheWalls()
{
	wallSprites.clear();
	for (Vector2i& w : walls) {
		sf::RectangleShape rect(Vector2f(16, 16));
		rect.setPosition((float)w.x * C::GRID_SIZE, (float)w.y * C::GRID_SIZE);
		rect.setFillColor(sf::Color(0x07ff07ff));
		wallSprites.push_back(rect);
	}
}

void Game::processInput(sf::Event ev) {
	if (ev.type == sf::Event::Closed) {
		win->close();
		closing = true;
		return;
	}
	if (ev.type == sf::Event::KeyReleased) {
		int here = 0;
		if (ev.key.code == Keyboard::K) {
			int there = 0;
			walls.clear();
			cacheWalls();
		}
	}
}


static double g_time = 0.0;
static double g_tickTimer = 0.0;


void Game::pollInput(double dt) {

	float moveSpeed = 20.0f;
	float jumpForce = -22.0f;
	float lateralSpeed = 8.0;
	float maxSpeed = 40.0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
		if (!isWall(player->cx - 1, player->cy) && !isWall(player->cx - 1, player->cy - 1))
			player->dx = -moveSpeed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		player->dx = moveSpeed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {

	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T)) {

	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
		if (!wasPressed) {
			if (isWall(player->cx, player->cy + 1)) {
				player->dy = jumpForce;
			}
			onSpacePressed();
			wasPressed = true;
		}
	}
	else {
		wasPressed = false;
	}
}

static sf::VertexArray va;
static RenderStates vaRs;
static std::vector<sf::RectangleShape> rects;

int blendModeIndex(sf::BlendMode bm) {
	if (bm == sf::BlendAlpha) return 0;
	if (bm == sf::BlendAdd) return 1;
	if (bm == sf::BlendNone) return 2;
	if (bm == sf::BlendMultiply) return 3;
	return 4;
};

void Game::update(double dt) {
	pollInput(dt);
	if (shakeTimer > 0) {
		shakeTimer -= dt;
		if (shakeTimer < 0) shakeTimer = 0;
	}
	g_time += dt;
	if (bgShader) bgShader->update(dt);

	beforeParts.update(dt);
	afterParts.update(dt);
	player->Update(*this, dt);
}

void Game::draw(sf::RenderWindow& win) {
	if (closing) return;
	sf::View currentView = win.getView();
	sf::Vector2f originalCenter = currentView.getCenter();

	if (shakeTimer > 0) {
		float offX = (rand() % 100 - 50) / 50.0f * shakeStrength;
		float offY = (rand() % 100 - 50) / 50.0f * shakeStrength;
		currentView.setCenter(originalCenter + sf::Vector2f(offX, offY));
		win.setView(currentView);
	}
	else {
	}

	sf::RenderStates states = sf::RenderStates::Default;
	sf::Shader* sh = &bgShader->sh;
	states.blendMode = sf::BlendAdd;
	states.shader = sh;
	states.texture = &tex;
	sh->setUniform("texture", tex);
	win.draw(bg, states);

	beforeParts.draw(win);

	for (sf::RectangleShape& r : wallSprites)
		win.draw(r);

	for (sf::RectangleShape& r : rects)
		win.draw(r);


	afterParts.draw(win);
	player->Draw(win);

	if (shakeTimer > 0) {
		currentView.setCenter(originalCenter);
		win.setView(currentView);
	}
}

void Game::onSpacePressed() {

}


bool Game::isWall(int cx, int cy)
{
	for (Vector2i& w : walls) {
		if (w.x == cx && w.y == cy)
			return true;
	}
	return false;
}

void Game::im() {
	ImGui::Begin("Level Editor");

	static char filename[128] = "level1.txt";
	ImGui::InputText("Filename", filename, sizeof(filename));

	if (ImGui::Button("Save")) saveLevel(filename);
	ImGui::SameLine();
	if (ImGui::Button("Load")) loadLevel(filename);

	ImGui::Separator();

	ImGui::Text("Left Click to build / Right Click to erase");

	if (ImGui::GetIO().WantCaptureMouse) {
		ImGui::End();
		return;
	}

	bool leftClick = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	bool rightClick = sf::Mouse::isButtonPressed(sf::Mouse::Right);

	if (leftClick || rightClick) {
		sf::Vector2i mousePos = sf::Mouse::getPosition(*win);
		sf::Vector2f worldPos = win->mapPixelToCoords(mousePos);

		int gx = static_cast<int>(worldPos.x / C::GRID_SIZE);
		int gy = static_cast<int>(worldPos.y / C::GRID_SIZE);

		if (gx >= 0 && gx < (C::RES_X / C::GRID_SIZE) && gy >= 0 && gy < (C::RES_Y / C::GRID_SIZE)) {
			bool exists = isWall(gx, gy);

			if (leftClick && !exists) {
				walls.push_back(sf::Vector2i(gx, gy));
				cacheWalls();
			}
			else if (rightClick && exists) {
				walls.erase(std::remove_if(walls.begin(), walls.end(),
					[gx, gy](sf::Vector2i v) { return v.x == gx && v.y == gy; }), walls.end());
				cacheWalls();
			}
		}
	}

	ImGui::End();
}

void Game::saveLevel(const char* filename) {
	std::ofstream file(filename);
	if (file.is_open()) {
		for (const auto& w : walls) {
			file << w.x << " " << w.y << " 1\n";
		}
		file.close();
		std::cout << "Niveau sauvegarde !" << std::endl;
	}
}

void Game::loadLevel(const char* filename) {
	std::ifstream file(filename);
	if (file.is_open()) {
		walls.clear();
		int x, y, type;
		while (file >> x >> y >> type) {
			if (type == 1) walls.push_back(sf::Vector2i(x, y));
		}
		cacheWalls();
		std::cout << "Niveau charge !" << std::endl;
	}
}
