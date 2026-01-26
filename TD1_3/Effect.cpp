#include "Effect.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <Novice.h>

// ========== ColorRGBA 実装 ==========
ColorRGBA ColorRGBA::FromUInt(unsigned int color) {
	ColorRGBA rgba;
	rgba.a = ((color >> 0) & 0xFF) / 255.0f;
	rgba.b = ((color >> 8) & 0xFF) / 255.0f;
	rgba.g = ((color >> 16) & 0xFF) / 255.0f;
	rgba.r = ((color >> 24) & 0xFF) / 255.0f;
	return rgba;
}

unsigned int ColorRGBA::ToUInt() const {
	unsigned int r_int = static_cast<unsigned int>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
	unsigned int g_int = static_cast<unsigned int>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
	unsigned int b_int = static_cast<unsigned int>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
	unsigned int a_int = static_cast<unsigned int>(std::clamp(a, 0.0f, 1.0f) * 255.0f);
	return (r_int << 24) | (g_int << 16) | (b_int << 8) | a_int;
}

ColorRGBA ColorRGBA::Lerp(const ColorRGBA& start, const ColorRGBA& end, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return ColorRGBA(
		start.r + (end.r - start.r) * t,
		start.g + (end.g - start.g) * t,
		start.b + (end.b - start.b) * t,
		start.a + (end.a - start.a) * t
	);
}

ColorRGBA ColorRGBA::Multiply(const ColorRGBA& a, const ColorRGBA& b) {
	return ColorRGBA(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
}

ColorRGBA ColorRGBA::Add(const ColorRGBA& a, const ColorRGBA& b) {
	return ColorRGBA(
		(std::min)(a.r + b.r, 1.0f),
		(std::min)(a.g + b.g, 1.0f),
		(std::min)(a.b + b.b, 1.0f),
		(std::min)(a.a + b.a, 1.0f)
	);
}

Effect::Effect() {
	// フェード効果の初期化（不透明な白）
	fadeEffect_.currentColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	fadeEffect_.startColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	fadeEffect_.targetColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
}

// ========== Effect 更新 ==========
void Effect::Update(float deltaTime) {
	UpdateShake(deltaTime);
	UpdateRotation(deltaTime);
	UpdateFade(deltaTime);
	UpdateFlash(deltaTime);
	UpdateScale(deltaTime);
	UpdateWobble(deltaTime);
	UpdateSquash(deltaTime);

	// フラッシュ点滅の更新
	if (flashBlink_.isActive) {
		flashBlink_.flashTimer += deltaTime / 60.0f;

		if (flashBlink_.flashTimer >= flashBlink_.flashDuration) {
			flashBlink_.flashTimer -= flashBlink_.flashDuration;
			flashBlink_.isFlashOn = !flashBlink_.isFlashOn;

			if (!flashBlink_.isFlashOn) {
				flashBlink_.flashCount--;
				if (flashBlink_.flashCount <= 0) {
					StopFlashBlink();
				}
			}
		}
	}
}

// ========== シェイク ==========
void Effect::StartShake(float intensity, float duration) {
	shakeEffect_.isActive = true;
	shakeEffect_.intensity = intensity;
	shakeEffect_.duration = duration;
	shakeEffect_.elapsed = 0.0f;
	shakeEffect_.continuous = false;
}

void Effect::StartShakeContinuous(float intensity) {
	shakeEffect_.isActive = true;
	shakeEffect_.intensity = intensity;
	shakeEffect_.continuous = true;
	shakeEffect_.elapsed = 0.0f;
}

void Effect::StopShake() {
	shakeEffect_.isActive = false;
	shakeEffect_.offset = { 0.0f, 0.0f };
}

void Effect::UpdateShake(float deltaTime) {
	if (!shakeEffect_.isActive) return;

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-shakeEffect_.intensity, shakeEffect_.intensity);

	shakeEffect_.offset.x = dist(gen);
	shakeEffect_.offset.y = dist(gen);

	if (!shakeEffect_.continuous) {
		shakeEffect_.elapsed += deltaTime / 60.0f;
		if (shakeEffect_.elapsed >= shakeEffect_.duration) {
			StopShake();
		}
	}
}

// ========== 回転 ==========
void Effect::StartRotation(float speed, float duration) {
	rotationEffect_.isActive = true;
	rotationEffect_.speed = speed;
	rotationEffect_.duration = duration;
	rotationEffect_.elapsed = 0.0f;
	rotationEffect_.continuous = false;
}

void Effect::StartRotationContinuous(float speed) {
	rotationEffect_.isActive = true;
	rotationEffect_.speed = speed;
	rotationEffect_.continuous = true;
	rotationEffect_.elapsed = 0.0f;
}

