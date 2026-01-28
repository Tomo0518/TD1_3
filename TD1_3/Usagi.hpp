#pragma once

#include "PhysicsObject.hpp"
#include "Boomerang.hpp"
#include "KinokoSpawner.hpp"
#include "UIManager.h"

class Usagi : public PhysicsObject {
private:
	DrawComponentManager drawManager_;
	/*DrawComponent2D* breatheComp_ = nullptr;
	DrawComponent2D* runComp_ = nullptr;
	DrawComponent2D* attackComp_ = nullptr;
	DrawComponent2D* jumpComp_ = nullptr;
	DrawComponent2D* fallComp_ = nullptr;*/

	DrawComponentManager boomerangDrawManager_;
	/*DrawComponent2D* BoomerangDrawComp_ = nullptr;
	DrawComponent2D* BoomerangBreatheComp_ = nullptr;
	DrawComponent2D* BoomerangRunComp_ = nullptr;
	DrawComponent2D* BoomerangAttackComp_ = nullptr;
	DrawComponent2D* BoomerangJumpComp_ = nullptr;
	DrawComponent2D* BoomerangFallComp_ = nullptr;*/

	DrawComponent2D* starComp_ = nullptr;

	std::vector<Boomerang*> boomerangs_;
	int starCount_ = 0;
	bool isCharging_ = false;
	float chargeTimer_ = 0;

	bool isflipX_ = false;
	float jumpForce_ = 21.f;
	float jumpForceBooster_ = 5.f;
	float JumpFriendlyTimer_ = 0;
	float JumpFriendlyDuration_ = 15.f;

	float dashSpeed_ = 20.f;
	float dashDuration_ = 20.f;
	float dashCooldown_ = 40.f;
	float dashDurationTimer_ = 0.f;
	float dashCooldownTimer_ = 0.f;
	bool dashAvailable_ = true;

	float BoomerangJumpCooldown_ = 15.f;
	float BoomerangJumpTimer_ = 0.f;

	float walkSpeed_ = 6.f;

	float currentHp_ = 100.f;
	float maxHp_ = 100.f;

	// ゲームパッド用のデッドゾーン
	const float stickDeadZone_ = 0.23f;

	// ブーメランの前フレームの状態(回収検知)
	bool wasBoomerangActive_ = false;

	Vector2 respawnPosition_ = { 0.f, 0.f };
	float respawnDelay_ = 120.f;
	float respawnTimer_ = 0.f;

