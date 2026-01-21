#pragma once
#include "PhysicsObject.hpp"
#include "algorithm"

enum class BoomerangState {
	Thrown,
	Returning,
	Idle
};

class Boomerang : public PhysicsObject {
private:
	BoomerangState state_;
	GameObject2D* owner_;
	Vector2 startPos_;
	Vector2 targetPos_;
	float moveTimer_;
	float maxTime_;
	float stayTimer_; // Timer for staying at target
	float maxStayTime_; // Duration to stay
	Vector2 farestDistance_;
	bool isHorizontal_;
	std::vector<int> hitEnemies_; // Track hit enemy IDs
	bool isTemporary_;
public:
	Boomerang(GameObject2D* owner, bool isTemp) {
		state_ = BoomerangState::Idle;
		owner_ = owner;
		maxTime_ = 0.5f;
		isTemporary_ = isTemp;
		stayTimer_ = 0.0f;
		maxStayTime_ = 0.5f;
		collider_.offset = { 0.0f, -20.0f };
		info_.isVisible = false;
		info_.isActive = false;

		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang), 4, 1, 4, 0.1f, true);
		drawComp_->Initialize();
	}

	~Boomerang() {}


	void Throw(Vector2 direction) {
		if (state_ != BoomerangState::Idle) return;

		state_ = BoomerangState::Thrown;
		info_.isActive = true;
		info_.isVisible = true;
		startPos_ = owner_->GetTransform().translate;
		transform_.translate = startPos_;
		moveTimer_ = 0;
		maxTime_ = 0.5f; // Half second to go out
		hitEnemies_.clear(); // Reset hit history
		stayTimer_ = 0.0f; // Reset stay timer
		float range = 380.0f;


		// Determine if horizontal or vertical throw
		if (abs(direction.x) > abs(direction.y)) {
			isHorizontal_ = true;
			targetPos_ = startPos_ + direction * range;
		}
		else {
			isHorizontal_ = false;
			targetPos_ = startPos_ + direction * range;
		}

		// Determine primary axis and normalize direction for raycasting
		Vector2 castDir = { 0.0f, 0.0f };

		// Determine if horizontal or vertical throw
		if (abs(direction.x) > abs(direction.y)) {
			isHorizontal_ = true;
			// Force strict horizontal direction (1 or -1)
			castDir.x = (direction.x > 0) ? 1.0f : -1.0f;
			targetPos_ = startPos_ + castDir * range;
		}
		else {
			isHorizontal_ = false;
			// Force strict vertical direction (1 or -1)
			castDir.y = (direction.y > 0) ? 1.0f : -1.0f;
			targetPos_ = startPos_ + castDir * range;
		}

		// === Raycast Logic ===
		auto& mapData = MapData::GetInstance();
		float tileSize = 64.0f; // Adjust this to your actual tile size (e.g. 64.f)
		float stepSize = tileSize / 2.0f; // Check every half-tile
		float dist = 0.0f;
		farestDistance_ = targetPos_;
		while (dist < range) {
			dist += stepSize;
			if (dist > range) dist = range;

			Vector2 checkPos = startPos_ + castDir * dist;

			// Convert world position to tile index
			int tx = static_cast<int>(checkPos.x / tileSize);
			int ty = static_cast<int>(checkPos.y / tileSize);

			// Check if tile is solid (assuming non-zero is solid)
			// Adjust 'TileLayer::Collision' to the correct layer enum for your game

			int tileID = mapData.GetTile(tx, ty, TileLayer::Block);
			if (tileID != 0) {
				const TileDefinition* def = TileRegistry::GetTile(tileID);
				if (def->isSolid) {
					// Determine collision point (snap to tile edge or use checkPos)
					farestDistance_ = checkPos;
					farestDistance_.x -= castDir.x * (tileSize );
					farestDistance_.y -= castDir.y * (tileSize );

					// Optional: Reduce travel time if distance is very short
					// maxTime_ = maxTime_ * (dist / range);
					break;

				}
			}

			if (dist >= range) break;
		}
	}

	float EaseOutCubic(float t) {
		float f = 1.0f - t;
		return 1.0f - f * f * f;
	}

	// float EaseInCubic(float t) {
	//    return t * t * t;
	// }

	void Update(float deltaTime = 1.0f) override {
		if (state_ == BoomerangState::Idle) {
			info_.isActive = false;
			info_.isVisible = false;
			return;
		}

		transform_.rotation -= 15.f * deltaTime;

		// Custom movement logic based on provided requirements
		Vector2 ownerPos = owner_->GetTransform().translate;

		if (state_ == BoomerangState::Thrown) {
			if (moveTimer_ < maxTime_) {
				moveTimer_ += deltaTime;
				float t = moveTimer_ / maxTime_;
				if (t > 1.0f) t = 1.0f;

				// Go to target
				float easedT = EaseOutCubic(t);
				Vector2 currentTarget = startPos_ + (targetPos_ - startPos_) * easedT;

				if (isHorizontal_) {
					// x goes to target, y follows player
					transform_.translate.x = currentTarget.x;
					transform_.translate.y = ownerPos.y;
					//clamp x to farest distance
					transform_.translate.x = std::clamp(transform_.translate.x,
						std::min(startPos_.x, farestDistance_.x),
						max(startPos_.x, farestDistance_.x));
				}
				else {
					// y goes to target, x follows player
					transform_.translate.y = currentTarget.y;
					transform_.translate.x = ownerPos.x;

					// clamp y to farest distance
					transform_.translate.y = std::clamp(transform_.translate.y,
						std::min(startPos_.y, farestDistance_.y),
						max(startPos_.y, farestDistance_.y));
				}
			}
			else {
				// Stay logic
				if (stayTimer_ < maxStayTime_) {
					stayTimer_ += deltaTime;
					// Still follow player while staying
					if (isHorizontal_) {
						transform_.translate.x = farestDistance_.x;
						transform_.translate.y = ownerPos.y;
					}
					else {
						transform_.translate.y = farestDistance_.y;
						transform_.translate.x = ownerPos.x;
					}
				}
				else {
					state_ = BoomerangState::Returning;
					moveTimer_ = 0;
					stayTimer_ = 0;
					// Update target pos to be current pos for smooth return calculation if needed
					targetPos_ = transform_.translate;
				}
			}
		}
		else if (state_ == BoomerangState::Returning) {

			// Simple return logic: Move towards player
			Vector2 dir = ownerPos - transform_.translate;
			if (Vector2::Length(dir) < 20.0f) {
				state_ = BoomerangState::Idle;
			}
			else {
				dir = Vector2::Normalize(dir);
				float speed = 15.0f;

				if (isHorizontal_) {
					// Only x moves back, y matches player
					transform_.translate.x += dir.x * speed;
					transform_.translate.y = ownerPos.y;
				}
				else {
					// Only y moves back, x matches player
					transform_.translate.y += dir.y * speed;
					transform_.translate.x = ownerPos.x;
				}
			}
		}

		GameObject2D::Update(deltaTime);

		// clamp to farthest distance
		Novice::ScreenPrintf(10, 60, "Farest X: %.2f Y: %.2f", farestDistance_.x, farestDistance_.y);
		Novice::ScreenPrintf(10, 80, "Current X: %.2f Y: %.2f", transform_.translate.x, transform_.translate.y);
		if (isHorizontal_) {
			transform_.translate.x = std::clamp(transform_.translate.x,
				std::min(ownerPos.x, farestDistance_.x),
				max(ownerPos.x, farestDistance_.x));
		}
		else {
			transform_.translate.y = std::clamp(transform_.translate.y,
				std::min(ownerPos.y, farestDistance_.y),
				max(ownerPos.y, farestDistance_.y));
		}

		if (IsTemporary() && IsIdle()) {
			isDead_ = true;
		}

	}

	bool IsIdle() const { return state_ == BoomerangState::Idle; }
	bool IsTemporary() const { return isTemporary_; }
	// void Draw(const Camera2D& camera) override;

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
	}

	int OnCollision(GameObject2D* other) override {
		// Check if other is an enemy
		if (other->GetInfo().tag == "Enemy") {
			other->Destroy();
		}
		return 0;
	}
};
