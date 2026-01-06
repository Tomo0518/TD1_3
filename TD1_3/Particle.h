#pragma once
#include "Vector2.h"
#include "ParticleEnum.h"
#include "Novice.h"

class Particle {
public:
	Particle() = default;
	~Particle() = default;

	void Initialize(
		const Vector2& pos,
		const Vector2& vel,
		const Vector2& acc,
		int life,
		int texHandle,
		float scaleStart,
		float scaleEnd,
		unsigned int colorStart,
		unsigned int colorEnd,
		float rotation = 0.0f,
		float rotationSpeed = 0.0f,
		BlendMode blendMode = kBlendModeNormal,
		float drawSize = 0.0f,
		bool useAnimation = false,
		int divX = 1,
		int divY = 1,
		int totalFrames = 1,
		float animSpeed = 0.0f
	);

	void Update(float deltaTime);

	// 地面衝突判定
	void CheckGroundCollision(float groundY);

	bool IsAlive() const { return isAlive_; }
	ParticleType GetType() const { return type_; }
	void SetType(ParticleType type) { type_ = type; }
	void SetBehavior(ParticleBehavior behavior) { behavior_ = behavior; }

	void SetHomingTarget(const Vector2* target, float strength) {
		homingTarget_ = target;
		homingStrength_ = strength;
	}

	Vector2 GetPosition() const { return position_; }
	float GetRotation() const { return rotation_; }
	float GetCurrentScale() const { return currentScale_; }
	unsigned int GetCurrentColor() const { return currentColor_; }
	int GetTextureHandle() const { return textureHandle_; }
	BlendMode GetBlendMode() const { return blendMode_; }
	float GetDrawSize() const { return drawSize_; }

	bool UseAnimation() const { return useAnimation_; }
	int GetDivX() const { return divX_; }
	int GetDivY() const { return divY_; }
	int GetCurrentFrame() const { return currentFrame_; }

private:
	ParticleType type_ = ParticleType::Explosion;
	ParticleBehavior behavior_ = ParticleBehavior::Physics;

	Vector2 position_ = { 0.0f, 0.0f };
	Vector2 velocity_ = { 0.0f, 0.0f };
	Vector2 acceleration_ = { 0.0f, 0.0f };

	float rotation_ = 0.0f;
	float rotationSpeed_ = 0.0f;

	float scaleStart_ = 1.0f;
	float scaleEnd_ = 1.0f;
	float currentScale_ = 1.0f;

	unsigned int colorStart_ = 0xFFFFFFFF;
	unsigned int colorEnd_ = 0xFFFFFFFF;
	unsigned int currentColor_ = 0xFFFFFFFF;

	int lifeTimer_ = 0;
	int maxLife_ = 0;
	bool isAlive_ = false;

	int textureHandle_ = -1;
	BlendMode blendMode_ = kBlendModeNormal;
	float drawSize_ = 0.0f;

	bool useAnimation_ = false;
	int divX_ = 1;
	int divY_ = 1;
	int totalFrames_ = 1;
	float animSpeed_ = 0.0f;
	float animTimer_ = 0.0f;
	int currentFrame_ = 0;

	const Vector2* homingTarget_ = nullptr;
	float homingStrength_ = 0.0f;

	float Lerp(float start, float end, float t) const;
	unsigned int LerpColor(unsigned int start, unsigned int end, float t) const;
};