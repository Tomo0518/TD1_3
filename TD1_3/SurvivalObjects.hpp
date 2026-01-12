#pragma once
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "GameObject2D.h"
#include "InputManager.h"
#include "WindowSize.h"

// forward（循環回避）
class SurvivalGameObjectManager;

class SurvivalPlayer : public GameObject2D {
public:
	explicit SurvivalPlayer(InputManager* input)
		: input_(input) {
		drawComp_->SetDrawSize(radius_ * 2, radius_ * 2);
		drawComp_->SetAnchorPoint({ 0.5f, 0.5f });
		drawComp_->SetBaseColor(0xAAAAFFFF);
		transform_.translate = { kWindowWidth / 2.0f, kWindowHeight / 2.0f };
	}

	void Update(float dt) override {
		Vector2 moveDir = { 0,0 };
		if (input_->PressKey(DIK_W)) moveDir.y -= 1.0f;
		if (input_->PressKey(DIK_S)) moveDir.y += 1.0f;
		if (input_->PressKey(DIK_A)) moveDir.x -= 1.0f;
		if (input_->PressKey(DIK_D)) moveDir.x += 1.0f;
		if (input_->GetInputMode() == InputMode::Gamepad) {
			moveDir = input_->GetPad()->GetLeftStick();
		}

		if (Vector2::Length(moveDir) > 0.0f) {
			moveDir = Vector2::Normalize(moveDir);
			transform_.translate += moveDir * speed_ * dt;
		}

		if (invincibilityTimer_ > 0.0f) {
			invincibilityTimer_ -= dt;
			drawComp_->SetBaseColor(((int)(invincibilityTimer_ * 20) % 2 == 0) ? 0x8888FFFF : 0xAAAAFFFF);
		}else {
			drawComp_->SetBaseColor(0xAAAAFFFF);
		}

		GameObject2D::Update(dt);
	}

	void OnDamage() {
		if (invincibilityTimer_ > 0.0f) return;
		hp_--;
		invincibilityTimer_ = 1.0f;
		drawComp_->StartShake(5.0f, 0.5f);
		drawComp_->StartFlash({ 1.0f, 0.0f, 0.0f, 1.0f }, 0.2f);
	}

	int GetHP() const { return hp_; }
	float GetRadius() const { return radius_; }

	const Vector2* GetPositionPtr() const { return &transform_.translate; }

private:
	InputManager* input_ = nullptr;

	float speed_ = 300.0f;
	float radius_ = 16.0f;
	int hp_ = 5;
	float invincibilityTimer_ = 0.0f;
};

enum class EnemyType { Normal, Tank };

class SurvivalEnemy : public GameObject2D {
public:
	SurvivalEnemy(Vector2 startPos, EnemyType type, std::shared_ptr<SurvivalPlayer> target)
		: target_(std::move(target))
		, type_(type) {
		transform_.translate = startPos;

		if (type_ == EnemyType::Tank) {
			hp_ = 15;
			radius_ = 24.0f;
			drawComp_->SetBaseColor(0x882222FF);
		}
		else {
			hp_ = 2;
			radius_ = 12.0f;
			drawComp_->SetBaseColor(0xFF4444FF);
		}

		drawComp_->SetDrawSize(radius_ * 2, radius_ * 2);
		drawComp_->SetAnchorPoint({ 0.5f, 0.5f });
	}

	void Update(float dt) override {
		if (hitInvincibility_ > 0.0f) hitInvincibility_ -= dt;

		if (knockbackDuration_ > 0.0f) {
			knockbackDuration_ -= dt;
			transform_.translate += knockbackVel_ * dt;
			knockbackVel_ *= 0.9f;
			drawComp_->SetBaseColor(0xFFFFFFFF);
		}
		else {
			drawComp_->SetBaseColor((type_ == EnemyType::Tank) ? 0x882222FF : 0xFF4444FF);

			if (target_) {
				Vector2 toPlayer = target_->GetPosition() - transform_.translate;
				if (Vector2::Length(toPlayer) > 1.0f) {
					Vector2 dir = Vector2::Normalize(toPlayer);
					const float speed = (type_ == EnemyType::Tank) ? 40.0f : 100.0f;
					transform_.translate += dir * speed * dt;
				}
			}
		}

		GameObject2D::Update(dt);
	}

