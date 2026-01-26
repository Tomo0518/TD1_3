#pragma once
#include "PhysicsObject.hpp"
#include "DrawComponentManager.hpp"

enum class AttackEnemyPhase {
	Patrolling,
	Battle,
};

enum class AttackEnemyBattleState {
	Idle,
	Running,
	Windup,
	Attacking,
};

class AttackEnemy : public PhysicsObject {
private:
	int direction_ = 1;         // 移動方向（1: 右, -1: 左）

	// 移動速度
	float moveSpeed_ = 2.0f;
	float runSpeed_ = 6.0f;

	// damaged handling
	float damagedShakingDuration_ = 20.0f; // ダメージ時の揺れ時間
	float damagedShakeTimer_ = 0.0f; // ダメージ時の揺れタイマー
	float damagedShakeMagnitude_ = 4.5f; // ダメージ時の揺れの大きさ
	Vector2 damagedShakeOffset_ = { 0.0f, 0.0f };
	bool isDamaged_ = false;

	// states
	AttackEnemyPhase state_ = AttackEnemyPhase::Patrolling;
	Vector2 initialPosition_;    // 初期位置
	bool stunned_ = false;

	// プレイヤー参照
	GameObject2D* playerRef_ = nullptr; // プレイヤー参照
	Vector2 playerPos_ = { 0.0f, 0.0f };
	float distanceToPlayer_ = 0.0f;
	float detectionRange_ = 500.0f; // プレイヤー検知範囲

	// stun state
	float stunDuration_ = 60.0f; // スタン状態の持続時間（フレーム数）
	float stunTimer_ = 0.0f;    // スタン状態のタイマー

	// patrol state
	float patrolRange_ = 200.0f; // パトロール範囲

	// battle state
	float attackRange_ = 140.0f; // 攻撃範囲
	float keepDistance_ = 110.0f; // プレイヤーとの距離維持
	float battleRange_ = 200.0f; // 戦闘維持範囲
	float escapeRange_ = 600.0f; // 戦闘離脱範囲

	float attackCooldown_ = 120.0f; // 攻撃クールダウン時間
	float attackTimer_ = 0.0f; // 攻撃タイマー

	float windDownDuration_ = 45.0f; // 攻撃後のクールダウン時間
	float windDownTimer_ = 0.0f; // クールダウンタイマー

	float windupDuration_ = 60.0f; // 攻撃の溜め時間
	float windupTimer_ = 0.0f; // 溜めタイマー
	float windupShakeMagnitude_ = 0.0f; // 溜め時の画面揺れの大きさ
	float windupShakeIncrement_ = 0.1f; // 画面揺れの増加量
	bool emitedCanAttackEffect_ = false;
	AttackEnemyBattleState battleState_ = AttackEnemyBattleState::Idle;

	// 描画マネージャー（全てのDrawComponent2Dを管理）
	DrawComponentManager drawManager_;

	// 描画コンポーネント
	//DrawComponent2D* StunnedComp_ = nullptr;
	//DrawComponent2D* PatrolComp_ = nullptr;
	//DrawComponent2D* AttackComp_ = nullptr;
	//DrawComponent2D* runComp_ = nullptr;
	//DrawComponent2D* windupComp_ = nullptr;
	//DrawComponent2D* battleIdleComp_ = nullptr;
public:
	AttackEnemy() {
		// 親クラスのdrawComp_を削除して無効化
		if (drawComp_) {
			delete drawComp_;
			drawComp_ = nullptr;
		}
	}

	~AttackEnemy() {
		drawComp_ = nullptr;
	}

