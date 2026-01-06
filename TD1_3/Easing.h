#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <cmath>

class Easing {
public:
	// 線形補間
	static float Linear(float t) {
		return t;
	}

	// イーズイン（加速）
	static float EaseInQuad(float t) {
		return t * t;
	}

	// イーズアウト（減速）
	static float EaseOutQuad(float t) {
		return t * (2.0f - t);
	}

	// イーズインアウト（加速→減速）
	static float EaseInOutQuad(float t) {
		return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
	}

	// なめらかなリズム感のイージング（加速→減速）
	static float easeInOutSine(float t) {
		return -(cosf((float)M_PI * t) - 1.0f) / 2.0f;
	}

	// イーズイン（強めの加速）
	static float EaseInCubic(float t) {
		return t * t * t;
	}

	// イーズアウト（強めの減速）
	static float EaseOutCubic(float t) {
		float p = t - 1.0f;
		return p * p * p + 1.0f;
	}

	// イーズインアウト（強めの加速→減速）
	static float EaseInOutCubic(float t) {
		return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
	}

	// バウンス（跳ねる）
	static float EaseOutBounce(float t) {
		if (t < (1 / 2.75f)) {
			return 7.5625f * t * t;
		}
		else if (t < (2 / 2.75f)) {
			t -= 1.5f / 2.75f;
			return 7.5625f * t * t + 0.75f;
		}
		else if (t < (2.5f / 2.75f)) {
			t -= 2.25f / 2.75f;
			return 7.5625f * t * t + 0.9375f;
		}
		else {
			t -= 2.625f / 2.75f;
			return 7.5625f * t * t + 0.984375f;
		}
	}

	static float EaseOutBack(float t, float overshoot = 1.70158f) {
		t = std::clamp(t, 0.0f, 1.0f);
		// Robert Penner の EaseOutBack 公式:
		// f(t) = 1 + (t-1)^3 * ((s+1)*(t-1) + s)
		float s = overshoot;
		t -= 1.0f;
		return 1.0f + (t * t * ((s + 1.0f) * t + s));
	}

	static float EaseOutBackRepeat(float t, float overshoot = 1.70158f) {
		// 0～1→0 三角波化
		float tri = static_cast<float>(t - std::floor(t)); // std::floor を使う
		tri = (tri <= 0.5f) ? (tri * 2.0f) : (2.0f - tri * 2.0f);
		return EaseOutBack(tri, overshoot);
	}

	static float EaseOutElastic(float t) {
		if (t == 0.0f) return 0.0f;
		if (t == 1.0f) return 1.0f;
		float p = 0.3f;
		float s = p / 4.0f;
		return powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * (float)M_PI) / p) + 1.0f;
	}
};