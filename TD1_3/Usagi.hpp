#pragma once

#include "PhysicsObject.hpp"
#include "Boomerang.hpp"
#include "Enemy.hpp"

class Usagi : public PhysicsObject {
private:
	DrawComponent2D* breatheComp_ = nullptr;
	DrawComponent2D* runComp_ = nullptr;
	DrawComponent2D* attackComp_ = nullptr;
	DrawComponent2D* jumpComp_ = nullptr;
	DrawComponent2D* fallComp_ = nullptr;

	std::vector<Boomerang*> boomerangs_;
	int starCount_ = 0;

	bool isflipX_ = false;
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
	}
	void Initialize() override {
		rigidbody_.Initialize();
		rigidbody_.deceleration = { 0.7f, 0.7f };
		collider_.size = { 52.f, 124.f };
		collider_.offset = { 5.f, -28.f };
		// 描画コンポーネントの初期化があれば呼ぶ
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiIdle), 1, 1, 1, 1.f, true);
		breatheComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiBreathe), 11, 1, 11, 5.f, true);
		runComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiRun), 8, 1, 8, 5.f, true);
		attackComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiAttack), 4, 1, 4, 5.f, false);
		jumpComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiJump), 2, 1, 2, 5.f, false);
		fallComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiFall), 2, 1, 2, 5.f, false);

		
		breatheComp_->Initialize();
		runComp_->Initialize();
		attackComp_->Initialize();
		jumpComp_->Initialize();
		fallComp_->Initialize();
		

		drawComp_ = breatheComp_;

		//==========================================
		boomerangs_.clear();
		auto boomerang = manager_->Spawn<Boomerang>(this, "Boomerang", this, false);
		boomerangs_.push_back(boomerang);
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
		// 例: 矢印キーで移動
		Vector2 inputDir = { 0.0f, 0.0f };

		if (Input().PressKey(DIK_W)) {
			if (isGrounded_) {
				rigidbody_.acceleration.y += 100.f;
				isGrounded_ = false;

				// ジャンプアニメーションに切り替え
				if (drawComp_ != jumpComp_) {
					drawComp_ = jumpComp_;
					//drawComp_->StopAnimation();
					drawComp_->PlayAnimation();
				}
			}
		}
		/*if (Input().PressKey(DIK_S)) {
			inputDir.y -= 1.0f;
		}*/
		if (Input().PressKey(DIK_A)) {
			inputDir.x -= 1.0f;

			// キャラクターの向きを左に設定
			if (drawComp_ != runComp_ && isGrounded_) {
				drawComp_ = runComp_;
				//drawComp_->StopAnimation();
				drawComp_->PlayAnimation();
			}
			isflipX_ = true;
		}
		if (Input().PressKey(DIK_D)) {
			inputDir.x += 1.0f;

			// キャラクターの向きを右に設定
			if (drawComp_ != runComp_ && isGrounded_) {
				drawComp_ = runComp_;
				//drawComp_->StopAnimation();
				drawComp_->PlayAnimation();
			}
			isflipX_ = false;
		}
		// 入力方向を正規化して速度に変換
		if (inputDir.x != 0.0f) {
			inputDir = Vector2::Normalize(inputDir);
			rigidbody_.acceleration += inputDir * rigidbody_.maxSpeedX/6.f;
		}
		else {
			if (isGrounded_) {
				if (drawComp_ != breatheComp_) {
					drawComp_ = breatheComp_;
					drawComp_->PlayAnimation();

				}
			}
		}
	}

	void HandleBoomerang(float deltaTime) {
		deltaTime;
		// Throw boomerang logic
		bool tryThrow = false;
		Vector2 throwDir = { 0, 0 };

		if (Input().TriggerKey(DIK_UP)) { throwDir = { 0, 1 }; tryThrow = true; }
		else if (Input().TriggerKey(DIK_DOWN)) { throwDir = { 0, -1 }; tryThrow = true; }
		else if (Input().TriggerKey(DIK_LEFT)) { throwDir = { -1, 0 }; tryThrow = true; }
		else if (Input().TriggerKey(DIK_RIGHT)) { throwDir = { 1, 0 }; tryThrow = true; }

		if (tryThrow) {
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
				if (!b && starCount_ > 0) {
					starCount_--;
					if (manager_) {
						Boomerang* starB = manager_->Spawn<Boomerang>(this, "Boomerang", this, true);
						starB->GetInfo().isActive = false;
						starB->GetInfo().isVisible = false;
						boomerangs_.push_back(starB);
						b = starB;
						Novice::ScreenPrintf(10, 100, "Created Star Boomerang!");
					}
					
				}

				if (b) {
					b->Throw(throwDir);
					if (Input().PressKey(DIK_DOWN)) {
						rigidbody_.acceleration.y += 100.f; // Jump on throw
					}
				}
			}
		}

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

	// ========== 更新・描画 ==========
	void Update(float deltaTime)override {
		HandleInput();
		Move(deltaTime);
		HandleBoomerang(deltaTime);

		drawComp_->SetFlipX(isflipX_);
		UpdateDrawComponent(deltaTime);

		//Novice::ConsolePrintf("isGrounded: %d\n", isGrounded_);
		if (Input().TriggerKey(DIK_U)) {
			Novice::ConsolePrintf("try spawn1\n");
			if (manager_) {
				Novice::ConsolePrintf("try spawn2\n");
				auto* enemy = manager_->Spawn<Enemy>(nullptr, "Enemy");
				enemy->SetPosition({transform_.translate.x, transform_.translate.y+50.f });
				enemy->Initialize();
			}
		}
	}

	void Draw(const Camera2D& camera)override {
		if (!info_.isActive || !info_.isVisible) return;
		// DrawComponent2Dを使って描画
		if (drawComp_) {
			drawComp_->Draw(camera);
		}

		// draw hitbox for debug
		Vector2 screenPos = const_cast<Camera2D&>(camera).WorldToScreen(transform_.translate);
		Vector2 colliderSize = const_cast<Vector2&>(collider_.size);
		Vector2 colliderOffset = const_cast<Vector2&>(collider_.offset);

		Novice::DrawBox(
			int(screenPos.x + colliderOffset.x - colliderSize.x/2.f),
			int(screenPos.y + colliderOffset.y),
			int(colliderSize.x), int(colliderSize.y),
			0.0f,
			0xFF0000FF, 
			kFillModeWireFrame
		);
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

};