	enum DrawCompState {
		eBreathe,
		eRun,
		eAttack,
		eJump,
		eFall
	};
public:
	Usagi() {
		//delete drawComp_;
		//Initialize();
	}
	~Usagi() {
		delete starComp_;
	}
	void Initialize() override {
		status_.maxHP = 50;
		status_.currentHP = status_.maxHP;

		rigidbody_.Initialize();
		rigidbody_.deceleration = { 0.7f, 0.7f };
		rigidbody_.maxSpeedY = jumpForceBooster_ + jumpForce_ + 5.f;
		collider_.size = { 52.f, 120.f };
		collider_.offset = { 5.f, -28.f };

		// 描画コンポーネントの初期化があれば呼ぶ
		delete drawComp_;
		drawComp_ = nullptr;

		drawManager_.RegisterComponent(DrawCompState::eBreathe,
			new DrawComponent2D(Tex().GetTexture(TextureId::UsagiBreathe), 11, 1, 11, 5.f, true));
		drawManager_.RegisterComponent(DrawCompState::eRun,
			new DrawComponent2D(Tex().GetTexture(TextureId::UsagiRun), 8, 1, 8, 5.f, true));
		drawManager_.RegisterComponent(DrawCompState::eAttack,
			new DrawComponent2D(Tex().GetTexture(TextureId::UsagiAttack), 4, 1, 4, 5.f, false));
		drawManager_.RegisterComponent(DrawCompState::eJump,
			new DrawComponent2D(Tex().GetTexture(TextureId::UsagiJump), 2, 1, 2, 5.f, false));
		drawManager_.RegisterComponent(DrawCompState::eFall,
			new DrawComponent2D(Tex().GetTexture(TextureId::UsagiFall), 2, 1, 2, 5.f, false));

		boomerangDrawManager_.RegisterComponent(DrawCompState::eBreathe,
			new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangBreathe), 11, 1, 11, 5.f, true));
		boomerangDrawManager_.RegisterComponent(DrawCompState::eRun,
			new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangRun), 8, 1, 8, 5.f, true));
		boomerangDrawManager_.RegisterComponent(DrawCompState::eAttack,
			new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangAttack), 4, 1, 4, 5.f, false));
		boomerangDrawManager_.RegisterComponent(DrawCompState::eJump,
			new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangJump), 2, 1, 2, 5.f, false));
		boomerangDrawManager_.RegisterComponent(DrawCompState::eFall,
			new DrawComponent2D(Tex().GetTexture(TextureId::BoomerangFall), 2, 1, 2, 5.f, false));

		auto compNames = { DrawCompState::eBreathe, DrawCompState::eRun, DrawCompState::eAttack, DrawCompState::eJump, DrawCompState::eFall };
		for (const auto& name : compNames) {
			if (auto* comp = drawManager_.GetComponent(name)) {
				comp->Initialize();
			}
			if (auto* comp = boomerangDrawManager_.GetComponent(name)) {
				comp->Initialize();
			}
		}

		starComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Star_shooting), 4, 1, 4, 5.f, true);
		starComp_->Initialize();


		//==========================================
		boomerangs_.clear();
		auto boomerang = manager_->Spawn<Boomerang>(this, "Boomerang", this, false);
		boomerangs_.push_back(boomerang);
	}



	void ChangeDrawComp(DrawCompState state) {
		switch (state) {
		case DrawCompState::eBreathe:
			drawManager_.ChangeComponent(DrawCompState::eBreathe);
			boomerangDrawManager_.ChangeComponent(DrawCompState::eBreathe);
			break;
		case DrawCompState::eRun:
			drawManager_.ChangeComponent(DrawCompState::eRun);
			boomerangDrawManager_.ChangeComponent(DrawCompState::eRun);
			break;
		case DrawCompState::eAttack:
			drawManager_.ChangeComponent(DrawCompState::eAttack);
			boomerangDrawManager_.ChangeComponent(DrawCompState::eAttack);
			break;
		case DrawCompState::eJump:
			drawManager_.ChangeComponent(DrawCompState::eJump);
			boomerangDrawManager_.ChangeComponent(DrawCompState::eJump);
			break;
		case DrawCompState::eFall:
			drawManager_.ChangeComponent(DrawCompState::eFall);
			boomerangDrawManager_.ChangeComponent(DrawCompState::eFall);
			break;
		}
	}


	void Jump(bool isDoubleJump = false) {
		if (isGravityEnabled_ == false) return;
		JumpFriendlyTimer_ = 0;
		rigidbody_.velocity.y = 0.f; // ジャンプ前に垂直速度をリセット
		rigidbody_.acceleration.y += jumpForce_ + (isDoubleJump ? jumpForceBooster_ : 0.f);
		isGrounded_ = false;
		// ジャンプアニメーションに切り替え
		ChangeDrawComp(DrawCompState::eJump);
		SoundManager::GetInstance().PlaySe(SeId::PlayerJump);

	}

	void HandleInput() {
		if (!info_.isActive) return;
		// *************** ジャンプ処理（チャージ状態の前に処理） ******************
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

			// ジャンプ
			if (Input().GetPad()->Trigger(Pad::Button::A)) {
				inputDir.y += 1.0f;
				jumpInput = true;
				JumpFriendlyTimer_ = JumpFriendlyDuration_;
			}

			// ダッシュ
			if (Input().GetPad()->Trigger(Pad::Button::B)||
				Input().GetPad()->RightTrigger() >= 0.2f || 
				Input().GetPad()->LeftTrigger() >= 0.2f
				) {
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
			JumpFriendlyTimer_ = JumpFriendlyDuration_;
		}

		// キーボードダッシュ
		if (Input().TriggerKey(DIK_K)) {
			dashInput = true;
		}

		// 入力方向を正規化
		inputDir = Vector2::Normalize(inputDir);

		// ブーメランジャンプの処理
		bool isCloseToBoomerang = false;
		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary()) {
				if (!boom->IsIdle()) {
					if (!isGrounded_ && boom->isWaitingToReturn() && BoomerangJumpTimer_ <= 0.f) {
						float distance = Vector2::Length(this->GetPosition() - boom->GetPosition());
						if (distance < 100.f) {
							BoomerangJumpTimer_ = BoomerangJumpCooldown_;
							isCloseToBoomerang = true;
							ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
						}
					}
				}
				break;
			}
		}

		// ジャンプ実行（チャージ中でも可能にする）
		if (((jumpInput || JumpFriendlyTimer_ > 0.f) && isGrounded_) || (isCloseToBoomerang)) {
			Jump(isCloseToBoomerang);
		}

		if (isGrounded_) {
			dashAvailable_ = true;
		}

		// ダッシュ処理
		if (dashInput) {
			if ((dashCooldownTimer_ <= 0.f && dashAvailable_)) {
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

				ChangeDrawComp(DrawCompState::eJump);
				SoundManager::GetInstance().PlaySe(SeId::PlayerDash);
			}
		}

		// チャージ中の処理
		if (isCharging_) {
			// チャージ中でもジャンプは可能なので、重力の無効化はジャンプしていない場合のみ
			if (!jumpInput) {
				isGravityEnabled_ = false;
			}
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

				if (isGrounded_)	ChangeDrawComp(DrawCompState::eRun);

				if (inputDir.x > 0) {
					isflipX_ = false;
				}
				else {
					isflipX_ = true;
				}

			}
			else {
				if (isGrounded_) {
					ChangeDrawComp(DrawCompState::eBreathe);
				}
			}
		}
	}
	bool hitMaxCharge = false;

	void HandleBoomerang(float deltaTime) {
		deltaTime;
		// Throw boomerang logic
		bool tryThrow = false;
		Vector2 throwDir = { 0, 0 };

		if (Input().ReleaseKey(DIK_J) || Input().GetPad()->Release(Pad::Button::X)) {
			tryThrow = true;
			throwDir = { !isflipX_ ? 1.f : -1.f, 0 };
		}

		/*if (Input().ReleaseKey(DIK_UP)) { throwDir = { 0, 1 }; tryThrow = true; }
		else if (Input().ReleaseKey(DIK_DOWN)) { throwDir = { 0, -1 }; tryThrow = true; }
		else if (Input().ReleaseKey(DIK_LEFT)) { throwDir = { -1, 0 }; tryThrow = true; }
		else if (Input().ReleaseKey(DIK_RIGHT)) { throwDir = { 1, 0 }; tryThrow = true; }*/


		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary()) {
				if (boom->IsIdle()) {
					boom->SetPosition(transform_.translate);
					if (/*Input().PressKey(DIK_UP) || Input().PressKey(DIK_DOWN) || Input().PressKey(DIK_LEFT) || Input().PressKey(DIK_RIGHT)*/
						Input().PressKey(DIK_J) || Input().GetPad()->Press(Pad::Button::X)
						) {
						isCharging_ = true;
						chargeTimer_ = std::min(chargeTimer_ + deltaTime, 120.f);
						if (chargeTimer_ == 120.f && !hitMaxCharge) {
							ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
							hitMaxCharge = true;
						}
					}

					if (!boom->isStarRetrieved()) {
						starCount_ = boom->retrieveStarCount();
					}

				}
				else {
					if (Input().ReleaseKey(DIK_J) || Input().GetPad()->Release(Pad::Button::X)) {
						boom->SwitchToReturn();
					}
				}

				break;
			}
		}


		if (tryThrow) {
			isCharging_ = false;
			hitMaxCharge = false;
			ThrowBoomerang(throwDir);
		}
		ClearDeadBoomerangs();
	}

	// ========== 更新・描画 ==========
	void Update(float deltaTime)override {
		UpdateTimer(deltaTime);

		HandleInput();
		Move(deltaTime);

		HandleBoomerang(deltaTime);
		UpdateBoomerangPadVibration();
		RespawnHandle();

		UpdateDrawComponent(deltaTime);

		//Novice::ConsolePrintf("isGrounded: %d\n", isGrounded_);
		SpawnTestKinoko();
	}

	void UpdateTimer(float deltaTime) {
		dashDurationTimer_ -= deltaTime;
		dashCooldownTimer_ -= deltaTime;
		BoomerangJumpTimer_ -= deltaTime;
		JumpFriendlyTimer_ -= deltaTime;
		respawnTimer_ -= deltaTime;
	}

	void RespawnHandle(){
		if (status_.currentHP <= 0.f) {
			if (respawnTimer_ <= 0.f) {
				// Respawn
				Respawn();
			}
			if (respawnTimer_ <= respawnDelay_ * 0.5f) {
				transform_.translate = respawnPosition_ + Vector2(0,10.f);
			}
		}
	}

	void SpawnTestKinoko() {
		if (Input().TriggerKey(DIK_U)) {
			Novice::ConsolePrintf("\ntry spawn1\n");
			if (manager_) {
				auto* enemy = manager_->Spawn<FatEnemy>(nullptr, "Enemy");
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

			SoundManager::GetInstance().PlaySe(SeId::PlayerBoomerangReturn);
		}
		wasBoomerangActive_ = currentBoomerangActive;
	}

	virtual void UpdateDrawComponent(float deltaTime) override {
		drawManager_.SetFlipX(isflipX_);
		boomerangDrawManager_.SetFlipX(isflipX_);
		float shakeX = (rand() % 100 / 100.0f - 0.5f) * 2.0f * chargeTimer_ / 6.f;
		float shakeY = (rand() % 100 / 100.0f - 0.5f) * 2.0f * chargeTimer_ / 6.f;
		Vector2 shakeOffset = { shakeX, shakeY };
		drawManager_.SetTransform(transform_);
		drawManager_.SetPosition(transform_.translate + shakeOffset);
		drawManager_.Update(deltaTime);

		boomerangDrawManager_.SetTransform(transform_);
		boomerangDrawManager_.SetPosition(transform_.translate + shakeOffset);
		boomerangDrawManager_.Update(deltaTime);

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
		if (!info_.isActive) {

			float RspawnProgress = (respawnTimer_ / (respawnDelay_*0.5f));
			unsigned int alpha = static_cast<unsigned int>(std::min(255.f,255.f *  RspawnProgress));
			Novice::DrawBox(
				0,0,
				1280, 720,
				0.0f,
				0x00000000 | alpha,
				kFillModeSolid
			);

			return;
		}
		if (!info_.isVisible) return;
		// DrawComponent2Dを使って描画
		drawManager_.Draw(camera);

		// if found a non temporary boomerang that is not idle, draw boomerang
		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary() && boom->IsIdle()) {
				if (boom->isStarRetrieved()) {
					boomerangDrawManager_.Draw(camera);
					DrawStar(camera);
				}
				break;
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
			const Vector2 gravity = { 0.0f, -1.0f };
			rigidbody_.AddForce(gravity);
		}

		bool previouslyGrounded = isGrounded_;

		rigidbody_.Update(deltaTime);

		Vector2 moveDelta = rigidbody_.GetMoveDelta(deltaTime);
		auto& mapData = MapData::GetInstance();

		// ========== Y方向の移動と衝突判定 ==========
		transform_.translate.y += moveDelta.y;
		if (!isGravityEnabled_ && isCharging_) transform_.translate.y -= 1;
		HitDirection hitDirY = PhysicsManager::ResolveMapCollisionY(this, mapData);
		isGrounded_ = (hitDirY == HitDirection::Top);
		if (!isGravityEnabled_ && !isGrounded_ && isCharging_) transform_.translate.y += 1;
		// ========== X方向の移動と衝突判定 ==========
		transform_.translate.x += moveDelta.x;
		HitDirection hitDirX = PhysicsManager::ResolveMapCollisionX(this, mapData);
		
		// ========== 回転 ==========
		transform_.rotation += rigidbody_.GetRotationDelta(deltaTime);

		// デバッグ出力
		if (hitDirX != HitDirection::None || hitDirY != HitDirection::None) {
			/*Novice::ConsolePrintf("Hit Direction X: %d, Y: %d, position: (%.f, %.f)\n",
				static_cast<int>(hitDirX), static_cast<int>(hitDirY),
				transform_.translate.x, transform_.translate.y);*/
		}

		if(previouslyGrounded == false && isGrounded_ == true) {
			SoundManager::GetInstance().PlaySe(SeId::PlayerLand);
		}

		transform_.CalculateWorldMatrix();
	}

	int OnCollision(GameObject2D* other) override {

		if (other->GetInfo().tag == "Star") {
			if (other->GetInfo().isActive) {

				starCount_ = std::min(4, starCount_ + 1);
				other->GetInfo().isActive = false;

				SoundManager::GetInstance().PlaySe(SeId::PlayerStarCollect);
			}
		}
		else if (other->GetInfo().tag == "Enemy" || other->GetInfo().tag == "Player") {
			// knockback on collision with other enemies
			Vector2 knockbackDir = Vector2::Subtract(transform_.translate, other->GetTransform().translate);
			knockbackDir = Vector2::Normalize(knockbackDir); 
			//rigidbody_.acceleration.x = 0.f;
			rigidbody_.velocity.x = 0.f;
			rigidbody_.acceleration.x += (knockbackDir.x * (rigidbody_.acceleration.x +2.5f));
			//rigidbody_.acceleration.y += (knockbackDir.y * rigidbody_.acceleration.y);

		}
		else if (other->GetInfo().tag == "CheckPoint") {
			status_.currentHP = status_.maxHP;
			respawnPosition_ = other->GetTransform().translate;
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

	void Respawn() {
		info_.isActive = true;
		starCount_ = 0;
		status_.currentHP = status_.maxHP;
		transform_.translate = respawnPosition_;
		ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
		Boomerang* firstBoomerang = static_cast<Boomerang*>(GetFirstBoomerang());
		firstBoomerang->ResetToIdle();
		firstBoomerang->SetPosition(transform_.translate);
	}


	virtual void OnDamaged(int damage) override {
		if (damage < 0) return; // 負のダメージは無効
		status_.currentHP -= damage;
		if (status_.currentHP <= 0) {
			status_.currentHP = 0;
			ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
			ParticleManager::GetInstance().Emit(ParticleType::Enemy_Dead, transform_.translate);

			respawnTimer_ = respawnDelay_;
			info_.isActive = false;
		}
		else {
			if (damage > 0) {
				ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
				drawManager_.StartFlashBlink(0xFF0000CC, 4, 0.1f, BlendMode::kBlendModeNormal, 1);

				SoundManager::GetInstance().PlaySe(SeId::PlayerDamage);
			}			
		}
	}

	PlayerSkillState GetSkillState() const {
		PlayerSkillState state;

		// ダッシュ状態
		state.isDashing = dashDurationTimer_ > 0.f;
		state.canDash = dashCooldownTimer_ <= 0.f && dashAvailable_;

		// ブーメランの状態を判定
		for (auto boom : boomerangs_) {
			if (!boom->IsTemporary()) {
				if (boom->IsIdle()) {
					// 待機中 → 投げることができる
					state.boomerangMode = PlayerSkillState::BoomerangMode::Throwing;
					state.canUseBoomerang = true;
				}
				else if (boom->isWaitingToReturn()) {
					// 目標到達 → 回収可能（不透明）
					state.boomerangMode = PlayerSkillState::BoomerangMode::Recalling;
					state.canUseBoomerang = true;
				}
				else {
					// 投げた直後 or 回収中 → 操作不可（半透明）
					state.boomerangMode = PlayerSkillState::BoomerangMode::Recalling;
					state.canUseBoomerang = false;
				}
				break;
			}
		}

		return state;
	}

	bool IsInRespawnDelay() const {
		return respawnTimer_ <= 0.f;
	}
};