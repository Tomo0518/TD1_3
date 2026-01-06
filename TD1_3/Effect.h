#pragma once
#include "Vector2.h"
#include <functional>

// ========== RGBA色管理 ==========
struct ColorRGBA {
	float r = 1.0f;  // 0.0 ~ 1.0
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	ColorRGBA() = default;
	ColorRGBA(float r, float g, float b, float a = 1.0f)
		: r(r), g(g), b(b), a(a) {
	}

	// unsigned int ⇔ RGBA 変換
	static ColorRGBA FromUInt(unsigned int color);
	unsigned int ToUInt() const;

	// 色操作
	static ColorRGBA Lerp(const ColorRGBA& start, const ColorRGBA& end, float t);
	static ColorRGBA Multiply(const ColorRGBA& a, const ColorRGBA& b);
	static ColorRGBA Add(const ColorRGBA& a, const ColorRGBA& b);

	// 便利なカラープリセット
	static ColorRGBA White() { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
	static ColorRGBA Black() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
	static ColorRGBA Red() { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
	static ColorRGBA Green() { return { 0.0f, 1.0f, 0.0f, 1.0f }; }
	static ColorRGBA Blue() { return { 0.0f, 0.0f, 1.0f, 1.0f }; }
	static ColorRGBA Yellow() { return { 1.0f, 1.0f, 0.0f, 1.0f }; }
	static ColorRGBA Transparent() { return { 1.0f, 1.0f, 1.0f, 0.0f }; }
};

// ========== エフェクト構造体 ==========
struct ShakeEffect {
	bool isActive = false;
	float intensity = 0.0f;
	float duration = 0.0f;
	float elapsed = 0.0f;
	bool continuous = false;
	Vector2 offset = { 0.0f, 0.0f };
};

struct RotationEffect {
	bool isActive = false;
	float speed = 0.0f;
	float duration = 0.0f;
	float elapsed = 0.0f;
	bool continuous = false;
	float accumulatedAngle = 0.0f;
};

struct FadeEffect {
	bool isActive = false;
	float duration = 0.0f;
	float elapsed = 0.0f;
	ColorRGBA startColor;
	ColorRGBA targetColor;
	ColorRGBA currentColor;
};

struct FlashEffect {
	bool isActive = false;
	float duration = 0.0f;
	float elapsed = 0.0f;
	ColorRGBA flashColor;
	float intensity = 1.0f;  // 0.0 ~ 1.0
};

struct ScaleEffect {
	bool isActive = false;
	float minScale = 1.0f;
	float maxScale = 1.0f;
	float speed = 1.0f;
	float elapsed = 0.0f;
	bool continuous = false;
	bool expanding = true;
	int maxCount = 0;
	int currentCount = 0;
	float currentScale = 1.0f;
};

struct WobbleEffect {
	bool isActive = false;
	float angle = 0.0f;        // 揺れの角度
	float speed = 1.0f;        // 揺れの速さ
	float elapsed = 0.0f;
	bool continuous = true;
};

struct SquashEffect {
	bool isActive = false;
	Vector2 targetScale = { 1.0f, 1.0f };
	float duration = 0.0f;
	float elapsed = 0.0f;
	Vector2 currentScale = { 1.0f, 1.0f };
};

// ========== Effect クラス ==========
class Effect {
public:
	Effect();
	~Effect() = default;

	// ========== 更新 ==========
	void Update(float deltaTime);

	// ========== 位置系エフェクト ==========
	void StartShake(float intensity, float duration);
	void StartShakeContinuous(float intensity);
	void StopShake();

	// ========== スケール系エフェクト ==========
	void StartPulse(float minScale, float maxScale, float speed, bool continuous = true);
	void StartSquash(const Vector2& targetScale, float duration);
	void StopScale();

	// ========== 回転系エフェクト ==========
	void StartRotation(float speed, float duration);
	void StartRotationContinuous(float speed);
	void StartWobble(float angle, float speed);
	void StopRotation();

	// ========== 色/透明度系エフェクト ==========
	void StartFade(float targetAlpha, float duration);
	void StartFadeOut(float duration) { StartFade(0.0f, duration); }
	void StartFadeIn(float duration) { StartFade(1.0f, duration); }

	void StartColorTransition(const ColorRGBA& targetColor, float duration);
	void StartFlash(const ColorRGBA& flashColor, float duration, float intensity = 1.0f);
	void StopFade();

	// ========== 複合エフェクト（プリセット） ==========
	void StartHitEffect();      // ダメージ演出（白フラッシュ + シェイク）
	void StartDeathEffect();    // 消滅演出（フェードアウト + 縮小）
	void StartSpawnEffect();    // 出現演出（フェードイン + 拡大）

	// ========== エフェクト適用結果を取得 ==========
	Vector2 GetPositionOffset() const;
	Vector2 GetScaleMultiplier() const;
	float GetRotationOffset() const;
	ColorRGBA GetColorModifier(const ColorRGBA& baseColor) const;
	unsigned int GetFinalColor(unsigned int baseColor) const;

	// ========== 状態確認 ==========
	bool IsAnyActive() const;
	bool IsShakeActive() const { return shakeEffect_.isActive; }
	bool IsRotationActive() const { return rotationEffect_.isActive; }
	bool IsFadeActive() const { return fadeEffect_.isActive; }
	bool IsScaleActive() const { return scaleEffect_.isActive; }

	// ========== リセット ==========
	void StopAll();

private:
	ShakeEffect shakeEffect_;
	RotationEffect rotationEffect_;
	FadeEffect fadeEffect_;
	FlashEffect flashEffect_;
	ScaleEffect scaleEffect_;
	WobbleEffect wobbleEffect_;
	SquashEffect squashEffect_;

	// 内部更新処理
	void UpdateShake(float deltaTime);
	void UpdateRotation(float deltaTime);
	void UpdateFade(float deltaTime);
	void UpdateFlash(float deltaTime);
	void UpdateScale(float deltaTime);
	void UpdateWobble(float deltaTime);
	void UpdateSquash(float deltaTime);
};