	void Initialize() override {
		rigidbody_.Initialize();
		rigidbody_.deceleration = { 0.7f, 0.7f };
		collider_.size = { 80.f, 80.f };
		collider_.offset = { 0.f, -20.f };

		// マネージャーにコンポーネントを登録
		drawManager_.RegisterComponent("Patrol",
			new DrawComponent2D(Tex().GetTexture(TextureId::AttackKinokoWalk), 10, 1, 10, 5.f, true));
		drawManager_.RegisterComponent("Stunned",
			new DrawComponent2D(Tex().GetTexture(TextureId::AttackKinokoStun), 4, 1, 4, 5.f, false));
		drawManager_.RegisterComponent("Attack",
			new DrawComponent2D(Tex().GetTexture(TextureId::AttackKinokoAttack), 9, 1, 9, 5.f, false));
		drawManager_.RegisterComponent("Run",
			new DrawComponent2D(Tex().GetTexture(TextureId::AttackKinokoRun), 4, 1, 4, 5.f, true));
		drawManager_.RegisterComponent("Windup",
			new DrawComponent2D(Tex().GetTexture(TextureId::AttackKinokoWindup), 4, 1, 4, 5.f, false));
		drawManager_.RegisterComponent("BattleIdle",
			new DrawComponent2D(Tex().GetTexture(TextureId::AttackKinokoBattleIdle), 6, 1, 6, 5.f, true));

		// 全コンポーネントを初期化
		auto compNames = { "Patrol", "Stunned", "Attack", "Run", "Windup", "BattleIdle" };
		for (const auto& name : compNames) {
			if (auto* comp = drawManager_.GetComponent(name)) {
				comp->Initialize();
			}
		}

		initialPosition_ = transform_.translate;
		status_.maxHP = 15;
		status_.currentHP = status_.maxHP;
	}

	bool IsFacingPlayer() {
		if (!playerRef_) return false;
		return (direction_ == 1 && playerPos_.x >= transform_.translate.x) ||
			(direction_ == -1 && playerPos_.x <= transform_.translate.x);
	}

	void FindPlayer() {
		if (!playerRef_) {
			if (manager_) {
				playerRef_ = manager_->GetPlayerObject();
			}
		}

		if (playerRef_) {
			playerPos_ = playerRef_->GetPosition();
			if (IsFacingPlayer()) {
				distanceToPlayer_ = Vector2::Length(Vector2::Subtract(playerPos_, transform_.translate));

				if (distanceToPlayer_ <= detectionRange_) {
					state_ = AttackEnemyPhase::Battle;
				}
			}
		}
		else {
			ReturnToPatrol();
		}
	}



	void UpdateDrawComponent(float deltaTime) override {
		Vector2 renderPos;
		renderPos.x = transform_.translate.x + float(rand() % 100) / 100.f * windupShakeMagnitude_;
		renderPos.y = transform_.translate.y + float(rand() % 100) / 100.f * windupShakeMagnitude_;

		drawManager_.SetTransform(transform_);
		drawManager_.SetPosition(renderPos + damagedShakeOffset_);
		drawManager_.Update(deltaTime);
	}

	void Stunning(float deltaTime) {
		stunTimer_ -= deltaTime;
		if (stunTimer_ <= 0) {
			stunned_ = false;
		}
	}

	void Attack(float deltaTime) {
		// 攻撃処理
		windDownTimer_ += deltaTime;
		if (windDownTimer_ >= windDownDuration_) {
			windDownTimer_ = 0.0f;
			battleState_ = AttackEnemyBattleState::Idle;
		}

	}

	void BattleIdle(float deltaTime) {
		// 待機処理
		if (distanceToPlayer_ > battleRange_) {
			battleState_ = AttackEnemyBattleState::Running;
			return;
		}

		attackTimer_ += deltaTime;

		direction_ = (playerPos_.x >= transform_.translate.x) ? 1 : -1;

		if (distanceToPlayer_ < keepDistance_) {
			// プレイヤーから離れる処理
			Vector2 directionAwayFromPlayer = Vector2::Subtract(transform_.translate, playerPos_);
			directionAwayFromPlayer = Vector2::Normalize(directionAwayFromPlayer);
			transform_.translate.x += directionAwayFromPlayer.x * moveSpeed_;
			return;
		}

		if (distanceToPlayer_ > attackRange_) {
			// プレイヤーに近づく処理
			Vector2 directionToPlayer = Vector2::Subtract(playerPos_, transform_.translate);
			directionToPlayer = Vector2::Normalize(directionToPlayer);
			transform_.translate.x += directionToPlayer.x * moveSpeed_;
			return;
		}

		if (distanceToPlayer_ <= attackRange_) {
			// 攻撃範囲内
			if (attackTimer_ >= attackCooldown_) {
				attackTimer_ = 0.0f;
				battleState_ = AttackEnemyBattleState::Windup;
				windupTimer_ = 0.0f;
			}
		}

	}

