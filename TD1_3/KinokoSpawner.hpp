#pragma once

#include "PhysicsObject.hpp"
#include "Enemy.hpp"
#include "AttackEnemy.hpp"

class KinokoSpawner : public PhysicsObject {
private:
	float spawnInterval_ = 300.0f; // スポーン間隔（フレーム数）
	float timer_ = 0.0f;
	int maxSpawned_ = 1; // 最大同時スポーン数
	std::vector<Enemy*> spawnedEnemies_;
public:
	KinokoSpawner() {
		drawComp_ = new DrawComponent2D();
		Initialize();
	}

	void Initialize() override {
		rigidbody_.Initialize();
		isGravityEnabled_ = false; // スポーナーは重力の影響を受けない
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Mystery), 4, 1, 4, 5.f, true);
		// 描画コンポーネントの初期化があれば呼ぶ
		if (drawComp_) {
			drawComp_->Initialize();
		}
	}
	void Update(float deltaTime) override {
		timer_ += deltaTime;
		if (timer_ >= spawnInterval_) {
			if (spawnedEnemies_.size() < maxSpawned_) {
				SpawnKinoko();
			}

			timer_ = 0.0f;

		}
		// スポーンしたキノコの状態をチェックし、非アクティブならリストから削除
		spawnedEnemies_.erase(
			std::remove_if(spawnedEnemies_.begin(), spawnedEnemies_.end(),
				[](Enemy* enemy) {
					// Remove if the pointer is null or the object is marked for death
					return enemy == nullptr || enemy->IsDead();
				}
			),
			spawnedEnemies_.end()
		);

		UpdateDrawComponent(deltaTime);

	}
	void SpawnKinoko() {
		// ここでゲームオブジェクトマネージャーに追加するコードが必要
		if (manager_) {
			Enemy* kinoko = manager_->Spawn<Enemy>(this, "Enemy");
			kinoko->SetPosition(this->GetPosition());
			kinoko->Initialize();
			spawnedEnemies_.push_back(kinoko);
		}

	}
};

class AttackKinokoSpawner : public PhysicsObject {
private:
	float spawnInterval_ = 300.0f; // スポーン間隔（フレーム数）
	float timer_ = 0.0f;
	int maxSpawned_ = 1; // 最大同時スポーン数
	std::vector<AttackEnemy*> spawnedEnemies_;
public:
	AttackKinokoSpawner() {
		drawComp_ = new DrawComponent2D();
		Initialize();
	}

	void Initialize() override {
		rigidbody_.Initialize();
		isGravityEnabled_ = false; // スポーナーは重力の影響を受けない
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Mystery), 4, 1, 4, 5.f, true);
		// 描画コンポーネントの初期化があれば呼ぶ
		if (drawComp_) {
			drawComp_->Initialize();
		}
	}
	void Update(float deltaTime) override {
		timer_ += deltaTime;
		if (timer_ >= spawnInterval_) {
			if (spawnedEnemies_.size() < maxSpawned_) {
				SpawnKinoko();
			}

			timer_ = 0.0f;

		}
		// スポーンしたキノコの状態をチェックし、非アクティブならリストから削除
		spawnedEnemies_.erase(
			std::remove_if(spawnedEnemies_.begin(), spawnedEnemies_.end(),
				[](AttackEnemy* enemy) {
					// Remove if the pointer is null or the object is marked for death
					return enemy == nullptr || enemy->IsDead();
				}
			),
			spawnedEnemies_.end()
		);

		UpdateDrawComponent(deltaTime);

	}
	void SpawnKinoko() {
		// ここでゲームオブジェクトマネージャーに追加するコードが必要
		if (manager_) {
			AttackEnemy* kinoko = manager_->Spawn<AttackEnemy>(this, "Enemy");
			kinoko->SetPosition(this->GetPosition());
			kinoko->Initialize();
			spawnedEnemies_.push_back(kinoko);
		}

	}
};