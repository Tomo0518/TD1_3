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
		delete drawComp_;
		drawComp_ = nullptr;
		//Initialize();
	}
	~Button() {
		if (onComp_ != drawComp_) delete onComp_;
		if (offComp_ != drawComp_) delete offComp_;
	}
	virtual void Initialize() override {
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = true;
		collider_.size = { 100.f, 100.f };
		collider_.offset = { 0.f, 0.f };
		if (isSwitch_) {
			onComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_On_Switch));
			offComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_Off_Switch));
		}
		else {
			onComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_On));
			offComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_Off));
		}		
		onComp_->Initialize();
		offComp_->Initialize();
		drawComp_ = offComp_;


	}

	virtual void BackToDefault() {
		SetPressed(false);
	}

	virtual void CheckPlayerPress() {
		FindPlayer();
		if (playerRef_) {
			Usagi* player = dynamic_cast<Usagi*>(playerRef_);

			if (player->GetStatus().currentHP <= 0) {
				BackToDefault();
				return;
			}

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

class Button3 : public Button {
public:
	Button3() {
		ButtonID_ = 3;
		isSwitch_ = true;
	}
};

class Button4 : public Button {
public:
	Button4() {
		ButtonID_ = 4;
	}
};

class Button5 : public Button {
public:
	Button5() {
		ButtonID_ = 5;
		isSwitch_ = true;
	}
};

class Button6 : public Button {
public:
	Button6() {
		ButtonID_ = 6;
	}
};

class EnemyEvent : public Button {
protected:
	std::vector<GameObject2D*> spawnedEnemies_;
	bool Spawned_ = false;
	float activeRange_ = 500.f;
public:
	EnemyEvent() {
		ButtonID_ = 101;
		isSwitch_ = true;
	}

	void BackToDefault() override {
		SetPressed(true);
		// destroy all spawned enemies
		for (auto enemy : spawnedEnemies_) {
			if (enemy && !enemy->IsDead()) {
				enemy->Destroy();
			}
		}
		spawnedEnemies_.clear();
		Spawned_ = false;

	}

	void Initialize() override {
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = false;
		collider_.size = { 100.f, 100.f };
		collider_.offset = { 0.f, 0.f };
		onComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_On));
		offComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Button_Off));
		onComp_->Initialize();
		offComp_->Initialize();
		drawComp_ = offComp_;

		SetPressed(true);
		Spawned_ = false;
	}

	virtual void spawnEnemy() {
		auto e = manager_->Spawn<FatEnemy>(this, "Enemy");
		e->SetPosition({ transform_.translate.x + 200.f, transform_.translate.y + 100.f });
		spawnedEnemies_.push_back(e);
	}

	bool CheckAllDefeated() {
		// clear all null or dead enemies
		spawnedEnemies_.erase(
			std::remove_if(spawnedEnemies_.begin(), spawnedEnemies_.end(),
				[](GameObject2D* enemy) {
					return enemy == nullptr || enemy->IsDead();
				}),
			spawnedEnemies_.end());

		if (spawnedEnemies_.size() > 0) {
			return false;
		}

		return true;
	}

	virtual void ActivateEvent() {		
		spawnEnemy();
	}

	virtual void CheckPlayerPress() override {
		FindPlayer();
		if (playerRef_) {
			Usagi* player = dynamic_cast<Usagi*>(playerRef_);

			if (player->GetStatus().currentHP <= 0) {
				BackToDefault();
				return;
			}

			Vector2 BoomerangPos = player->GetFirstBoomerang()->GetPosition();

			Vector2 buttonPos = transform_.translate;
			float distance = Vector2::Length(Vector2::Subtract(BoomerangPos, buttonPos));
			if (distance < activeRange_ && !Spawned_) {
				ActivateEvent();
				SetPressed(false);
				Spawned_ = true;
			}
		}

		if (Spawned_) {
			if (CheckAllDefeated()) {
				SetPressed(true);
			}
		}
	}
};

class EnemyEvent2 : public EnemyEvent {
	public:
	EnemyEvent2() {
		ButtonID_ = 102;
		isSwitch_ = true;
		activeRange_ = 100.f;
	}

	void ActivateEvent() override {
		auto e1 = manager_->Spawn<AttackEnemy>(this, "Enemy");
		e1->SetPosition({ transform_.translate.x - 400.f, transform_.translate.y + 100.f });
		spawnedEnemies_.push_back(e1);
		auto e2 = manager_->Spawn<AttackEnemy>(this, "Enemy");
		e2->SetPosition({ transform_.translate.x - 200.f, transform_.translate.y + 100.f });
		spawnedEnemies_.push_back(e2);
	}
};





// endButton
class EndButton : public Button {
protected:
	float activeRange_ = 100.f;
public:
	EndButton() {
		ButtonID_ = 999;
		isSwitch_ = true;
	}

	void Initialize() override {
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = true;
		collider_.size = { 100.f, 100.f };
		collider_.offset = { 0.f, 0.f };
		onComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Coin), 4, 1, 4, 5.f, true);
		offComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Coin), 4, 1, 4, 5.f, true);
		onComp_->Initialize();
		offComp_->Initialize();
		drawComp_ = offComp_;

		SetPressed(false);
	}

	virtual void CheckPlayerPress() override {
		FindPlayer();
		if (playerRef_) {
			Usagi* player = dynamic_cast<Usagi*>(playerRef_);

			if (player->GetStatus().currentHP <= 0) {
				BackToDefault();
				return;
			}

			Vector2 BoomerangPos = player->GetFirstBoomerang()->GetPosition();

			Vector2 buttonPos = transform_.translate;
			float distance = Vector2::Length(Vector2::Subtract(BoomerangPos, buttonPos));
			if (distance < activeRange_ ) {				
				SetPressed(true);
			}
		}

	}
};

