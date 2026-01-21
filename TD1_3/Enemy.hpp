#pragma once
#include "PhysicsObject.hpp"

class Enemy : public PhysicsObject {
private:
	float patrolRange_ = 200.0f; // パトロール範囲
	Vector2 initialPosition_;    // 初期位置
	int direction_ = 1;         // 移動方向（1: 右, -1: 左）
	float moveSpeed_ = 2.0f;
public:
	Enemy() {
		drawComp_ = nullptr;
		//Initialize();
	}
	~Enemy() {
	}
	void Initialize() override {
		rigidbody_.Initialize();
		rigidbody_.deceleration = { 0.7f, 0.7f };
		collider_.size = { 80.f, 80.f };
		collider_.offset = { 0.f, -20.f };
		// 描画コンポーネントの初期化があれば呼ぶ
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::KinokoWalk), 10, 1, 10, 5.f, true);
		drawComp_->Initialize();
		initialPosition_ = transform_.translate;
	}

	void Update(float deltaTime) override {
		Patrol(deltaTime);
		UpdateDrawComponent(deltaTime);
	}

	void Patrol(float deltaTime) {
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

		Vector2 bottomLeft = { transform_.translate.x - collider_.offset.x - collider_.size.x, transform_.translate.y - collider_.offset.y - collider_.size.y * 1.5f };
		Vector2 bottomRight = { transform_.translate.x - collider_.offset.x + collider_.size.x, transform_.translate.y - collider_.offset.y - collider_.size.y * 1.5f };


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

		Move(deltaTime);

		drawComp_->SetFlipX(direction_ == 1);

		// Update world matrix
		transform_.CalculateWorldMatrix();
	}

	void Draw(const Camera2D& camera) override {
		if (!info_.isActive || !info_.isVisible) return;
		// DrawComponent2Dを使って描画
		if (drawComp_) {
			drawComp_->Draw(camera);
		}
		Vector2 screenPos = const_cast<Camera2D&>(camera).WorldToScreen(transform_.translate);
		Vector2 colliderSize = const_cast<Vector2&>(collider_.size);
		Vector2 colliderOffset = const_cast<Vector2&>(collider_.offset);

		Novice::DrawBox(
			int(screenPos.x + colliderOffset.x - colliderSize.x / 2.f),
			int(screenPos.y + colliderOffset.y),
			int(colliderSize.x), int(colliderSize.y),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);
		Vector2 bottomLeft = { transform_.translate.x - collider_.offset.x - collider_.size.x, transform_.translate.y - collider_.offset.y - collider_.size.y*1.5f };
		Vector2 screenPos3 = const_cast<Camera2D&>(camera).WorldToScreen(bottomLeft);

		Novice::DrawBox(
			int(screenPos3.x -5),
			int(screenPos3.y -5),
			int(10), int(10),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);

		Vector2 bottomRight = { transform_.translate.x - collider_.offset.x + collider_.size.x, transform_.translate.y - collider_.offset.y - collider_.size.y * 1.5f };
		Vector2 screenPos2 = const_cast<Camera2D&>(camera).WorldToScreen(bottomRight);

		Novice::DrawBox(
			int(screenPos2.x - 5),
			int(screenPos2.y - 5),
			int(10), int(10),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);

	}
};