void Effect::StartWobble(float angle, float speed) {
	wobbleEffect_.isActive = true;
	wobbleEffect_.angle = angle;
	wobbleEffect_.speed = speed;
	wobbleEffect_.elapsed = 0.0f;
}

void Effect::StopRotation() {
	rotationEffect_.isActive = false;
	rotationEffect_.accumulatedAngle = 0.0f;
	wobbleEffect_.isActive = false;
}

void Effect::UpdateRotation(float deltaTime) {
	if (!rotationEffect_.isActive) return;

	rotationEffect_.accumulatedAngle += rotationEffect_.speed * deltaTime / 60.0f;

	if (!rotationEffect_.continuous) {
		rotationEffect_.elapsed += deltaTime / 60.0f;
		if (rotationEffect_.elapsed >= rotationEffect_.duration) {
			StopRotation();
		}
	}
}

void Effect::UpdateWobble(float deltaTime) {
	if (!wobbleEffect_.isActive) return;
	wobbleEffect_.elapsed += deltaTime / 60.0f;
}

// ========== フェード ==========
void Effect::StartFade(float targetAlpha, float duration) {
	fadeEffect_.isActive = true;
	fadeEffect_.duration = duration;
	fadeEffect_.elapsed = 0.0f;

	// 初期化されていない場合は白・不透明で初期化
	if (!fadeEffect_.isActive && fadeEffect_.currentColor.a == 0.0f) {
		fadeEffect_.currentColor = ColorRGBA::White();
	}

	fadeEffect_.startColor = fadeEffect_.currentColor;
	fadeEffect_.targetColor = fadeEffect_.currentColor;
	fadeEffect_.targetColor.a = targetAlpha;
}

void Effect::StartColorTransition(const ColorRGBA& targetColor, float duration) {
	fadeEffect_.isActive = true;
	fadeEffect_.duration = duration;
	fadeEffect_.elapsed = 0.0f;

	// 初期化されていない場合は白・不透明で初期化
	if (fadeEffect_.currentColor.a == 0.0f && fadeEffect_.currentColor.r == 0.0f) {
		fadeEffect_.currentColor = ColorRGBA::White();
	}

	fadeEffect_.startColor = fadeEffect_.currentColor;
	fadeEffect_.targetColor = targetColor;
}

void Effect::StartFlash(const ColorRGBA& flashColor, float duration, float intensity) {
	flashEffect_.isActive = true;
	flashEffect_.flashColor = flashColor;
	flashEffect_.duration = duration;
	flashEffect_.elapsed = 0.0f;
	flashEffect_.intensity = intensity;
}

void Effect::StopFade() {
	fadeEffect_.isActive = false;
}

void Effect::UpdateFade(float deltaTime) {
	if (!fadeEffect_.isActive) return;

	fadeEffect_.elapsed += deltaTime / 60.0f;
	float t = fadeEffect_.elapsed / fadeEffect_.duration;

	if (t >= 1.0f) {
		t = 1.0f;
		fadeEffect_.currentColor = fadeEffect_.targetColor;
		fadeEffect_.isActive = false;
	}
	else {
		fadeEffect_.currentColor = ColorRGBA::Lerp(fadeEffect_.startColor, fadeEffect_.targetColor, t);
	}
}

void Effect::UpdateFlash(float deltaTime) {
	if (!flashEffect_.isActive) return;

	flashEffect_.elapsed += deltaTime / 60.0f;
	float t = flashEffect_.elapsed / flashEffect_.duration;

	if (t >= 1.0f) {
		flashEffect_.isActive = false;
	}
}

// ========== スケール ==========
void Effect::StartPulse(float minScale, float maxScale, float speed, bool continuous) {
	scaleEffect_.isActive = true;
	scaleEffect_.minScale = minScale;
	scaleEffect_.maxScale = maxScale;
	scaleEffect_.speed = speed;
	scaleEffect_.continuous = continuous;
	scaleEffect_.expanding = true;
	scaleEffect_.elapsed = 0.0f;
}

void Effect::StartSquash(const Vector2& targetScale, float duration) {
	squashEffect_.isActive = true;
	squashEffect_.targetScale = targetScale;
	squashEffect_.duration = duration;
	squashEffect_.elapsed = 0.0f;
	squashEffect_.currentScale = { 1.0f, 1.0f };
}

void Effect::StopScale() {
	scaleEffect_.isActive = false;
	squashEffect_.isActive = false;
}

