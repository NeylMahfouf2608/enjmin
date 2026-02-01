#include <iostream> 
#include <array>
#include <numeric>
#include <algorithm>
#include <functional>

#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Audio.hpp>

#include "Bloom.hpp"
#include "Dice.hpp"
#include "Lib.hpp"
#include "Game.hpp"
#include "Interp.hpp"
#include "HotReloadShader.hpp"
#include "app.h"
#include "C.hpp"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
extern "C" {
	// Force the use of the NVidia-graphics card on notebooks with both an IGP and a GPU
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	// AMD equivalent to the above
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

using namespace std;
using namespace sf;

static HotReloadShader* bloomShader = nullptr;
static HotReloadShader* blurShader = nullptr;

static std::array<double, 60> dts;
static int curDts = 0;

int main()
{
	cout << "Hello World!\n";

	sf::RenderWindow window(sf::VideoMode(C::RES_X, C::RES_Y, 32), "SFML works!");
	//sf::RenderWindow window(sf::VideoMode(800, 600,32), "SFML works!");
	//sf::RenderWindow window(sf::VideoMode(1280, 720,32), "SFML works!");
	window.setVerticalSyncEnabled(false);
	Font font;

	if (!font.loadFromFile("res/MAIAN.TTF")) {
		cout << "ERROR NO FONT" << endl;
		return 1;
	}

	if (!sf::Shader::isAvailable())
	{
		cout << "ERROR NO SHADER SYSTEM" << endl;
		return 1;
	}

	ImGui::SFML::Init(window);

	Game g(&window);

	Vector2i winPos;

	View v = window.getDefaultView();
	Vector2f viewCenter = v.getCenter();

	sf::Clock timer;

	sf::Text fpsCounter;
	fpsCounter.setFont(font);
	fpsCounter.setString("FPS:");

	double frameStart = 0.0;
	double frameEnd = 0.0;

	sf::Texture winTex;
	winTex.create(window.getSize().x, window.getSize().y);

	bloomShader = new HotReloadShader("res/simple.vert", "res/bloom.frag");
	blurShader = new HotReloadShader("res/simple.vert", "res/blur.frag");
	sf::RenderTexture* destX = new sf::RenderTexture();
	destX->create(window.getSize().x, window.getSize().y);
	destX->clear(sf::Color(0, 0, 0, 0));

	sf::RenderTexture* destFinal = new sf::RenderTexture();
	destFinal->create(window.getSize().x, window.getSize().y);
	destFinal->clear(sf::Color(0, 0, 0, 0));

	float bloomWidth = 0;
	sf::Glsl::Vec4 bloomMul(1, 1, 1, 0.8f);

	while (window.isOpen())
	{
		double dt = frameEnd - frameStart;
		frameStart = Lib::getTimeStamp();

		if (dt < 0.00000001) {
			dt = 0.00000001;
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			g.processInput(event);

			if (event.type == sf::Event::Resized) {
				auto nsz = window.getSize();
				winTex.create(window.getSize().x, window.getSize().y);

				destX->create(window.getSize().x, window.getSize().y);
				destX->clear(sf::Color(0, 0, 0, 0));

				destFinal->create(window.getSize().x, window.getSize().y);
				destFinal->clear(sf::Color(0, 0, 0, 0));

				v = sf::View(Vector2f(nsz.x * 0.5f, nsz.y * 0.5f), Vector2f((float)nsz.x, (float)nsz.y));
				viewCenter = v.getCenter();
			}
		}

		//don't use imgui before this;
		ImGui::SFML::Update(window, sf::seconds((float)dt));

		g.update(dt);

		if (ImGui::CollapsingHeader("View")) {
			auto sz = v.getSize();
			ImGui::Value("size x", sz.x);
			ImGui::Value("size y", sz.y);
		}
		if (ImGui::CollapsingHeader("App Stats", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
			//double df = (Lib::getTimeStamp() - frameStart);

			double mdt = std::accumulate(dts.begin(), dts.end(), 0.0) / dts.size();
			ImGui::LabelText("Update Time", "%0.6f", dt);
			ImGui::LabelText("FPS", "%0.6f", 1.0 / dt);

			static double captureMdt = 0.0;
			if (curDts == 0) {
				captureMdt = mdt;
			}
			ImGui::LabelText("Avg Update Time", "%0.6f", captureMdt);
			ImGui::LabelText("Avg FPS", "%0.6f", 1.0 / captureMdt);
		}
		window.clear();

		//window.setView(v);//keep view up to date in case we want to do something with like... you know what.

		if (ImGui::CollapsingHeader("Bloom Control")) {
			ImGui::SliderFloat("bloomWidth", &bloomWidth, 0, 55);//55 is max acceptable kernel size for constants, otherwise we should use a texture
			ImGui::ColorEdit4("bloomMul", &bloomMul.x);
			ImGui::ColorEdit4("bloomMul2", &bloomMul.x);
		}
		g.im();

		g.draw(window);

		window.draw(fpsCounter);

		if (blurShader) blurShader->update(dt);
		if (bloomShader) bloomShader->update(dt);
		window.setView(window.getDefaultView());
		if (bloomWidth)
			Bloom::render(window, winTex, destX, destFinal, &blurShader->sh, &bloomShader->sh, bloomWidth, bloomMul);

		ImGui::SFML::Render(window);
		window.display();


		frameEnd = Lib::getTimeStamp();

		fpsCounter.setString("FPS: " + std::to_string(1.0 / dt));

		ImGui::EndFrame();

		curDts++;
		if (curDts >= dts.size()) {
			curDts = 0;
		}
		dts[curDts] = dt;
	}

	ImGui::SFML::Shutdown();

	return 0;
}