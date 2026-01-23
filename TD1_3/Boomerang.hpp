#pragma once
#include "PhysicsObject.hpp"
#include "algorithm"
#include "Star.hpp"

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
	float defaultMaxStayTime_ = 20.f;
	Vector2 farestDistance_;
	bool isHorizontal_;
	std::vector<int> hitEnemies_; // Track hit enemy IDs
	bool isTemporary_;
	int starCount_;
	int damage_;
	int damageBonus_ = 0;
	float delayPerStar = 18;
	float range_ = 560.0f;
	Vector2 RenderPos_;

	DrawComponent2D* starComp_ = nullptr;
	DrawComponent2D* effectComp_ = nullptr;
public:
	Boomerang(GameObject2D* owner, bool isTemp) {
		state_ = BoomerangState::Idle;
		owner_ = owner;
		maxTime_ = 30.f;
		isTemporary_ = isTemp;
		stayTimer_ = 0.0f;
		maxStayTime_ = defaultMaxStayTime_;
		collider_.offset = { 0.0f, -20.0f };
		info_.isVisible = false;
		info_.isActive = false;

		delete drawComp_;
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang), 4, 1, 4, 5.f, true);
		drawComp_->Initialize();

		starComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Star_shooting), 4, 1, 4, 5.f, true);
		starComp_->Initialize();

		effectComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang_Charged), 4, 1, 4, 5.f, true);
		effectComp_->Initialize();
	}

	~Boomerang() {
		delete starComp_;
		delete effectComp_;
	}


	void Throw(Vector2 direction, int Star = 0) {
		if (state_ != BoomerangState::Idle) return;
		starCount_ = Star;
		
		maxStayTime_ = defaultMaxStayTime_ + starCount_ * delayPerStar; // Each star adds 2 seconds

		damage_ = 0;
		damageBonus_ = 0;

		state_ = BoomerangState::Thrown;
		info_.isActive = true;
		info_.isVisible = true;
		startPos_ = owner_->GetTransform().translate;
		transform_.translate = startPos_;
		moveTimer_ = 0;
		maxTime_ = 30.f; // Half second to go out
		hitEnemies_.clear(); // Reset hit history
		stayTimer_ = 0.0f; // Reset stay timer
		float range = range_;


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
					farestDistance_.x -= castDir.x * (tileSize);
					farestDistance_.y -= castDir.y * (tileSize);

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

	void AddDamageFromStar() {
		if (starCount_ <= 0) return;
		damage_ += 2;
		starCount_ -= 1;
	}

	void Update(float deltaTime = 1.0f) override {
		if (state_ == BoomerangState::Idle) {
			info_.isActive = false;
			info_.isVisible = false;
			return;
		}

		transform_.rotation -= (0.3f + float(damage_ + damageBonus_) / 10.f) * deltaTime;

		// Custom movement logic based on provided requirements
		Vector2 ownerPos = owner_->GetTransform().translate;

		if (state_ == BoomerangState::Thrown) {
			rigidbody_.velocity = { 0.0f, 0.0f }; // No physics movement
			if (moveTimer_ < maxTime_) {
				moveTimer_ += deltaTime;

				if (moveTimer_ >= maxTime_) hitEnemies_.clear();

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

					float distanceFormOwner = Vector2::Length(transform_.translate - ownerPos);
					//closer the distance bigger damage bonus
					damageBonus_ = int((range_ - distanceFormOwner) / 50.f);

					if (int(stayTimer_) % int(delayPerStar) == 0) AddDamageFromStar();
					if (stayTimer_ >= maxStayTime_) hitEnemies_.clear();

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
					damage_ += 3;
				}
			}
		}
		else if (state_ == BoomerangState::Returning) {
			// Simple return logic: Move towards player
			Vector2 dir = ownerPos - transform_.translate;
			if (Vector2::Length(dir) < 40.0f) {
				state_ = BoomerangState::Idle;
			}
			else {
				dir = Vector2::Normalize(dir);
				float speed = max(4.0f, 15.0f + float(damage_ + damageBonus_));

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
		UpdateDrawComponent(deltaTime);

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
			Destroy();
		}

	}

	void UpdateDrawComponent(float deltaTime) override {
		if (!info_.isActive) return;
		float ShakeIntensity = max(0.f, float(/*damage_ + */damageBonus_) * (IsReturning() ? 1.f : 5.f));
		RenderPos_.x = transform_.translate.x + float(rand() % 100) / 100.f * ShakeIntensity;
		RenderPos_.y = transform_.translate.y + float(rand() % 100) / 100.f * ShakeIntensity;

		if (drawComp_) {						
			drawComp_->SetTransform(transform_);
			drawComp_->SetPosition(RenderPos_);
			drawComp_->Update(deltaTime);
		}
		// Update star component if stars are active
		if (starCount_ > 0 && starComp_) {
			starComp_->Update(deltaTime);
		}

		if (effectComp_) {
			//effectComp_->SetTransform(transform_);
			effectComp_->SetPosition(RenderPos_);
			effectComp_->Update(deltaTime);
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

	bool IsIdle() const { return state_ == BoomerangState::Idle; }
	bool IsThrown() const { return state_ == BoomerangState::Thrown; }
	bool IsReturning() const { return state_ == BoomerangState::Returning; }
	bool IsTemporary() const { return isTemporary_; }
	// void Draw(const Camera2D& camera) override;

	void Draw(const Camera2D& camera) override {
		if (!info_.isActive || !info_.isVisible) return;
		// DrawComponent2Dを使って描画
		if (drawComp_) {
			drawComp_->Draw(camera);
		}
		if (effectComp_ && (damageBonus_) >= 0 && damage_ > 1 && IsReturning()) {
			effectComp_->Draw(camera);
		}

		DrawStar(camera);

		/*Vector2 screenPos = const_cast<Camera2D&>(camera).WorldToScreen(transform_.translate);
		Vector2 colliderSize = const_cast<Vector2&>(collider_.size);
		Vector2 colliderOffset = const_cast<Vector2&>(collider_.offset);

		Novice::DrawBox(
			int(screenPos.x + colliderOffset.x - colliderSize.x / 2.f),
			int(screenPos.y + colliderOffset.y),
			int(colliderSize.x), int(colliderSize.y),
			0.0f,
			0xFF0000FF,
			kFillModeWireFrame
		);*/
	}

	int OnCollision(GameObject2D* other) override {
		// Check if other is an enemy
		if (other->GetInfo().tag == "Enemy") {
			int enemyId = other->GetInfo().id;
			if (std::find(hitEnemies_.begin(), hitEnemies_.end(), enemyId) != hitEnemies_.end()) {
				return 0; // Ignore repetitive hits
			}

			hitEnemies_.push_back(enemyId);
			other->OnDamaged(damage_ + damageBonus_);
			Novice::ConsolePrintf("Boomerang hit Enemy ID %d for %d damage(%d + %d).\n", enemyId, damage_ + damageBonus_, damage_, damageBonus_);
			//other->Destroy();

			Star* star = manager_->Spawn<Star>(this, "Star");

			star->SetPosition(transform_.translate);
			star->SetOwner(other);

			if (IsThrown()) {
				// On first hit during throw, immediately start returning
				state_ = BoomerangState::Returning;
				moveTimer_ = 0;
				stayTimer_ = 0;
				targetPos_ = transform_.translate;
			}


		}
		return 0;
	}
};
