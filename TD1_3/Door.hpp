#pragma once
#include "Button.hpp"

class Door : public PhysicsObject {
protected:
	int targetButtonID_ = 0;
	bool isOpen_ = false;
	DrawComponent2D* openComp_ = nullptr;
	DrawComponent2D* closedComp_ = nullptr;
public:
	Door() {
		drawComp_ = nullptr;
		//Initialize();
	}
	~Door() {
		if (openComp_ != drawComp_) delete openComp_;
		if (closedComp_ != drawComp_) delete closedComp_;
	}
	void Initialize() override {
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = true;
		collider_.size = { 64.f, 256.f };
		collider_.offset = { 0.f, 0.f };
		openComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Door_Open));
		closedComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Door_Closed));
		openComp_->Initialize();
		closedComp_->Initialize();
		drawComp_ = closedComp_;

		isGravityEnabled_ = false;
	}

	void Update(float deltaTime) override {
		//PhysicsObject::Update(deltaTime);
		CheckOpen();
		UpdateDrawComponent(deltaTime);
	}

	void CheckOpen() {
		// Find all buttons in the manager
		auto buttons = manager_->GetObjectsByTag("Button");
		bool allPressed = true;
		for (auto& buttonObj : buttons) {
			Button* button = dynamic_cast<Button*>(buttonObj);
			if (button && button->GetButtonID() == targetButtonID_) {
				if (!button->IsPressed()) {
					allPressed = false;
					break;
				}
			}
		}
		SetOpen(allPressed);
	}


	void SetOpen(bool isOpen) {
		isOpen_ = isOpen;
		if (isOpen_) {
			drawComp_ = openComp_;
			collider_.canCollide = false;
		}
		else {
			drawComp_ = closedComp_;
			collider_.canCollide = true;
		}
	}

	virtual int OnCollision(GameObject2D* other) override {
		if (other->GetInfo().tag == "Player" || other->GetInfo().tag == "Enemy") {
			Vector2 knockbackDir = Vector2::Subtract(transform_.translate, other->GetTransform().translate);
			knockbackDir = Vector2::Normalize(knockbackDir) * -1.f;
			other->GetRigidbody().acceleration.x += knockbackDir.x * 7.5f;
		}
		return 0;
	}
};


class Door1 : public Door {
public:
	Door1() {
		targetButtonID_ = 1;
	}
};

