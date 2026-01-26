#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "GameObject2D.h"
#include "GameObjectManager.h"
#include "PhysicsManager.h"
#include "SceneUtilityIncludes.h"

class PhysicsObject : public GameObject2D {
protected:
	bool isGrounded_ = false; // 地面に接地しているかどうか
	bool isGravityEnabled_ = true; // 重力の影響を受けるかどうか

public:
	PhysicsObject() {		
		//drawComp_ = new DrawComponent2D();
		Initialize();		
	}
	~PhysicsObject() {
		delete drawComp_;
	}

	void Initialize() override {
		rigidbody_.Initialize();
		// 描画コンポーネントの初期化があれば呼ぶ


		if (drawComp_) {
			drawComp_->Initialize();
		}
	}

	// ========== 更新・描画 ==========
	virtual void Update(float deltaTime)override {
		Move(deltaTime);
		UpdateDrawComponent(deltaTime);
	}

	virtual void Draw(const Camera2D& camera)override {
		if (!info_.isActive || !info_.isVisible) return;

		// DrawComponent2Dを使って描画
		if (drawComp_) {
			drawComp_->Draw(camera);
		}
	}
	
	// ========== 移動 ==========
	virtual void Move(float deltaTime) {
		
		if (isGravityEnabled_) {
			// 重力を加える
			//if (!isGrounded_) {
				const Vector2 gravity = { 0.0f, -1.f }; // 下方向に980ピクセル/秒²の重力
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
		PhysicsManager::ResolveMapCollision(this, mapData);		
		if (isGrounded_)  transform_.translate.y -= 2;
		// Apply rotation (if any)
		transform_.rotation += rigidbody_.GetRotationDelta(deltaTime);

		

		transform_.CalculateWorldMatrix();

	}

	virtual void UpdateDrawComponent(float deltaTime) {
		if (drawComp_) {
			drawComp_->SetTransform(transform_);
			drawComp_->Update(deltaTime);
		}
	}

	// ========== ゲッター ==========
	Vector2 GetPosition() const { return transform_.translate; }
	Vector2 GetVelocity() const { return rigidbody_.velocity; }
	bool IsAlive() const { return info_.isActive; }

	// 位置への const 参照を返すメソッド
	const Vector2& GetPositionRef() const { return transform_.translate; }


	// ========== セッター ==========
	void SetPosition(const Vector2& pos) { transform_.translate = pos; }
	void SetAlive(bool alive) { info_.isActive = alive; }

	bool IsOnGround() const {
		return isGrounded_;
	}
	bool IsGravityEnabled() const {
		return isGravityEnabled_;
	}
};