#pragma once

#include "PhysicsObject.hpp"
#include "Boomerang.hpp"
#include "KinokoSpawner.hpp"

class Usagi : public PhysicsObject {
private:
	DrawComponent2D* breatheComp_ = nullptr;
	DrawComponent2D* runComp_ = nullptr;
	DrawComponent2D* attackComp_ = nullptr;
	DrawComponent2D* jumpComp_ = nullptr;
	DrawComponent2D* fallComp_ = nullptr;

	DrawComponent2D* BoomerangDrawComp_ = nullptr;
	DrawComponent2D* BoomerangBreatheComp_ = nullptr;
	DrawComponent2D* BoomerangRunComp_ = nullptr;
	DrawComponent2D* BoomerangAttackComp_ = nullptr;
	DrawComponent2D* BoomerangJumpComp_ = nullptr;
	DrawComponent2D* BoomerangFallComp_ = nullptr;

	DrawComponent2D* starComp_ = nullptr;

	std::vector<Boomerang*> boomerangs_;
	int starCount_ = 0;
	bool isCharging_ = false;
	float chargeTimer_ = 0;

	bool isflipX_ = false;
	float jumpForce_ = 21.f;

	float dashSpeed_ = 20.f;
	float dashDuration_ = 20.f;
	float dashCooldown_ = 40.f;
	float dashDurationTimer_ = 0.f;
	float dashCooldownTimer_ = 0.f;
	bool dashAvailable_ = true;

	float walkSpeed_ = 6.f;

	// ゲームパッド用のデッドゾーン
	const float stickDeadZone_ = 0.23f;