	void Windup(float deltaTime) {
		windupTimer_ += deltaTime;

		if (!emitedCanAttackEffect_) {
			ParticleManager::GetInstance().Emit(ParticleType::Enemy_CanStan, transform_.translate);
			emitedCanAttackEffect_ = true;

			// エフェクトを開始
			drawManager_.StartFlashBlink(0xFFFFFFFF, 4, 0.3f, BlendMode::kBlendModeAdd, 2);
		}

		if (windupTimer_ >= windupDuration_ / 2) {
			windupShakeMagnitude_ += windupShakeIncrement_ * deltaTime;
		}

		if (windupTimer_ >= windupDuration_) {
			battleState_ = AttackEnemyBattleState::Attacking;
			windupTimer_ = 0.0f;
			windupShakeMagnitude_ = 0.0f;
			emitedCanAttackEffect_ = false;
		}
	}

	void RunTowardsPlayer(float deltaTime) {
		// プレイヤーに向かって走る
		Vector2 directionToPlayer = Vector2::Subtract(playerPos_, transform_.translate);
		directionToPlayer = Vector2::Normalize(directionToPlayer);
		transform_.translate.x += directionToPlayer.x * runSpeed_ * deltaTime;

		direction_ = (directionToPlayer.x >= 0) ? 1 : -1;

		if (distanceToPlayer_ <= battleRange_) {
			battleState_ = AttackEnemyBattleState::Idle;
		}
	}

	void ChangeDrawComponent(DrawComponent2D* newComp) {
		if (drawComp_ != newComp) {
			drawComp_ = newComp;
			drawComp_->PlayAnimation();
		}
	}

	void Update(float deltaTime) override {

		Behavior(deltaTime);
		Move(deltaTime);
		damageHandling(deltaTime);
		UpdateDrawComponent(deltaTime);
	}

	void Behavior(float deltaTime) {
		drawManager_.SetFlipX(direction_ == 1);
		FindPlayer();

		if (stunned_) {
			drawManager_.ChangeComponent("Stunned");
			Stunning(deltaTime);
			return;
		}

		if (state_ == AttackEnemyPhase::Patrolling) {
			Patrol(deltaTime);
		}
		else if (state_ == AttackEnemyPhase::Battle) {
			BattleBehavior(deltaTime);
		}
	}

	
	void ReturnToPatrol() {
		state_ = AttackEnemyPhase::Patrolling;
		battleState_ = AttackEnemyBattleState::Idle;
		attackTimer_ = 0.0f;
	}

	void BattleBehavior(float deltaTime) {
		if (distanceToPlayer_ > escapeRange_) {
			ReturnToPatrol();
			return;
		}

		switch (battleState_) {
		case AttackEnemyBattleState::Idle:
			drawManager_.ChangeComponent("BattleIdle");
			BattleIdle(deltaTime);
			break;
		case AttackEnemyBattleState::Running:
			drawManager_.ChangeComponent("Run");
			RunTowardsPlayer(deltaTime);
			break;
		case AttackEnemyBattleState::Windup:
			drawManager_.ChangeComponent("Windup");
			Windup(deltaTime);
			break;
		case AttackEnemyBattleState::Attacking:
			drawManager_.ChangeComponent("Attack");
			Attack(deltaTime);
			break;
		}
	}


	void Stun() {
		stunned_ = true;
		stunTimer_ = stunDuration_;
		drawManager_.StartFlashBlink(0x0000FFCC, 2, 0.1f,BlendMode::kBlendModeNormal, 1);
		ParticleManager::GetInstance().Emit(ParticleType::Charge, transform_.translate);

		attackTimer_ = 0.0f;
		battleState_ = AttackEnemyBattleState::Idle;

	}

