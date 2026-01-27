#pragma once
#include "AttackEnemy.hpp"

class FatEnemyHitBox : public PhysicsObject {
	float lifetime_ = 15.f;
	float offsetXAmount_ = 64.f;
	float offsetX_ = offsetXAmount_;
public:
	FatEnemyHitBox() {
		delete drawComp_;
		drawComp_ = nullptr;
		//Initialize();
	}
	~FatEnemyHitBox() {
	}

	void Initialize() override {
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = true;
		collider_.size = { 140.f, 160.f };
		collider_.offset = { 0.f, 0.f };
		isGravityEnabled_ = false;
	}

	int OnCollision(GameObject2D* other) override {
		if (other->GetInfo().tag == "Player") {
			// Here you can add logic to damage the player
			Novice::ConsolePrintf("\n!!!!!!Player Hit by AttackEnemy!!!!!!!!!\n");
			info_.isActive = false; // Deactivate hitbox after hit

			Vector2 knockbackDir = Vector2::Subtract(transform_.translate, other->GetTransform().translate);
			knockbackDir = Vector2::Normalize(knockbackDir) * -1.f;
			other->GetRigidbody().acceleration.x += knockbackDir.x * 14.5f;
			other->GetRigidbody().acceleration.y += 6.5f;

			other->OnDamaged(10); // Deal 10 damage to player

			Destroy();

		}
		return 0;
	}

	void Draw(const Camera2D& camera) override {
		// Draw hitbox for debug
		Novice::ConsolePrintf("transform: %f, %f\n", transform_.translate.x, transform_.translate.y);
		Vector2 screenPos = const_cast<Camera2D&>(camera).WorldToScreen(transform_.translate);
		Vector2 colliderSize = const_cast<Vector2&>(collider_.size);
		Vector2 colliderOffset = const_cast<Vector2&>(collider_.offset);
		Novice::DrawBox(
			int(screenPos.x + colliderOffset.x - colliderSize.x / 2.f),
			int(screenPos.y + colliderOffset.y - colliderSize.y / 2.f),
			int(colliderSize.x), int(colliderSize.y),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);
	}

	void Update(float deltaTime) override {

		lifetime_ -= deltaTime;
		if (lifetime_ <= 0.f) {
			info_.isActive = false; // Deactivate hitbox after lifetime ends
			Destroy();
			return;
		}

		if (owner_) {
			// Follow owner position with offset
			Vector2 ownerPos = owner_->GetTransform().translate;
			float directionX = owner_->GetRigidbody().velocity.x;
			if (directionX < 0) {
				offsetX_ = -offsetXAmount_;
			}
			else if (directionX > 0) {
				offsetX_ = offsetXAmount_;
			}
			transform_.translate = { ownerPos.x + offsetX_, ownerPos.y };
		}
		
	}

	void setLifetime(float time) {
		lifetime_ = time;
	}

};

class FatEnemy : public AttackEnemy {
	float windDownTwoDuration_ = 25.0f; // 2回目の攻撃後のクールダウン時間
	float windDownTwoTimer_ = 0.0f; // 2回目のクールダウンタイマー

	float chargeForce_ = 15.0f; // 突進力