	void OnHit(int damage, Vector2 knockbackDir, float knockbackPower) {
		if (hitInvincibility_ > 0.0f) return;

		hp_ -= damage;
		knockbackVel_ = knockbackDir * knockbackPower;
		knockbackDuration_ = 0.2f + (knockbackPower / 2000.0f);
		drawComp_->StartSquash({ 1.3f, 0.7f }, 0.1f);

		if (hp_ <= 0) {
			GetInfo().isActive = false;
			Destroy();
		}
	}

	void PushBack(Vector2 dir, float dist) {
		transform_.translate += dir * dist;
	}

	EnemyType GetType() const { return type_; }
	float GetRadius() const { return radius_; }

private:
	std::shared_ptr<SurvivalPlayer> target_;

	EnemyType type_;
	int hp_ = 0;
	float radius_ = 0.0f;

	Vector2 knockbackVel_ = { 0,0 };
	float knockbackDuration_ = 0.0f;
	float hitInvincibility_ = 0.0f;
};

class DebrisPiece : public GameObject2D {
public:
	DebrisPiece(int index, int totalCount, std::shared_ptr<SurvivalPlayer> anchor)
		: anchor_(std::move(anchor))
		, index_(index)
		, totalCount_(totalCount) {
		drawComp_->SetDrawSize(12.0f, 12.0f);
		drawComp_->SetAnchorPoint({ 0.5f, 0.5f });

		const float pi = 3.14159265f;
		angleOffset_ = (float)index_ / (float)totalCount_ * 2.0f * pi;
		distNoise_ = (float)(rand() % 40 - 20);
		selfRotSpeed_ = (float)(rand() % 100 - 50) / 10.0f;
	}

	void SetStateInfo(float currentRadius, float rotationAngle) {
		const float r = currentRadius + distNoise_;
		const float a = angleOffset_ + rotationAngle;
		Vector2 offset = { std::cos(a) * r, std::sin(a) * r };
		targetPos_ = anchor_->GetPosition() + offset;

		currentSelfRot_ += selfRotSpeed_ * 0.016f;
		drawComp_->SetRotation(a + currentSelfRot_);
	}

	void Update(float dt) override {
		const float lerpFactor = 8.0f * dt;
		Vector2 diff = targetPos_ - transform_.translate;
		transform_.translate += diff * lerpFactor;

		if (Vector2::Length(diff) > 800.0f) {
			transform_.translate = targetPos_;
		}

		GameObject2D::Update(dt);
	}

	Vector2 GetActualPosition() const { return transform_.translate; }

private:
	std::shared_ptr<SurvivalPlayer> anchor_;

	int index_ = 0;
	int totalCount_ = 0;

	float angleOffset_ = 0.0f;
	float distNoise_ = 0.0f;
	float selfRotSpeed_ = 0.0f;
	float currentSelfRot_ = 0.0f;

	Vector2 targetPos_ = { 0,0 };
};

class DebrisController : public GameObject2D {
public:
	DebrisController(SurvivalGameObjectManager* manager, InputManager* input, std::shared_ptr<SurvivalPlayer> anchor)
		: manager_(manager)
		, input_(input)
		, anchor_(std::move(anchor)) {
		currentRadius_ = minRadius_;

		const int count = 64;
		for (int i = 0; i < count; i++) {
			auto piece = std::make_shared<DebrisPiece>(i, count, anchor_);
			pieces_.push_back(piece);
			manager_->AddObject(piece, "Debris");
		}
	}