	void Patrol(float deltaTime) {
		drawManager_.ChangeComponent("Patrol");

		// パトロール範囲内で移動
		transform_.translate.x += direction_ * moveSpeed_ * deltaTime;
		//Novice::ConsolePrintf( "id: %d ,Enemy Pos X: %.2f ", info_.id,transform_.translate.x);
		//Novice::ConsolePrintf("Initial Pos X: %.2f, overPatrol: %d\n", initialPosition_.x, transform_.translate.x > initialPosition_.x + patrolRange_ || transform_.translate.x < initialPosition_.x - patrolRange_);
		if (transform_.translate.x > initialPosition_.x + patrolRange_) {
			direction_ = -1; // 左に移動
		}
		else if (transform_.translate.x < initialPosition_.x - patrolRange_) {
			direction_ = 1; // 右に移動
		}

		//find the bottomRight and bottomLeft haveing blocks
		auto& mapData = MapData::GetInstance();
		float tileSize = 64.0f; // Adjust this to your actual tile size (e.g. 64.f)

		Vector2 bottomLeft = { transform_.translate.x - collider_.offset.x - collider_.size.x, transform_.translate.y - collider_.offset.y - collider_.size.y * 1.25f };
		Vector2 bottomRight = { transform_.translate.x - collider_.offset.x + collider_.size.x, transform_.translate.y - collider_.offset.y - collider_.size.y * 1.25f };


		int leftTileX = static_cast<int>(bottomLeft.x / tileSize);
		int rightTileX = static_cast<int>(bottomRight.x / tileSize);
		int bottomTileY = static_cast<int>(bottomLeft.y / tileSize);

		//check if cornor tile dont have block, change direction
		int LBtileID = mapData.GetTile(leftTileX, bottomTileY, TileLayer::Block);
		const TileDefinition* LBDef = TileRegistry::GetTile(LBtileID);
		if (LBDef == nullptr || !LBDef->isSolid) {
			direction_ = 1; // Change direction to right
		}

		int RBtileID = mapData.GetTile(rightTileX, bottomTileY, TileLayer::Block);
		const TileDefinition* RBDef = TileRegistry::GetTile(RBtileID);
		if (RBDef == nullptr || !RBDef->isSolid) {
			direction_ = -1; // Change direction to left
		}
	}

	void Draw(const Camera2D& camera) override {
		if (!info_.isActive || !info_.isVisible) return;
		drawManager_.Draw(camera);
	}

	void damageHandling(float deltaTime) {
		if (isDamaged_) {
			damagedShakeTimer_ += deltaTime;
			if (damagedShakeTimer_ >= damagedShakingDuration_) {
				isDamaged_ = false;
				damagedShakeTimer_ = 0.0f;
				damagedShakeOffset_ = { 0.0f, 0.0f };
			}
			else {
				// Apply shaking effect
				float shakeX = (rand() % 100 / 100.0f - 0.5f) * 2.0f * damagedShakeMagnitude_;
				float shakeY = (rand() % 100 / 100.0f - 0.5f) * 2.0f * damagedShakeMagnitude_;
				damagedShakeOffset_ = { shakeX, shakeY };
			}
		}
	}

	void SpawnStar() {
		Star* star = manager_->Spawn<Star>(this, "Star");
		star->SetPosition(transform_.translate);
		star->SetOwner(this);
	}

	virtual void OnDamaged(int damage) override {
		if (damage < 0) return; // 負のダメージは無効
		status_.currentHP -= damage;
		if (status_.currentHP <= 0) {
			status_.currentHP = 0;
			ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
			ParticleManager::GetInstance().Emit(ParticleType::Enemy_Dead, transform_.translate);
			Destroy();
		}
		else {
			if (damage > 0) {
				isDamaged_ = true;
				drawManager_.StartFlashBlink(0xFF0000CC, 4, 0.3f, BlendMode::kBlendModeNormal, 1);
				damagedShakeTimer_ = 0.0f;

				ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
			}


			if (battleState_ == AttackEnemyBattleState::Windup) {
				Stun();

				SpawnStar();
			}
		}
	}

	virtual int OnCollision(GameObject2D* other) override {
		if (other->GetInfo().tag == "Boomerang") {
			direction_ = (transform_.translate.x > other->GetTransform().translate.x) ? -1 : 1;
		}
		if (other->GetInfo().tag == "Enemy") {
			// knockback on collision with other enemies
			Vector2 knockbackDir = Vector2::Subtract(transform_.translate, other->GetTransform().translate);
			knockbackDir = Vector2::Normalize(knockbackDir);
			rigidbody_.acceleration.x += knockbackDir.x * 7.5f;

			if (state_ == AttackEnemyPhase::Patrolling) {
				// reverse direction
				direction_ *= -1;
			}

		}

		return 0;
	}

};