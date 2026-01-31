#pragma once

#include "PhysicsObject.hpp"

class Star : public PhysicsObject {
private:
	float lifeTime_;
	float moveSpeed_;

public:
	Star() {

		lifeTime_ = 300.0f; // 5 seconds at 60fps

		delete drawComp_;
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Star_idle), 4, 1, 4, 5.f, true);
	}

	void Initialize() override {
		// 1. Important: Let the parent class reset/init everything first
		PhysicsObject::Initialize();

		// 2. Now set your custom physics values
		info_.isActive = true;
		info_.isVisible = true;
		isGravityEnabled_ = false;

		rigidbody_.deceleration = { 0.85f, 0.85f }; // No deceleration

		float angle = (rand() % 100 / 100.0f) * 3.14159f / 2.0f + 3.14159f / 4.0f; // 45 to 135 degrees
		rigidbody_.velocity = { cosf(angle) *(10.f+ (rand() % 100 / 100.0f) *20.0f), sinf(angle) * (10.f + (rand() % 100 / 100.0f) * 20.0f) };
	}

	void Update(float deltaTime = 1.0f) override {
		// Move
		transform_.translate += rigidbody_.velocity;
#ifdef _DEBUG
		Novice::ScreenPrintf(0,100, "velocity: %.2f, %.2f", rigidbody_.velocity.x, rigidbody_.velocity.y);
#endif
		//rigidbody_.AddForce({ 0.0f, 1.f }); // Simulate slight gravity
		// Decelerate velocity
		//rigidbody_.velocity *= 0.95f;

		lifeTime_ -= deltaTime;
		if (lifeTime_ <= 0) {
			info_.isActive = false;
			Destroy();
		}

		PhysicsObject::Update(deltaTime);
	}

	int OnCollision(GameObject2D* other) override {
		if (other->GetInfo().tag == "Player") {
			info_.isActive = false;
			Destroy();
		}
		return 0;
	}

};