	void Update(float dt) override {
		float t = (currentRadius_ - minRadius_) / (maxRadius_ - minRadius_);
		t = std::clamp(t, 0.0f, 1.0f);
		float rotSpeed = 8.0f * (1.0f - t) + 1.0f * t;
		if (state_ == State::Contracting) rotSpeed = 15.0f;
		rotationAngle_ += rotSpeed * dt;

		const bool isSpace = input_->PressKey(DIK_SPACE) || input_->GetPad()->Press(Pad::Button::A);

		switch (state_) {
		case State::Idle:
			currentRadius_ = minRadius_;
			if (isSpace) { state_ = State::Expanding; isCritical_ = false; }
			break;

		case State::Expanding:
			currentRadius_ += 400.0f * dt;
			if (currentRadius_ >= maxRadius_) {
				currentRadius_ = maxRadius_;
				state_ = State::WaitMax;
				isCritical_ = true;
			}
			if (!isSpace) state_ = State::Contracting;
			break;

		case State::WaitMax:
			currentRadius_ = maxRadius_;
			if (!isSpace) state_ = State::Contracting;
			break;

		case State::Contracting: {
			float speed = 1500.0f;
			if (isCritical_) speed *= 1.2f;
			currentRadius_ -= speed * dt;
			if (currentRadius_ <= minRadius_) {
				currentRadius_ = minRadius_;
				state_ = State::Cooldown;
				cooldownTimer_ = isCritical_ ? 0.8f : 0.15f;
			}
		} break;

		case State::Cooldown:
			cooldownTimer_ -= dt;
			if (cooldownTimer_ <= 0.0f) state_ = State::Idle;
			break;
		}

		unsigned int color = 0x00FF00FF;
		if (state_ == State::Expanding) color = 0xFFFF00FF;
		if (state_ == State::WaitMax) color = isCritical_ ? 0xFFFFFFFF : 0xFFFF00FF;
		if (state_ == State::Contracting) color = 0xFF0000FF;
		if (state_ == State::Cooldown) color = 0x555555FF;

		for (auto& p : pieces_) {
			p->SetStateInfo(currentRadius_, rotationAngle_);
			p->GetDrawComponent()->SetBaseColor(color);
		}
	}

	float GetCurrentRadius() const { return currentRadius_; }
	bool IsExpanding() const { return state_ == State::Expanding || state_ == State::WaitMax; }
	bool IsContracting() const { return state_ == State::Contracting; }
	bool IsCritical() const { return isCritical_; }

	const std::vector<std::shared_ptr<DebrisPiece>>& GetPieces() const { return pieces_; }

private:
	SurvivalGameObjectManager* manager_ = nullptr;
	InputManager* input_ = nullptr;
	std::shared_ptr<SurvivalPlayer> anchor_;

	float minRadius_ = 60.0f;
	float maxRadius_ = 350.0f;
	float currentRadius_ = 0.0f;
	float rotationAngle_ = 0.0f;

	enum class State { Idle, Expanding, WaitMax, Contracting, Cooldown };
	State state_ = State::Idle;

	float cooldownTimer_ = 0.0f;
	bool isCritical_ = false;

	std::vector<std::shared_ptr<DebrisPiece>> pieces_;
};

// ==============================
// SurvivalGameObjectManager::CheckCollisions 実装（ここで型が揃う）
// ==============================
inline void SurvivalGameObjectManager::CheckCollisions() {
	if (!player_ || !debrisController_) return;

	const float playerRadius = player_->GetRadius();
	const Vector2 playerPos = player_->GetPosition();

	const bool isAttacking = debrisController_->IsContracting();
	const bool isCritical = debrisController_->IsCritical();

	for (auto& enemy : enemies_) {
		if (!enemy) continue;

		const Vector2 enemyPos = enemy->GetPosition();
		const float enemyRadius = enemy->GetRadius();

		const float distToPlayer = Vector2::Length(enemyPos - playerPos);
		if (distToPlayer < playerRadius + enemyRadius) {
			player_->OnDamage();
			enemy->OnHit(0, Vector2::Normalize(enemyPos - playerPos), 500.0f);
		}

		for (auto& piece : debrisController_->GetPieces()) {
			const Vector2 debrisPos = piece->GetActualPosition();
			const float debrisRadius = 6.0f;

			if (Vector2::Length(enemyPos - debrisPos) < enemyRadius + debrisRadius) {
				if (isAttacking) {
					const int dmg = isCritical ? 5 : 1;
					const float power = isCritical ? 1200.0f : 400.0f;
					const Vector2 knockDir = Vector2::Normalize(enemyPos - playerPos);
					enemy->OnHit(dmg, knockDir, power);
				}
				else {
					const Vector2 pushDir = Vector2::Normalize(enemyPos - debrisPos);
					enemy->PushBack(pushDir, 5.0f);
				}
			}
		}
	}
}