	// ブーメランの前フレームの状態(回収検知)
	bool wasBoomerangActive_ = false;
public:
	Usagi() {
		drawComp_ = nullptr;
		//Initialize();
	}
	~Usagi() {
		if (breatheComp_ != drawComp_) delete breatheComp_;
		if (runComp_ != drawComp_) delete runComp_;
		if (attackComp_ != drawComp_) delete attackComp_;
		if (jumpComp_ != drawComp_) delete jumpComp_;
		if (fallComp_ != drawComp_) delete fallComp_;

		delete BoomerangBreatheComp_;
		delete BoomerangRunComp_;
		delete BoomerangAttackComp_;
		delete BoomerangJumpComp_;
		delete BoomerangFallComp_;
		delete starComp_;
	}
	void Initialize() override {
		rigidbody_.Initialize();
		rigidbody_.deceleration = { 0.7f, 0.7f };
		collider_.size = { 52.f, 120.f };
		collider_.offset = { 5.f, -28.f };
		// 描画コンポーネントの初期化があれば呼ぶ
		delete drawComp_;
		drawComp_ = nullptr;

		breatheComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiBreathe), 11, 1, 11, 5.f, true);
		runComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiRun), 8, 1, 8, 5.f, true);
		attackComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiAttack), 4, 1, 4, 5.f, false);
		jumpComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiJump), 2, 1, 2, 5.f, false);
		fallComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiFall), 2, 1, 2, 5.f, false);

		BoomerangBreatheComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangBreathe), 11, 1, 11, 5.f, true);
		BoomerangRunComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangRun), 8, 1, 8, 5.f, true);
		BoomerangAttackComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangAttack), 4, 1, 4, 5.f, false);
		BoomerangJumpComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangJump), 2, 1, 2, 5.f, false);
		BoomerangFallComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangFall), 2, 1, 2, 5.f, false);

		starComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Star_shooting), 4, 1, 4, 5.f, true);

		breatheComp_->Initialize();
		runComp_->Initialize();
		attackComp_->Initialize();
		jumpComp_->Initialize();
		fallComp_->Initialize();

		BoomerangBreatheComp_->Initialize();
		BoomerangRunComp_->Initialize();
		BoomerangAttackComp_->Initialize();
		BoomerangJumpComp_->Initialize();
		BoomerangFallComp_->Initialize();

		starComp_->Initialize();

		BoomerangDrawComp_ = BoomerangBreatheComp_;
		drawComp_ = breatheComp_;

		//==========================================
		boomerangs_.clear();
		auto boomerang = manager_->Spawn<Boomerang>(this, "Boomerang", this, false);
		boomerangs_.push_back(boomerang);
	}

	void Jump() {		
			rigidbody_.acceleration.y += jumpForce_;
			isGrounded_ = false;
			// ジャンプアニメーションに切り替え
			if (drawComp_ != jumpComp_) {
				drawComp_ = jumpComp_;
				BoomerangDrawComp_ = BoomerangJumpComp_;
				//drawComp_->StopAnimation();
				drawComp_->PlayAnimation();
				BoomerangDrawComp_->PlayAnimation();
			}
		
	}

	void HandleInput() {
		if (!isGrounded_) {
			if (rigidbody_.velocity.y < 0) {
				// 落下アニメーションに切り替え
				if (drawComp_ != fallComp_) {
					drawComp_ = fallComp_;
					//drawComp_->StopAnimation();
					drawComp_->PlayAnimation();
				}
			}
		}
		// 入力取得
		Vector2 inputDir = { 0.0f, 0.0f };
		bool jumpInput = false;
		bool dashInput = false;

		// ゲームパッド入力（左スティック）
		if (Input().GetPad()) {
			float stickX = Input().GetPad()->GetLeftStick().x;
			float stickY = Input().GetPad()->GetLeftStick().y;

			// 左右移動（デッドゾーン適用）
			if (abs(stickX) > stickDeadZone_) {
				inputDir.x = stickX;
			}

			// 上下移動（デッドゾーン適用）- ダッシュで使用
			if (abs(stickY) > stickDeadZone_) {
				inputDir.y = stickY;
			}

			// スマブラ風ジャンプ（スティック上弾き）
			// 接地中かつ強く上に倒した場合のみジャンプ
			if (stickY > 0.7f && isGrounded_) {
				jumpInput = true;
			}

			// ダッシュ（Bボタン）
			if (Input().GetPad()->Trigger(Pad::Button::B)) {
				dashInput = true;
			}
		}

		// キーボード入力（フォールバック）
		if (Input().PressKey(DIK_A)) {
			inputDir.x -= 1.0f;
		}
		if (Input().PressKey(DIK_D)) {
			inputDir.x += 1.0f;
		}
		if (Input().PressKey(DIK_W)) {
			inputDir.y += 1.0f;
		}
		if (Input().PressKey(DIK_S)) {
			inputDir.y -= 1.0f;
		}

		// キーボードジャンプ
		if (Input().TriggerKey(DIK_W)) {
			jumpInput = true;
		}

		// キーボードダッシュ
		if (Input().TriggerKey(DIK_K)) {
			dashInput = true;
		}

		// 入力方向を正規化
		inputDir = Vector2::Normalize(inputDir);

		// ジャンプ処理
		if (jumpInput && isGrounded_) {
			Jump();
		}

		if (isGrounded_) {
			dashAvailable_ = true;
		}

		// ダッシュ処理
		if (dashInput) {
			if (dashCooldownTimer_ <= 0.f && dashAvailable_) {
				if (Input().GetInputMode() == InputMode::Gamepad) {
					Input().GetPad()->StartVibration(0.5f, 0.5f, 10);
				}

				dashAvailable_ = false;

				Vector2 dashDir = { 0.f, 0.f };
				if (Vector2::Length(inputDir) == 0.f) {
					dashDir.x = isflipX_ ? -1.f : 1.f;
				}
				else {
					dashDir = inputDir;
				}

				rigidbody_.velocity = { 0.f, 0.f };
				rigidbody_.acceleration += dashDir * dashSpeed_;

				if (dashDir.y == 0.f) transform_.translate.y += 2.f;

				dashDurationTimer_ = dashDuration_;
				dashCooldownTimer_ = dashCooldown_;

				isGravityEnabled_ = false;
				rigidbody_.deceleration = { 0.9f, 0.9f };
				rigidbody_.maxSpeedX = dashSpeed_;

				if (drawComp_ != jumpComp_) {
					drawComp_ = jumpComp_;
					BoomerangDrawComp_ = BoomerangJumpComp_;
					drawComp_->PlayAnimation();
					BoomerangDrawComp_->PlayAnimation();
				}
			}
		}

		if (isCharging_) {
			isGravityEnabled_ = false;
			if (inputDir.x != 0) {
				isflipX_ = inputDir.x < 0.f;
			}
			
		}

		if (dashDurationTimer_ <= 0.f && !isCharging_) {
			isGravityEnabled_ = true;
			rigidbody_.deceleration = { 0.7f, 0.7f };
			rigidbody_.maxSpeedX = walkSpeed_;

			if (inputDir.x != 0.0f) {
				rigidbody_.acceleration.x += inputDir.x * rigidbody_.maxSpeedX / 6.f;

				if (inputDir.x > 0) {
					// キャラクターの向きを右に設定
					if (drawComp_ != runComp_ && isGrounded_) {
						drawComp_ = runComp_;
						BoomerangDrawComp_ = BoomerangRunComp_;
						//drawComp_->StopAnimation();
						drawComp_->PlayAnimation();
						BoomerangDrawComp_->PlayAnimation();
					}
					isflipX_ = false;
				}
				else {
					// キャラクターの向きを左に設定
					if (drawComp_ != runComp_ && isGrounded_) {
						drawComp_ = runComp_;
						BoomerangDrawComp_ = BoomerangRunComp_;
						//drawComp_->StopAnimation();
						drawComp_->PlayAnimation();
						BoomerangDrawComp_->PlayAnimation();
					}
					isflipX_ = true;
				}

			}
			else {
				if (isGrounded_) {
					if (drawComp_ != breatheComp_) {
						drawComp_ = breatheComp_;
						BoomerangDrawComp_ = BoomerangBreatheComp_;
						drawComp_->PlayAnimation();
						BoomerangDrawComp_->PlayAnimation();

					}
				}
			}
		}
	}



	void HandleBoomerang(float deltaTime) {
		deltaTime;
		// Throw boomerang logic
		bool tryThrow = false;
		Vector2 throwDir = { 0, 0 };

		

		if (Input().ReleaseKey(DIK_J) || Input().GetPad()->Trigger(Pad::Button::A)) {
			tryThrow = true;
			throwDir = {!isflipX_?1.f:-1.f, 0};
		}

		/*if (Input().ReleaseKey(DIK_UP)) { throwDir = { 0, 1 }; tryThrow = true; }
		else if (Input().ReleaseKey(DIK_DOWN)) { throwDir = { 0, -1 }; tryThrow = true; }
		else if (Input().ReleaseKey(DIK_LEFT)) { throwDir = { -1, 0 }; tryThrow = true; }
		else if (Input().ReleaseKey(DIK_RIGHT)) { throwDir = { 1, 0 }; tryThrow = true; }*/


		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary()) {
				if (boom->IsIdle()) {
					if (/*Input().PressKey(DIK_UP) || Input().PressKey(DIK_DOWN) || Input().PressKey(DIK_LEFT) || Input().PressKey(DIK_RIGHT)*/
						Input().PressKey(DIK_J) || Input().GetPad()->Trigger(Pad::Button::A)
						) {
						isCharging_ = true;
						chargeTimer_ = std::min(chargeTimer_ + deltaTime, 120.f);
						if (chargeTimer_ == 120.f) {
							ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
						}
					}

					if (!boom->isStarRetrieved()) {
						starCount_ = boom->retrieveStarCount();
					}
					break;
				}
				else {
					if (Input().PressKey(DIK_J) || Input().GetPad()->Trigger(Pad::Button::A)) {
						boom->SwitchToReturn();
					}
						
				}
			}
		}


		if (tryThrow) {
			isCharging_ = false;
			ThrowBoomerang(throwDir);
		}
		ClearDeadBoomerangs();
	}

	// ========== 更新・描画 ==========
	void Update(float deltaTime)override {
		dashDurationTimer_ -= deltaTime;
		dashCooldownTimer_ -= deltaTime;

		HandleInput();
		Move(deltaTime);
		HandleBoomerang(deltaTime);

		UpdateBoomerangPadVibration();

		if (drawComp_) drawComp_->SetFlipX(isflipX_);
		if (BoomerangDrawComp_) BoomerangDrawComp_->SetFlipX(isflipX_);
		UpdateDrawComponent(deltaTime);

		//Novice::ConsolePrintf("isGrounded: %d\n", isGrounded_);
		if (Input().TriggerKey(DIK_U)) {
			Novice::ConsolePrintf("try spawn1\n");
			if (manager_) {
				Novice::ConsolePrintf("try spawn2\n");
				auto* enemy = manager_->Spawn<AttackEnemy>(nullptr, "Enemy");
				enemy->SetPosition({ transform_.translate.x, transform_.translate.y + 50.f });
				enemy->Initialize();
			}
		}
	}

	void UpdateBoomerangPadVibration() {
		// ブーメランの滞空振動と回収検出
		bool currentBoomerangActive = false;
		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary() && !boom->IsIdle()) {
				currentBoomerangActive = true;
				// ブーメランとプレイヤーの距離を計算
				Vector2 boomerangPos = boom->GetPosition();
				Vector2 playerPos = transform_.translate;
				float distance = Vector2::Length(boomerangPos - playerPos);
				// 距離に応じた振動（近いほど強く、最大距離800.0fと仮定）
				float maxDistance = 800.0f;
				float minDistance = 100.0f;
				if (distance > minDistance && distance < maxDistance) {
					// 距離が近いほど強く振動（0.0f～0.2f）
					float vibrationPower = (1.0f - (distance - minDistance) / (maxDistance - minDistance)) * 0.2f;
					if (Input().GetInputMode() == InputMode::Gamepad) {
						Input().GetPad()->StartVibration(vibrationPower, vibrationPower, 2);
					}
				}
				break;
			}
		}
		// ブーメラン回収時の振動検出
		if (wasBoomerangActive_ && !currentBoomerangActive) {
			if (Input().GetInputMode() == InputMode::Gamepad) {
				Input().GetPad()->StartVibration(0.5f, 0.5f, 15);
			}
		}
		wasBoomerangActive_ = currentBoomerangActive;
	}

	virtual void UpdateDrawComponent(float deltaTime) override {
		if (drawComp_) {
			float shakeX = (rand() % 100 / 100.0f - 0.5f) * 2.0f * chargeTimer_/4.f;
			float shakeY = (rand() % 100 / 100.0f - 0.5f) * 2.0f * chargeTimer_/4.f;
			Vector2 shakeOffset = { shakeX, shakeY };
			drawComp_->SetTransform(transform_);
			drawComp_->SetPosition(transform_.translate + shakeOffset);
			drawComp_->Update(deltaTime);
		}
		if (BoomerangDrawComp_) {
			BoomerangDrawComp_->SetTransform(transform_);
			BoomerangDrawComp_->Update(deltaTime);
		}

		if (starComp_) {
			starComp_->Update(deltaTime);
		}
	}

	float AngleToRadians(float angleDegrees) {
		return angleDegrees * (3.14159265f / 180.0f);
	}

	void DrawStar(const Camera2D& camera) {
		if (starCount_ <= 0) return;

		float radius = 50.f;
		float angleStep = 360.f / starCount_;

		starComp_->SetRotation(starComp_->GetRotation() - 0.2f);

		for (int i = 0; i < starCount_; ++i) {
			float angle = angleStep * i;
			Vector2 offset = { cosf(AngleToRadians(angle)) * radius, sinf(AngleToRadians(angle)) * radius };
			starComp_->SetPosition(transform_.translate + offset);
			starComp_->Draw(camera);
		}
	}

	void Draw(const Camera2D& camera)override {
		if (!info_.isActive || !info_.isVisible) return;
		// DrawComponent2Dを使って描画
		if (drawComp_) {
			drawComp_->Draw(camera);
		}
		if (BoomerangDrawComp_) {
			// if found a non temporary boomerang that is not idle, draw boomerang
			for (auto boom : boomerangs_) {
				if (!boom->IsTemporary() && boom->IsIdle()) {
					if (boom->isStarRetrieved()) {
						BoomerangDrawComp_->Draw(camera);
						DrawStar(camera);
					}
					break;
				}
			}
		}



		// draw hitbox for debug
		/*Vector2 screenPos = const_cast<Camera2D&>(camera).WorldToScreen(transform_.translate);
		Vector2 colliderSize = const_cast<Vector2&>(collider_.size);
		Vector2 colliderOffset = const_cast<Vector2&>(collider_.offset);

		Novice::DrawBox(
			int(screenPos.x + colliderOffset.x - colliderSize.x/2.f),
			int(screenPos.y + colliderOffset.y),
			int(colliderSize.x), int(colliderSize.y),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);*/
	}

	virtual void Move(float deltaTime) override {

		if (isGravityEnabled_) {
			// 重力を加える
			//if (!isGrounded_) {
			const Vector2 gravity = { 0.0f, -1.0f }; // 下方向に980ピクセル/秒²の重力
			rigidbody_.AddForce(gravity);
			//}
		}

		rigidbody_.Update(deltaTime);

		Vector2 moveDelta = rigidbody_.GetMoveDelta(deltaTime);
		auto& mapData = MapData::GetInstance();

		transform_.translate.y += moveDelta.y;
		HitDirection hitDir = PhysicsManager::ResolveMapCollision(this, mapData);
		isGrounded_ = (hitDir == HitDirection::Top);
		if (isGrounded_)  transform_.translate.y += 2;

		transform_.translate.x += moveDelta.x;
		HitDirection hitDir2 = PhysicsManager::ResolveMapCollision(this, mapData);
		if (isGrounded_)  transform_.translate.y -= 2;
		// Apply rotation (if any)
		transform_.rotation += rigidbody_.GetRotationDelta(deltaTime);

		if ((hitDir != HitDirection::None && hitDir != HitDirection::Top) || (hitDir2 != HitDirection::None && hitDir2 != HitDirection::Top)) {
			//Novice::ConsolePrintf("Hit Direction X: %d, Y: %d, position: (%.f, %.f)\n", static_cast<int>(hitDir2), static_cast<int>(hitDir), transform_.translate.x, transform_.translate.y);
		}


		transform_.CalculateWorldMatrix();

	}

	int OnCollision(GameObject2D* other) override {

		if (other->GetInfo().tag == "Star") {
			if (other->GetInfo().isActive) {

				starCount_ = std::min(4, starCount_ + 1);
				other->GetInfo().isActive = false;
			}
		}

		return 0;
	}

	void ThrowBoomerang(Vector2& throwDir) {
		// Check throw/jump permission first
		bool allDefaultReturned = true;
		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary() && !boom->IsIdle()) {
				allDefaultReturned = false;
				break;
			}
		}

		bool canThrow = true; /*transform_.isGrounded || allDefaultReturned;*/

		if (canThrow) {
			// Determine which boomerang to use
			Boomerang* b = nullptr;
			for (auto boom : boomerangs_) {
				if (boom->IsIdle()) {
					b = boom;
					break;
				}
			}

			// If no idle, try use star
			/*if (!b && starCount_ > 0) {
				starCount_--;
				if (manager_) {
					Boomerang* starB = manager_->Spawn<Boomerang>(this, "Boomerang", this, true);
					starB->GetInfo().isActive = false;
					starB->GetInfo().isVisible = false;
					boomerangs_.push_back(starB);
					b = starB;
					Novice::ScreenPrintf(10, 100, "Created Star Boomerang!");
				}

			}*/

			if (b) {
				b->Throw(throwDir, starCount_, chargeTimer_);
				chargeTimer_ = 0;

				// ブーメラン投げ時の振動

				if (Input().GetInputMode() == InputMode::Gamepad) {
					Input().GetPad()->StartVibration(0.3f, 0.3f, 10);
				}

				if (Input().PressKey(DIK_DOWN)) {
					rigidbody_.acceleration.y = 0;
					rigidbody_.velocity.y = 0;
					Jump();
				}
			}
		}
	}

	void ClearDeadBoomerangs() {
		boomerangs_.erase(
			std::remove_if(boomerangs_.begin(), boomerangs_.end(),
				[](Boomerang* b) {
					// Remove if the pointer is null or the object is marked for death
					return b == nullptr || b->IsDead();
				}
			),
			boomerangs_.end()
		);
	}

	GameObject2D* GetFirstBoomerang() {
		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary()) {
				return boom;
			}
		}
		return nullptr;
	}

};