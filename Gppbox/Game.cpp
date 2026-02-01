
#include <imgui.h>
#include <iostream>
#include <array>
#include <vector>
#include <sstream>

#include "C.hpp"
#include "Game.hpp"

#include "HotReloadShader.hpp"
#include "Enemy.h"

static int cols = 10000;
static int lastLine = C::RES_Y / C::GRID_SIZE - 1;
static const float PI = 3.14159265f;

void Game::loadScenario(const char* filename) {
	std::ifstream file(filename);

	introLines.clear();
	std::string lineMeta;
	std::string lineText;


	while (std::getline(file, lineMeta)) {
		if (!std::getline(file, lineText)) break;

		std::stringstream ss(lineMeta);
		std::string name;
		int r, g, b;
		ss >> name >> r >> g >> b;

		DialogueLine dl;
		dl.name = name;
		dl.text = lineText;
		dl.color = sf::Color(r, g, b);

		introLines.push_back(dl);
	}

	file.close();
	std::cout << "scenario loaded : " << introLines.size() << " lines." << std::endl;
}

void Game::addBloodEffect(float x, float y) {
	for (int i = 0; i < 10; i++) {
		Particle* p = new Particle();
		p->x = x;
		p->y = y;
		p->el.setSize(sf::Vector2f(4, 4));
		p->el.setFillColor(sf::Color::Red);

		float angle = (rand() % 360) * 3.14f / 180.0f;
		float speed = (rand() % 100) + 50.0f;
		p->dx = std::cos(angle) * speed;
		p->dy = std::sin(angle) * speed;

		p->life = 0.5f + (rand() % 10) / 20.0f;

		p->bhv = [](Particle* pt, float dt) {
			pt->dy += 800.0f * dt;
			pt->scaleX -= 1.0f * dt;
			pt->scaleY -= 1.0f * dt;
			};
		particles.push_back(p);
	}
}


Game::Game(sf::RenderWindow* win) {
	this->win = win;
	gameView = win->getDefaultView();

	if (!font.loadFromFile("res/MAIAN.TTF")) {
	}

	text.setFont(font);
	text.setCharacterSize(24);

	name.setFont(font);
	name.setCharacterSize(30);
	name.setStyle(sf::Text::Bold);

	loadScenario("scenario.txt");

	inCutscene = true;

	bg = sf::RectangleShape(Vector2f((float)win->getSize().x, (float)win->getSize().y));

	bool isOk = tex.loadFromFile("res/bg_stars.png");
	if (!isOk) {
		printf("ERR : LOAD FAILED\n");
	}
	bg.setTexture(&tex);
	bg.setSize(sf::Vector2f(C::RES_X, C::RES_Y));

	bgShader = new HotReloadShader("res/bg.vert", "res/bg.frag");

	loadLevel("level1.txt");
	cacheWalls();
	player = new Entity(1, 10);
	drone = new Drone(1, 10);

	initLayer("res/1.png", 0.05f, 0.05f);
	initLayer("res/2.png", 0.2f, 0.05f);
	initLayer("res/3.png", 0.5f, 0.1f);

	defaultViewSize = gameView.getSize();
	currentZoom = 0.5f;
	targetZoom = 0.5f;
	gameView.setSize(defaultViewSize.x * currentZoom, defaultViewSize.y * currentZoom);
}

void Game::cacheWalls()
{
	wallSprites.clear();
	for (const auto& b : blocks) {
		sf::RectangleShape rect(sf::Vector2f(16, 16));
		rect.setPosition((float)b.x * C::GRID_SIZE, (float)b.y * C::GRID_SIZE);
		rect.setFillColor(b.color);

		wallSprites.push_back(rect);
	}
}

void Game::initLayer(const char* path, float speedX, float speedY) {
	ParallaxLayer* layer = new ParallaxLayer();

	if (layer->texture.loadFromFile(path)) {
		layer->texture.setRepeated(true);
		layer->shape.setTexture(&layer->texture);
	}
	else {
		layer->shape.setFillColor(sf::Color(255, 255, 255, 50));
	}

	layer->speedFactor = sf::Vector2f(speedX, speedY);
	parallaxLayers.push_back(layer);
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
			blocks.clear();
			cacheWalls();
		}
	}
	if (editorMode && !ImGui::GetIO().WantCaptureMouse) {
		if (ev.type == sf::Event::MouseWheelScrolled) {
			if (editorMode) {
				float delta = ev.mouseWheelScroll.delta;
				targetZoom -= delta * 0.1f;
				if (targetZoom < 0.1f) targetZoom = 0.1f;
				if (targetZoom > 2.0f) targetZoom = 2.0f;

				Tween t;
				t.value = &currentZoom;
				t.start = currentZoom;
				t.end = targetZoom;
				t.duration = 0.2f;
				t.easing = Ease::OutQuad;
				tweens.push_back(t);
			}
		}
	}
}


