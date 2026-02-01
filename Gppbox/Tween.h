#pragma once
#include <functional>
#include <cmath>
#include <vector>
#include <algorithm>

namespace Ease {
	inline float Linear(float t) { return t; }
	inline float OutQuad(float t) { return t * (2 - t); }
	inline float OutBack(float t) {
		float c1 = 1.70158f;
		float c3 = c1 + 1;
		return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
	}
}

struct Tween {
	float* value;
	float start;
	float end;
	float duration;
	float time = 0.0f;
	std::function<float(float)> easing;
	bool active = true;

	void update(float dt) {
		if (!active) return;

		time += dt;
		float t = time / duration;

		if (t >= 1.0f) {
			t = 1.0f;
			active = false;
			if (value) *value = end;
		}
		else {
			if (value) *value = start + (end - start) * easing(t);
		}
	}
};