void Effect::UpdateScale(float deltaTime) {
	if (!scaleEffect_.isActive) return;

	scaleEffect_.elapsed += deltaTime / 60.0f;

	// サイン波で 0.0 ~ 1.0 の範囲で振動
	float t = std::sin(scaleEffect_.elapsed * scaleEffect_.speed) * 0.5f + 0.5f;

	// minScale と maxScale の間で補間
	scaleEffect_.currentScale = scaleEffect_.minScale + (scaleEffect_.maxScale - scaleEffect_.minScale) * t;

	// 非連続モードの場合、カウントを管理
	if (!scaleEffect_.continuous) {
		// 1周期 = 2π / speed
		float period = (2.0f * 3.14159265f) / scaleEffect_.speed;
		if (scaleEffect_.elapsed >= period * (scaleEffect_.currentCount + 1)) {
			scaleEffect_.currentCount++;
			if (scaleEffect_.currentCount >= scaleEffect_.maxCount) {
				StopScale();
			}
		}
	}
}
void Effect::UpdateSquash(float deltaTime) {
	if (!squashEffect_.isActive) return;

	squashEffect_.elapsed += deltaTime / 60.0f;
	float t = squashEffect_.elapsed / squashEffect_.duration;

	if (t >= 1.0f) {
		t = 1.0f;
		squashEffect_.isActive = false;
	}

	squashEffect_.currentScale.x = 1.0f + (squashEffect_.targetScale.x - 1.0f) * (1.0f - t);
	squashEffect_.currentScale.y = 1.0f + (squashEffect_.targetScale.y - 1.0f) * (1.0f - t);
}

// ========== 複合エフェクト ==========
void Effect::StartHitEffect() {
	StartFlash(ColorRGBA::White(), 0.3f, 4.8f);
	StartShake(5.0f, 0.2f);
}

void Effect::StartDeathEffect() {
	StartFadeOut(0.5f);
	StartPulse(1.0f, 0.0f, 2.0f, false);
}

void Effect::StartSpawnEffect() {
	StartFadeIn(0.3f);
	StartPulse(0.0f, 1.0f, 3.0f, false);
}

// ========== 結果取得 ==========
Vector2 Effect::GetPositionOffset() const {
	return shakeEffect_.offset;
}

Vector2 Effect::GetScaleMultiplier() const {
	Vector2 scale = { 1.0f, 1.0f };

	if (scaleEffect_.isActive) {
		scale.x *= scaleEffect_.currentScale;
		scale.y *= scaleEffect_.currentScale;
	}

	if (squashEffect_.isActive) {
		scale.x *= squashEffect_.currentScale.x;
		scale.y *= squashEffect_.currentScale.y;
	}

	return scale;
}

float Effect::GetRotationOffset() const {
	float rotation = 0.0f;

	if (rotationEffect_.isActive) {
		rotation += rotationEffect_.accumulatedAngle;
	}

	if (wobbleEffect_.isActive) {
		rotation += std::sin(wobbleEffect_.elapsed * wobbleEffect_.speed) * wobbleEffect_.angle;
	}

	return rotation;
}

ColorRGBA Effect::GetColorModifier(const ColorRGBA& baseColor) const {
	ColorRGBA result = baseColor;

	// フェード適用
	if (fadeEffect_.isActive) {
		result = ColorRGBA::Multiply(result, fadeEffect_.currentColor);
	}

	// フラッシュ適用
	if (flashEffect_.isActive) {
		float t = 1.0f - (flashEffect_.elapsed / flashEffect_.duration);
		ColorRGBA flash = ColorRGBA::Lerp(baseColor, flashEffect_.flashColor, t * flashEffect_.intensity);
		result = flash;
	}

	return result;
}

unsigned int Effect::GetFinalColor(unsigned int baseColor) const {
	ColorRGBA base = ColorRGBA::FromUInt(baseColor);
	ColorRGBA modified = GetColorModifier(base);
	return modified.ToUInt();
}

bool Effect::IsAnyActive() const {
	return shakeEffect_.isActive || rotationEffect_.isActive ||
		fadeEffect_.isActive || flashEffect_.isActive ||
		scaleEffect_.isActive || wobbleEffect_.isActive ||
		squashEffect_.isActive || IsFlashBlinking();
}

void Effect::StopAll() {
	StopShake();
	StopRotation();
	StopFade();
	StopScale();
	StopFlashBlink();

	// フェード色をリセット
	fadeEffect_.currentColor = ColorRGBA::White();
	fadeEffect_.startColor = ColorRGBA::White();
	fadeEffect_.targetColor = ColorRGBA::White();
}


void Effect::StartFlashBlink(unsigned int color, int count, float duration, BlendMode blend, unsigned int layer) {
	flashBlink_.isActive = true;
	flashBlink_.flashColor = color;
	flashBlink_.flashCount = count;
	flashBlink_.totalFlashCount = count;
	flashBlink_.flashDuration = duration;
	flashBlink_.flashTimer = 0.0f;
	flashBlink_.isFlashOn = true;
	flashBlink_.layer = layer;
	flashBlink_.blend = blend;
}

void Effect::StopFlashBlink() {
	flashBlink_.isActive = false;
	flashBlink_.flashCount = 0;
	flashBlink_.isFlashOn = false;
}


