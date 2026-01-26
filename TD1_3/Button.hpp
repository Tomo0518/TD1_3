#pragma once
#include "PhysicsObject.hpp"
#include "Usagi.hpp"


class Button : public PhysicsObject {
protected:
	bool isPressed_ = false;
	int ButtonID_ = 0;
	bool isSwitch_ = false;
	DrawComponent2D* onComp_ = nullptr;
	DrawComponent2D* offComp_ = nullptr;

	GameObject2D* playerRef_ = nullptr;
public:
	Button() {
		drawComp_ = nullptr;
		//Initialize();
	}
	~Button() {
		if (onComp_ != drawComp_) delete onComp_;
		if (offComp_ != drawComp_) delete offComp_;
	}
	void Initialize() override {
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = true;
		collider_.size = { 100.f, 100.f };
		collider_.offset = { 0.f, 0.f };
		onComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_On));
		offComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_Off));
		onComp_->Initialize();
		offComp_->Initialize();
		drawComp_ = offComp_;
	}

	void CheckPlayerPress() {
		FindPlayer();
		if (playerRef_) {
			Usagi* player = dynamic_cast<Usagi*>(playerRef_);

			Vector2 BoomerangPos = player->GetFirstBoomerang()->GetPosition();

			Vector2 buttonPos = transform_.translate;
			float distance = Vector2::Length(Vector2::Subtract(BoomerangPos, buttonPos));
			if (distance < 64.f) {
				SetPressed(true);
			}
			else {
				if (isSwitch_) return;

				SetPressed(false);

			}
		}
	}

	void FindPlayer() {
		GameObject2D* players = manager_->GetPlayerObject();
		if (players) {
			playerRef_ = players;
		}
	}

	void Update(float deltaTime) override {
		CheckPlayerPress();
		PhysicsObject::Update(deltaTime);
	}

	void SetPressed(bool pressed) {
		isPressed_ = pressed;
		if (isPressed_) {
			drawComp_ = onComp_;
		}
		else {
			drawComp_ = offComp_;
		}
	}

	bool IsPressed() const {
		return isPressed_;
	}

	int GetButtonID() const {
		return ButtonID_;
	}
};

class Button1 : public Button {
public:
	Button1() {
		ButtonID_ = 1;
	}
};

class Button2 : public Button {
public:
	Button2() {
		ButtonID_ = 2;
		isSwitch_ = true;
	}
};