static double g_time = 0.0;
static double g_tickTimer = 0.0;


void Game::pollInput(double dt) {

	float moveSpeed = 20.0f;
	float jumpForce = -22.0f;


	bool hasController = sf::Joystick::isConnected(0);
	float joystickX = hasController ? sf::Joystick::getAxisPosition(0, sf::Joystick::X) : 0.0f;
	float joystickAimX = hasController ? sf::Joystick::getAxisPosition(0, sf::Joystick::U) : 0.0f;
	float joystickAimY = hasController ? sf::Joystick::getAxisPosition(0, sf::Joystick::V) : 0.0f;

	if (std::abs(joystickX) < 20.0f) joystickX = 0.0f;
	if (std::abs(joystickAimX) < 20.0f) joystickAimX = 0.0f;
	if (std::abs(joystickAimY) < 20.0f) joystickAimY = 0.0f;


	bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || joystickX < -20.0f;
	bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || joystickX > 20.0f;

	if (moveLeft) {
		if (!isWall(player->cx - 1, player->cy) && !isWall(player->cx - 1, player->cy - 1))
			player->dx = -moveSpeed;
	}

	if (moveRight) {
		player->dx = moveSpeed;
	}

	bool jumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || (hasController && sf::Joystick::isButtonPressed(0, 0));

	if (jumpPressed) {
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

	bool fireLaser = sf::Mouse::isButtonPressed(sf::Mouse::Left) || (hasController && sf::Joystick::isButtonPressed(0, 2));

	if (!editorMode && fireLaser && !ImGui::GetIO().WantCaptureMouse) {
		static float shootCooldown = 0.0f;
		if (shootCooldown <= 0.0f) {
			shootCooldown = 0.15f;

			laserTimer = 0.2f;
			addShake(5.0f);

			sf::Vector2f worldPos;
			bool isAimingWithStick = (std::abs(joystickAimX) > 0.0f || std::abs(joystickAimY) > 0.0f);

			if (isAimingWithStick) {
				sf::Vector2f playerCenter((player->cx + 0.5f) * C::GRID_SIZE, (player->cy + 0.5f) * C::GRID_SIZE);
				worldPos = playerCenter + sf::Vector2f(joystickAimX, joystickAimY) * 10.0f;
			}
			else {
				sf::Vector2i mousePos = sf::Mouse::getPosition(*win);
				worldPos = win->mapPixelToCoords(mousePos);
			}

			sf::Vector2f startPos = player->sprite->getPosition() + sf::Vector2f(0.0f, -20.0f);


			float kickback = 10.0f;
			if (worldPos.x > player->cx * C::GRID_SIZE)
				player->dx -= kickback;
			else
				player->dx += kickback;

			sf::Vector2f dir = worldPos - startPos;
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len != 0) dir /= len;

			sf::Vector2f endRay = startPos + dir * 1000.0f;
			int targetX = (int)(endRay.x / C::GRID_SIZE);
			int targetY = (int)(endRay.y / C::GRID_SIZE);

			int startGridX = (int)(startPos.x / C::GRID_SIZE);
			int startGridY = (int)(startPos.y / C::GRID_SIZE);

			auto line = Bresenham(startGridX, startGridY, targetX, targetY);
			bool hasHit = false;
			sf::Vector2i endPoint = sf::Vector2i(targetX, targetY);

			for (auto& p : line) {
				if (isWall(p.x, p.y)) {
					endPoint = p;
					break;
				}

				for (auto it = enemies.begin(); it != enemies.end(); ) {
					Enemy* e = *it;

					float ex = e->cx + e->xr;
					float ey = e->cy + e->yr;

					float distSq = std::pow(ex - (p.x + 0.5f), 2) + std::pow(ey - (p.y + 0.5f), 2);
					if (distSq < 0.64f) {
						freeze(0.05f);
						addShake(10.0f);

						e->TakeDamage(4, *this);
						hasHit = true;

						endPoint = p;
						break;
					}
					else {
						++it;
					}
				}
				if (hasHit) break;
			}
		}
		shootCooldown -= dt;
	}

	bool fireMissile = sf::Mouse::isButtonPressed(sf::Mouse::Right) || (hasController && sf::Joystick::isButtonPressed(0, 1));

	if (fireMissile && !ImGui::GetIO().WantCaptureMouse) {
		static float missileCooldown = 0.0f;
		if (missileCooldown <= 0.0f) {
			missileCooldown = 0.15f;

			Missile* m = new Missile(drone->xx, drone->yy);
			missiles.push_back(m);

			Tween t;
			t.value = &m->scale;
			t.start = 0.0f;
			t.end = 1.0f;
			t.duration = 0.4f;
			t.easing = Ease::OutBack;

			tweens.push_back(t);
		}
		missileCooldown -= dt;
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

	if (inCutscene) {
		if (currentLineIndex < introLines.size()) {
			DialogueLine& line = introLines[currentLineIndex];

			if (!lineFinished) {
				charTimer += (float)dt;
				if (charTimer > 0.03f) {
					charTimer = 0.0f;
					visibleChars++;

					if (visibleChars >= line.text.size()) {
						visibleChars = (int)line.text.size();
						lineFinished = true;
					}
				}
			}

			static bool spaceWasPressed = false;
			bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Mouse::isButtonPressed(sf::Mouse::Left);

			if (spacePressed && !spaceWasPressed) {
				if (!lineFinished) {
					visibleChars = (int)line.text.size();
					lineFinished = true;
				}
				else {
					currentLineIndex++;
					visibleChars = 0;
					lineFinished = false;

					if (currentLineIndex >= introLines.size()) {
						inCutscene = false;
					}
				}
			}
			spaceWasPressed = spacePressed;
		}
		return;
	}
	if (hitStopTimer > 0)
	{
		hitStopTimer -= dt;
		if (hitStopTimer < 0) hitStopTimer = 0;
		dt = 0.0;
	}

	for (auto it = missiles.begin(); it != missiles.end(); ) {
		Missile* m = *it;
		m->Update(dt, enemies);
		bool hit = false;

		for (auto eIt = enemies.begin(); eIt != enemies.end(); ) {
			Enemy* e = *eIt;

			float ex = (e->cx + e->xr) * C::GRID_SIZE;
			float ey = (e->cy + e->yr) * C::GRID_SIZE;
			float dist = std::sqrt(std::pow(m->x - ex, 2) + std::pow(m->y - ey, 2));

			if (dist < 20.0f) {
				addShake(5.0f);

				e->TakeDamage(10, *this);

				hit = true;
				break;
			}
			else {
				++eIt;
			}
		}

		if (hit || !m->active) {
			delete m;
			it = missiles.erase(it);
		}
		else {
			++it;
		}
	}

	for (auto it = tweens.begin(); it != tweens.end(); ) {
		it->update((float)dt);
		if (!it->active) {
			it = tweens.erase(it);
		}
		else {
			++it;
		}
	}

	if (laserTimer > 0) laserTimer -= dt;

	pollInput(dt);

	if (shakeTimer > 0) {
		shakeTimer -= dt;
		if (shakeTimer < 0) shakeTimer = 0;
	}
	g_time += dt;
	if (bgShader) bgShader->update(dt);

	if (!editorMode) {

		float desiredZoom = (std::abs(player->dx) > 15.0f) ? 0.55f : 0.5f;

		if (std::abs(targetZoom - desiredZoom) > 0.01f) {
			targetZoom = desiredZoom;

			Tween t;
			t.value = &currentZoom;
			t.start = currentZoom;
			t.end = targetZoom;
			t.duration = 0.8f;
			t.easing = Ease::OutQuad;

			tweens.push_back(t);
		}
	}

	gameView.setSize(defaultViewSize.x * currentZoom, defaultViewSize.y * currentZoom);

	if (editorMode) {
		float panSpeed = 500.0f * (float)dt * currentZoom;
		int margin = 50;
		sf::Vector2i mousePos = sf::Mouse::getPosition(*win);
		sf::Vector2u winSize = win->getSize();

		if (mousePos.x < margin) gameView.move(-panSpeed, 0);
		if (mousePos.x > (int)winSize.x - margin) gameView.move(panSpeed, 0);
		if (mousePos.y < margin) gameView.move(0, -panSpeed);
		if (mousePos.y > (int)winSize.y - margin) gameView.move(0, panSpeed);
	}
	else {
		sf::Vector2f targetPos;
		targetPos.x = (player->cx + player->xr) * C::GRID_SIZE;
		targetPos.y = (player->cy + player->yr) * C::GRID_SIZE;

		float lookAheadFactor = 15.0f;
		targetPos.x += player->dx * lookAheadFactor;

		float camSpeed = 5.0f * (float)dt;

		sf::Vector2f currentPos = gameView.getCenter();
		float newX = lerp(currentPos.x, targetPos.x, camSpeed);
		float newY = lerp(currentPos.y, targetPos.y, camSpeed);

		gameView.setCenter(newX, newY);


		win->setView(gameView);
		beforeParts.update(dt);
		afterParts.update(dt);

		player->Update(*this, dt);
		drone->UpdateLogic(*this, dt, player);

		for (auto it = particles.begin(); it != particles.end(); ) {
			(*it)->update((float)dt);
			if ((*it)->destroyed) {
				delete* it;
				it = particles.erase(it);
			}
			else {
				++it;
			}
		}

		for (auto it = enemies.begin(); it != enemies.end(); ) {
			Enemy* e = *it;
			e->UpdateAI(*this, dt, player);
			if (e->isDead && e->deadTimer > 4.0f) {
				delete e;
				it = enemies.erase(it);
			}
			else {
				++it;
			}
		}
	}
}

void Game::draw(sf::RenderWindow& win) {
	if (closing) return;

	if (inCutscene) {

		win.setView(win.getDefaultView());
		win.clear(sf::Color::Black);

		if (currentLineIndex < introLines.size()) {
			DialogueLine& line = introLines[currentLineIndex];

			sf::Vector2u winSize = win.getSize();
			float centerX = winSize.x / 2.0f;
			float centerY = winSize.y / 2.0f;

			name.setString(line.name);
			name.setFillColor(line.color);
			sf::FloatRect nameRect = name.getLocalBounds();
			name.setOrigin(nameRect.width / 2.0f, nameRect.height / 2.0f);
			name.setPosition(centerX, centerY - 50);
			win.draw(name);

			std::string subString = line.text.substr(0, visibleChars);
			text.setString(subString);
			text.setFillColor(line.color);

			sf::FloatRect textRect = text.getLocalBounds();
			text.setOrigin(textRect.width / 2.0f, textRect.height / 2.0f);
			text.setPosition(centerX, centerY);

			win.draw(text);

			if (lineFinished) {
				sf::CircleShape nextDot(5);
				nextDot.setFillColor(sf::Color::White);
				nextDot.setPosition(centerX, centerY + 40);
				if ((int)(g_time * 2) % 2 == 0) win.draw(nextDot);
			}
		}
		return;
	}

	sf::View logicView = gameView;
	sf::View renderView = logicView;

	if (shakeTimer > 0) {
		float offX = (rand() % 100 - 50) / 50.0f * shakeStrength;
		float offY = (rand() % 100 - 50) / 50.0f * shakeStrength;
		renderView.move(offX, offY);
	}

	win.setView(renderView);
	sf::RenderStates states = sf::RenderStates::Default;

	sf::Vector2f viewCenter = renderView.getCenter();
	sf::Vector2f viewSize = renderView.getSize();

	for (ParallaxLayer* layer : parallaxLayers) {

		layer->shape.setSize(viewSize);
		layer->shape.setOrigin(viewSize.x / 2.0f, viewSize.y / 2.0f);
		layer->shape.setPosition(viewCenter);

		int texX = (int)(viewCenter.x * layer->speedFactor.x);
		int texY = (int)(viewCenter.y * layer->speedFactor.y);
		layer->shape.setTextureRect(sf::IntRect(texX, texY, (int)viewSize.x, (int)viewSize.y));

		win.draw(layer->shape, states);
	}

	beforeParts.draw(win);

	for (sf::RectangleShape& r : wallSprites)
		win.draw(r);

	for (sf::RectangleShape& r : rects)
		win.draw(r);

	for (auto* p : particles) {
		win.draw(p->el);
	}

	if (!editorMode) {
		sf::Vector2i mousePos = sf::Mouse::getPosition(win);
		sf::Vector2f worldMouse = win.mapPixelToCoords(mousePos);
		sf::Vector2f playerCenter((player->cx + 0.5f) * C::GRID_SIZE, (player->cy + 0.5f) * C::GRID_SIZE);

		float dx = worldMouse.x - playerCenter.x;
		float dy = worldMouse.y - playerCenter.y;
		float len = std::sqrt(dx * dx + dy * dy);
		if (len == 0) len = 1;

		float dirX = dx / len;
		float dirY = dy / len;

		int maxDist = 200;
		int targetGX = player->cx + (int)(dirX * maxDist);
		int targetGY = player->cy + (int)(dirY * maxDist);

		auto line = Bresenham(player->cx, player->cy, targetGX, targetGY);
		sf::Vector2i impact = sf::Vector2i(targetGX, targetGY);

		for (auto& p : line) {
			if (isWall(p.x, p.y)) {
				impact = p;
				break;
			}
			bool hitEnemy = false;
			for (auto* e : enemies) {
				float ex = e->cx + e->xr;
				float ey = e->cy + e->yr;
				float distSq = std::pow(ex - (p.x + 0.5f), 2) + std::pow(ey - (p.y + 0.5f), 2);

				if (distSq < 0.64f) {
					impact = p;
					hitEnemy = true;
					break;
				}
			}
			if (hitEnemy) break;
		}

		sf::Vector2f impactPos((impact.x + 0.5f) * C::GRID_SIZE, (impact.y + 0.5f) * C::GRID_SIZE);
		sf::Vector2f diff = impactPos - playerCenter;
		float beamLength = std::sqrt(diff.x * diff.x + diff.y * diff.y);
		float angle = std::atan2(diff.y, diff.x) * 180.0f / PI;

		sf::RectangleShape beam;
		beam.setPosition(playerCenter);

		float ratio = laserTimer / 0.2f;
		if (ratio < 0.0f) ratio = 0.0f;
		if (ratio > 1.0f) ratio = 1.0f;

		float thickness = 1.0f + (ratio * 12.0f);
		beam.setSize(sf::Vector2f(beamLength, thickness));

		beam.setOrigin(0, thickness / 2.0f);
		beam.setRotation(angle);

		if (laserTimer > 0.0f)
			beam.setFillColor(sf::Color::Red);
		else
			beam.setFillColor(sf::Color(255, 0, 0, 100));

		win.draw(beam);
	}

	if (laserTimer > 0.05f) {
		sf::CircleShape muzzle(8);
		muzzle.setFillColor(sf::Color::Yellow);
		muzzle.setPosition((player->cx + player->xr) * C::GRID_SIZE, (player->cy + player->yr) * C::GRID_SIZE);
		win.draw(muzzle);
	}

	afterParts.draw(win);
	for (Enemy* e : enemies) {
		e->Draw(win);
	}

	for (Missile* m : missiles) {
		m->Draw(win);
	}

	player->Draw(win);
	drone->Draw(win);
	win.setView(logicView);
}

void Game::onSpacePressed() {

}


bool Game::isWall(int cx, int cy)
{
	for (const auto& b : blocks) {
		if (b.x == cx && b.y == cy)
			return true;
	}
	return false;
}

void Game::im() {
	ImGui::Begin("Level Editor");
	ImGui::Checkbox("EDITOR MODE", &editorMode);

	if (!editorMode) {
		ImGui::End();
		return;
	}

	ImGui::Separator();

	ImGui::Text("Outil :");
	ImGui::RadioButton("Pinceau Blocs", &currentTool, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Tampon Ennemi", &currentTool, 1);

	ImGui::Separator();

	if (currentTool == 0) {
		ImGui::Text("Configuration :");
		ImGui::ColorEdit4("Teinte", currentColor);
	}
	else {
		ImGui::Text("Type d'ennemi :");
		const char* enemiesList[] = { "Soldat" };
		ImGui::Combo("Type", &currentEnemyType, enemiesList, IM_ARRAYSIZE(enemiesList));
	}

	ImGui::Separator();

	static char filename[128] = "level1.txt";
	ImGui::InputText("Fichier", filename, sizeof(filename));
	if (ImGui::Button("Sauvegarder")) saveLevel(filename);
	ImGui::SameLine();
	if (ImGui::Button("Charger")) loadLevel(filename);

	ImGui::End();
	bool isLeftDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	bool isRightDown = sf::Mouse::isButtonPressed(sf::Mouse::Right);

	static bool wasLeftDown = false;
	bool isLeftClicked = isLeftDown && !wasLeftDown;
	wasLeftDown = isLeftDown;

	if (!ImGui::GetIO().WantCaptureMouse) {

		if (isLeftDown || isRightDown) {
			sf::Vector2i mousePos = sf::Mouse::getPosition(*win);
			sf::Vector2f worldPos = win->mapPixelToCoords(mousePos, gameView);

			int gx = static_cast<int>(worldPos.x / C::GRID_SIZE);
			int gy = static_cast<int>(worldPos.y / C::GRID_SIZE);

			if (gx >= 0 && gx < cols && gy >= 0 && gy < (C::RES_Y / C::GRID_SIZE)) {

				if (currentTool == 0) {
					int foundIndex = -1;
					for (size_t i = 0; i < blocks.size(); i++) {
						if (blocks[i].x == gx && blocks[i].y == gy) {
							foundIndex = (int)i;
							break;
						}
					}

					if (isRightDown) {
						if (foundIndex != -1) {
							blocks.erase(blocks.begin() + foundIndex);
							cacheWalls();
						}
					}
					else if (foundIndex == -1 && isLeftDown) {
						BlockData b;
						b.x = gx;
						b.y = gy;
						b.color = sf::Color(
							(sf::Uint8)(currentColor[0] * 255),
							(sf::Uint8)(currentColor[1] * 255),
							(sf::Uint8)(currentColor[2] * 255),
							(sf::Uint8)(currentColor[3] * 255)
						);
						blocks.push_back(b);
						cacheWalls();
					}
				}

				else if (currentTool == 1 && !isRightDown && isLeftClicked) {

					bool exists = false;
					for (const auto& e : enemiesData) {
						if (e.x == gx && e.y == gy) { exists = true; break; }
					}

					if (!exists) {
						EnemyData ed;
						ed.x = gx;
						ed.y = gy;
						ed.type = currentEnemyType;
						enemiesData.push_back(ed);

						Enemy* newE = new Enemy(gx, gy);
						newE->xx = (float)gx * C::GRID_SIZE;
						newE->yy = (float)gy * C::GRID_SIZE;
						newE->sprite->setPosition(newE->xx, newE->yy);

						enemies.push_back(newE);
					}
				}
			}
		}
	}
}
void Game::saveLevel(const char* filename) {
	std::ofstream file(filename);
	if (file.is_open()) {
		for (const auto& b : blocks) {
			file << "BLOCK " << b.x << " " << b.y << " " << b.textureID << " "
				<< (int)b.color.r << " " << (int)b.color.g << " " << (int)b.color.b << "\n";
		}
		for (const auto& e : enemiesData) {
			file << "ENEMY " << e.x << " " << e.y << " " << e.type << "\n";
		}
		file.close();
		std::cout << "Level saved!" << std::endl;
	}
}

void Game::loadLevel(const char* filename) {
	std::ifstream file(filename);
	if (file.is_open()) {
		blocks.clear();
		enemiesData.clear();

		for (auto* e : enemies) delete e;
		enemies.clear();

		std::string type;
		while (file >> type) {
			if (type == "BLOCK") {
				BlockData b;
				int r, g, bl;
				file >> b.x >> b.y >> b.textureID >> r >> g >> bl;
				b.color = sf::Color(r, g, bl);
				blocks.push_back(b);
			}
			else if (type == "ENEMY") {
				EnemyData e;
				file >> e.x >> e.y >> e.type;
				enemiesData.push_back(e);

				enemies.push_back(new Enemy(e.x, e.y));
			}
		}
		cacheWalls();
		std::cout << "Level loaded !" << std::endl;
	}
}

std::vector<sf::Vector2i> Bresenham(int x0, int y0, int x1, int y1) {
	std::vector<sf::Vector2i> line;
	int dx = std::abs(x1 - x0);
	int dy = -std::abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;

	while (true) {
		line.push_back(sf::Vector2i(x0, y0));
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
	return line;
}