	enum  FatEnemyDrawState {
		ePatrol,
		eStunned,
		eAttack,
		eWindup,
		eBattleIdle,
		eRun,
		eAttack2,
	};

public:
	FatEnemy() {
		direction_ = 1;         // 移動方向（1: 右, -1: 左）

		// 移動速度
		moveSpeed_ = 2.0f;
		runSpeed_ = 4.0f;

		// damaged handling
		damagedShakingDuration_ = 20.0f; // ダメージ時の揺れ時間
		damagedShakeTimer_ = 0.0f; // ダメージ時の揺れタイマー
		damagedShakeMagnitude_ = 4.5f; // ダメージ時の揺れの大きさ
		damagedShakeOffset_ = { 0.0f, 0.0f };
		isDamaged_ = false;

		// states
		state_ = AttackEnemyPhase::Patrolling;
		initialPosition_;    // 初期位置
		stunned_ = false;

		// プレイヤー参照
		playerRef_ = nullptr; // プレイヤー参照
		playerPos_ = { 0.0f, 0.0f };
		distanceToPlayer_ = 0.0f;
		detectionRange_ = 500.0f; // プレイヤー検知範囲

		// stun state
		stunDuration_ = 60.0f; // スタン状態の持続時間（フレーム数）
		stunTimer_ = 0.0f;    // スタン状態のタイマー

		// patrol state
		patrolRange_ = 200.0f; // パトロール範囲

		// battle state
		attackRange_ = 140.0f; // 攻撃範囲
		keepDistance_ = 110.0f; // プレイヤーとの距離維持
		battleRange_ = 200.0f; // 戦闘維持範囲
		escapeRange_ = 600.0f; // 戦闘離脱範囲

		attackCooldown_ = 60.0f; // 攻撃クールダウン時間
		attackTimer_ = 0.0f; // 攻撃タイマー

		windDownDuration_ = 30.0f; // 攻撃後のクールダウン時間
		windDownTimer_ = 0.0f; // クールダウンタイマー

		windupDuration_ = 60.0f; // 攻撃の溜め時間
		windupTimer_ = 0.0f; // 溜めタイマー
		windupShakeMagnitude_ = 0.0f; // 溜め時の画面揺れの大きさ
		windupShakeIncrement_ = 0.1f; // 画面揺れの増加量
		emitedCanAttackEffect_ = false;
		battleState_ = AttackEnemyBattleState::Idle;


		DrawOffset_ = { -64.f, 50.f };
	}

	virtual void Initialize() override {
		rigidbody_.Initialize();
		rigidbody_.deceleration = { 0.9f, 0.9f };
		collider_.size = { 140.f, 200.f };
		collider_.offset = { 0.f, 0.f };

		rigidbody_.maxSpeedX = chargeForce_;

		// マネージャーにコンポーネントを登録
		drawManager_.RegisterComponent(FatEnemyDrawState::ePatrol,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyWalk), 8, 1, 8, 5.f, true));
		drawManager_.RegisterComponent(FatEnemyDrawState::eStunned,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyStun), 3, 1, 3, 5.f, false));
		drawManager_.RegisterComponent(FatEnemyDrawState::eAttack,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyAttack), 6, 1, 6, 5.f, false));
		drawManager_.RegisterComponent(FatEnemyDrawState::eRun,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyRun), 8, 1, 8, 3.f, true));
		drawManager_.RegisterComponent(FatEnemyDrawState::eWindup,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyWindup), 4, 1, 4, 5.f, false));
		drawManager_.RegisterComponent(FatEnemyDrawState::eBattleIdle,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyBattleIdle), 6, 1, 6, 5.f, true));
		drawManager_.RegisterComponent(FatEnemyDrawState::eAttack2,
			new DrawComponent2D(Tex().GetTexture(TextureId::FatEnemyAttack2), 5, 1, 5, 5.f, false));

		// 全コンポーネントを初期化
		auto compNames = { FatEnemyDrawState::ePatrol, FatEnemyDrawState::eStunned, FatEnemyDrawState::eAttack, FatEnemyDrawState::eRun, FatEnemyDrawState::eWindup, FatEnemyDrawState::eBattleIdle, FatEnemyDrawState::eAttack2 };
		for (const auto& name : compNames) {
			if (auto* comp = drawManager_.GetComponent(name)) {
				comp->Initialize();
			}
		}

		initialPosition_ = transform_.translate;
		status_.maxHP = 45;
		status_.currentHP = status_.maxHP;
	}

	virtual void SpawnStar() override {
		for (int i = 0; i < 2; ++i) {
			Star* star = manager_->Spawn<Star>(this, "Star");
			star->SetPosition(transform_.translate);
			star->SetOwner(this);
		}
		
	}

	virtual void Windup(float deltaTime) {
		windupTimer_ += deltaTime;

		if (!emitedCanAttackEffect_) {
			ParticleManager::GetInstance().Emit(ParticleType::Enemy_CanStan, transform_.translate);
			emitedCanAttackEffect_ = true;

			// エフェクトを開始
			drawManager_.StartFlashBlink(0xFFFFFFFF, 4, windupDuration_/8.f/60.f, BlendMode::kBlendModeAdd, 2);
		}

		if (windupTimer_ >= windupDuration_ / 2) {
			windupShakeMagnitude_ += windupShakeIncrement_ * deltaTime;
		}

		if (windupTimer_ >= windupDuration_) {
			battleState_ = AttackEnemyBattleState::Attacking;
			windupTimer_ = 0.0f;
			windupShakeMagnitude_ = 0.0f;
			emitedCanAttackEffect_ = false;

			rigidbody_.AddForce({ direction_ * chargeForce_, 0.0f });

			// Create and position the hitbox
			SpawnHitBox(15.f);
		}
	}

	virtual void SpawnHitBox(float lifetime) {
		FatEnemyHitBox* hitbox = manager_->Spawn<FatEnemyHitBox>(this, "FatEnemyHitBox");
		hitbox->SetPosition(transform_.translate + Vector2(64.f, 0.f) * float(direction_));
		hitbox->setLifetime(lifetime);
		hitbox->SetOwner(this);
	}

	void Attack(float deltaTime)  override{
		windDownTimer_ += deltaTime;
		if (windDownTimer_ >= windDownDuration_) {
			battleState_ = AttackEnemyBattleState::Attacking2;
			windDownTimer_ = 0.0f;
			SpawnHitBox(15.f);

			rigidbody_.AddForce({ direction_ * chargeForce_, 0.0f });
		}
	}

	void Attack2(float deltaTime) {
		windDownTwoTimer_ += deltaTime;
		if (windDownTwoTimer_ >= windDownTwoDuration_) {
			battleState_ = AttackEnemyBattleState::Idle;
			windDownTwoTimer_ = 0.0f;
			// Create and position the hitbox
			
		}
	}

	virtual void BattleBehavior(float deltaTime) override {
		if (distanceToPlayer_ > escapeRange_) {
			ReturnToPatrol();
			return;
		}

		switch (battleState_) {
		case AttackEnemyBattleState::Idle:
			drawManager_.ChangeComponent(FatEnemyDrawState::eBattleIdle);
			BattleIdle(deltaTime);
			break;
		case AttackEnemyBattleState::Running:
			drawManager_.ChangeComponent(FatEnemyDrawState::eRun);
			RunTowardsPlayer(deltaTime);
			break;
		case AttackEnemyBattleState::Windup:
			drawManager_.ChangeComponent(FatEnemyDrawState::eWindup);
			Windup(deltaTime);
			break;
		case AttackEnemyBattleState::Attacking:
			drawManager_.ChangeComponent(FatEnemyDrawState::eAttack);
			Attack(deltaTime);
			break;
		case AttackEnemyBattleState::Attacking2:
			drawManager_.ChangeComponent(FatEnemyDrawState::eAttack2);
			Attack2(deltaTime);
			break;
		}
	}

	void Draw(const Camera2D& camera) override {
		if (!info_.isActive || !info_.isVisible) return;

		drawManager_.Draw(camera);

		Vector2 screenPos = const_cast<Camera2D&>(camera).WorldToScreen(transform_.translate);
		Vector2 colliderSize = const_cast<Vector2&>(collider_.size);
		Vector2 colliderOffset = const_cast<Vector2&>(collider_.offset);

		Novice::DrawBox(
			int(screenPos.x + colliderOffset.x - colliderSize.x / 2.f),
			int(screenPos.y + colliderOffset.y - colliderSize.y / 2.f),
			int(colliderSize.x), int(colliderSize.y),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);

		Novice::DrawBox(
			int(screenPos.x - 5.f),
			int(screenPos.y -5.f),
			10, 10,
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);
	}
};