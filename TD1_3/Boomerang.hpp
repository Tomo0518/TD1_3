#pragma once
#include "PhysicsObject.hpp"
#include "algorithm"
#include "Star.hpp"

enum class BoomerangState {
	Thrown,
	Returning,
	Idle,
	blocked,
};


struct TrailInfo {
	Transform2D pos;
	float lifeTime;
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
	float defaultMaxStayTime_ = 80.f;
	Vector2 farestDistance_;
	bool isHorizontal_;
	std::vector<int> hitEnemies_; // Track hit enemy IDs
	bool isTemporary_;

	bool blocked_ = false;
	float blockedTimer_ = 0.f;
	float blockedDuration_ = 30.f;
	
	int damage_;
	int damageBonus_ = 0;
	float delayPerStar = 18;
	float range_ = 360.0f;
	float activeRange_ = range_;
	Vector2 RenderPos_;

	bool isAddBonusShake = false;
	float bonusShakeTimer = 0.f;
	float bonusShakeDuration = 15.f;

	int starCount_;
	bool starRetrieved_ = false;

	float frameCount_ = 0.f;

	bool waitingToReturn_ = false;
	bool isGoing = false;

	DrawComponent2D* starComp_ = nullptr;
	DrawComponent2D* effectComp_ = nullptr;
	DrawComponent2D* effectCompLv2_ = nullptr;
	DrawComponent2D* effectCompLv3_ = nullptr;

	std::vector<TrailInfo> trailInfos_;
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
		isGravityEnabled_ = false;
		rigidbody_.deceleration = { 0.95f, 0.95f };
	

		delete drawComp_;
		drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang), 4, 1, 4, 5.f, true);
		drawComp_->Initialize();

		starComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Star_shooting), 4, 1, 4, 5.f, true);
		starComp_->Initialize();

		effectComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang_ChargedLv1), 8, 1, 8, 2.f, true);
		effectComp_->Initialize();
		effectComp_->SetBaseColor({ 1.f, 1.f, 1.f, 0.8f });

		effectCompLv2_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang_ChargedLv2), 8, 1, 8, 2.f, true);
		effectCompLv2_->Initialize();
		effectCompLv2_->SetBaseColor({ 1.f, 1.f, 1.f, 0.8f });

		effectCompLv3_ = new DrawComponent2D(Tex().GetTexture(TextureId::Boomerang_ChargedLv3), 8, 1, 8, 2.f, true);
		effectCompLv3_->Initialize();
		effectCompLv3_->SetBaseColor({ 1.f, 1.f, 1.f, 0.8f });
	}

	~Boomerang() {
		delete starComp_;
		delete effectComp_;
		delete effectCompLv2_;
		delete effectCompLv3_;
	}

	bool isStarRetrieved() const {
		return starRetrieved_;
	}

	int retrieveStarCount() {
		starRetrieved_ = true;
		int temp = starCount_;
		starCount_ = 0;
		return temp;
	}
	bool isHitWall = false;

	void Throw(Vector2 direction, int Star = 0, float Charge = 0) {
		if (state_ != BoomerangState::Idle) return;
		starCount_ = Star;
		starRetrieved_ = false;
		isGoing = true;

		trailInfos_.clear();

		maxStayTime_ = defaultMaxStayTime_ + starCount_ * delayPerStar + Charge/2.f; // Each star adds 2 seconds

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
		activeRange_ = std::min(range_ + Charge * 2.f, 600.f);
		float range = activeRange_;

		isHitWall = false;

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
					//isHitWall = true;

					// Optional: Reduce travel time if distance is very short
					// maxTime_ = maxTime_ * (dist / range);
					break;

				}
			}

			if (dist >= range) break;
		}

		SoundManager::GetInstance().PlaySe(SeId::PlayerBoomerangThrow);
	}

	float EaseOutCubic(float t) {
		float f = 1.0f - t;
		return 1.0f - f * f * f;
	}

	void Blocked(Vector2 dir) {
		if (state_ != BoomerangState::Thrown) return;
		
		state_ = BoomerangState::blocked;
		blocked_ = true;
		blockedTimer_ = blockedDuration_;
		moveTimer_ = 0;
		stayTimer_ = 0;

		damage_ = 0;
		damageBonus_ = 0;

		collider_.canCollide = false;
		rigidbody_.AddForce(dir * 30.f);
	}

	// float EaseInCubic(float t) {
	//    return t * t * t;
	// }

	void AddDamageFromStar() {
		if (starCount_ <= 0) return;
		damage_ += 6;

		float radius = 50.f;
		float angleStep = 360.f / starCount_;

		starCount_ -= 1;		

		starComp_->SetRotation(starComp_->GetRotation() - 0.2f);
		
		float angle = angleStep * (starCount_);
		Vector2 offset = { cosf(AngleToRadians(angle)) * radius, sinf(AngleToRadians(angle)) * radius };
		Vector2 pos = transform_.translate + offset;
		ParticleManager::GetInstance().Emit(ParticleType::Hit, pos);
		
	}

	bool isWaitingToReturn() const {
		return waitingToReturn_;
	}

	void ResetToIdle() {
		waitingToReturn_ = false;
		starCount_ = 0;
		state_ = BoomerangState::Idle;
		info_.isActive = false;
		info_.isVisible = false;
	}

	void SwitchToReturn() {
		if (state_ != BoomerangState::Thrown) return;
		if (!waitingToReturn_) return;
		waitingToReturn_ = false;
		state_ = BoomerangState::Returning;
		collider_.canCollide = true;
		moveTimer_ = 0;
		stayTimer_ = 0;
		// Update target pos to be current pos for smooth return calculation if needed
		targetPos_ = transform_.translate;
	}

	void UpdateTrail(float deltaTime) {
		for (auto& trail : trailInfos_) {
			trail.lifeTime -= deltaTime;
		}

		// delete expired trails
		trailInfos_.erase(
			std::remove_if(trailInfos_.begin(), trailInfos_.end(),
				[](const TrailInfo& t) { return t.lifeTime <= 0.f; }),
			trailInfos_.end()
		);
	}

	bool IsGoing() const {
		return isGoing;
	}

	void Update(float deltaTime = 1.0f) override {
		if (state_ == BoomerangState::Idle) {
			info_.isActive = false;
			info_.isVisible = false;
			return;
		}

		frameCount_ += deltaTime;

		if(state_ != BoomerangState::blocked)
		transform_.rotation -= (0.03f + float(damage_ + damageBonus_) / 20.f) * deltaTime;	

		// Update trail
		UpdateTrail(deltaTime);

		
		Vector2 ownerPos = owner_->GetTransform().translate;

		if (int(frameCount_) % (damageBonus_ > 0 ? 1 : 7) == 0) {
			Vector2 toOwner = ownerPos - transform_.translate;
			Vector2 dirToOwner = Vector2::Normalize(toOwner);
			float distanceToOwner = Vector2::Length(toOwner);

			Vector2 trail = transform_.translate + dirToOwner * abs(sinf(frameCount_ / 10.f)) * (distanceToOwner);
			ParticleManager::GetInstance().Emit(ParticleType::MuzzleFlash, trail);
			float distanceFormOwner = Vector2::Length(transform_.translate - ownerPos);
			int sparkCount = (int)std::max(0.f, std::min(5.f, (activeRange_ * 2.5f - distanceFormOwner) / 50.f));
			for (int i = 0; i < sparkCount; ++i) {
				Vector2 randomOffset = { float((rand() % 200) - 100) / 100.f * 5.f, float((rand() % 200) - 100) / 100.f * 5.f };
				ParticleManager::GetInstance().Emit(ParticleType::Sparkle, trail + randomOffset);
			}
		}

		if (state_ == BoomerangState::blocked) {
			isGravityEnabled_ = true;
			if (rigidbody_.velocity.Length(rigidbody_.velocity) != 0.f || rigidbody_.acceleration.Length(rigidbody_.acceleration) != 0.f) {
				collider_.canCollide = true;
				Move(deltaTime);
				collider_.canCollide = false;
			}
			else {
				drawComp_->PlayAnimation();
			}


			blockedTimer_ -= deltaTime;

			if (blockedTimer_ <= 0.f) {
				isGravityEnabled_ = false;
				blocked_ = false;
				state_ = BoomerangState::Thrown;
				waitingToReturn_ = true;
				SwitchToReturn();
			}
		}
		else if (state_ == BoomerangState::Thrown) {
			
			if (moveTimer_ < maxTime_) {
				rigidbody_.velocity = { 0.0f, 0.0f }; // No physics movement
				moveTimer_ += deltaTime;

				isGoing = true;

				if (moveTimer_ >= maxTime_) {
					hitEnemies_.clear();
					damage_ += 8;
					waitingToReturn_ = true;
					isGoing = false;
				}
				if (moveTimer_ >= maxTime_ / 1.7f) collider_.canCollide = false;

				float t = moveTimer_ / maxTime_;
				if (t > 1.0f) t = 1.0f;

				// Go to target
				float easedT = EaseOutCubic(t);
				Vector2 currentTarget = startPos_ + (targetPos_ - startPos_) * easedT;

				if (isHorizontal_) {
					// x goes to target, y follows player
					transform_.translate.x = currentTarget.x;
					transform_.translate.y = (isHitWall ? farestDistance_.y : transform_.translate.y> ownerPos.y?ownerPos.y: transform_.translate.y);
					//clamp x to farest distance
					transform_.translate.x = std::clamp(transform_.translate.x,
						std::min(startPos_.x, farestDistance_.x),
						std::max(startPos_.x, farestDistance_.x));
				}
				else {
					// y goes to target, x follows player
					transform_.translate.y = currentTarget.y;
					transform_.translate.x = isHitWall ? farestDistance_.x : ownerPos.x;

					// clamp y to farest distance
					transform_.translate.y = std::clamp(transform_.translate.y,
						std::min(startPos_.y, farestDistance_.y),
						std::max(startPos_.y, farestDistance_.y));
				}
			}
			else {
				// Stay logic
				//if (stayTimer_ < maxStayTime_) {
					stayTimer_ += deltaTime;

					if (rigidbody_.velocity.Length(rigidbody_.velocity) != 0.f||rigidbody_.acceleration.Length(rigidbody_.acceleration) != 0.f) {
						collider_.canCollide = true;
						Move(deltaTime);
						collider_.canCollide = false;
					}
					collider_.canCollide = true;
					//closer the distance bigger damage bonus
					//damageBonus_ = std::max(0,int((activeRange_*1.4f - distanceFormOwner) / 50.f));
					float distanceFormOwner = Vector2::Length(transform_.translate - ownerPos);
					

					if (int(stayTimer_) % int(delayPerStar) == 0) AddDamageFromStar();
					if (stayTimer_ >= maxStayTime_) hitEnemies_.clear();

					// Still follow player while staying
					//if (isHorizontal_) {
					//	transform_.translate.x = farestDistance_.x;
					//	//transform_.translate.y = ownerPos.y;
					//}
					//else {
					//	transform_.translate.y = farestDistance_.y;
					//	transform_.translate.x = isHitWall ? farestDistance_.x : ownerPos.x;
					//}

					if (int(stayTimer_) % 30 == 0) {
						SoundManager::GetInstance().PlaySe(SeId::PlayerBoomerangFly);
					}

					if (int(stayTimer_) % 15 == 0) {
						damageBonus_ = std::min(20, damageBonus_ + 1);
					}

					if (distanceFormOwner > activeRange_ * 2.5f) {
						damageBonus_ = 0;
						damage_ = 0;
						SwitchToReturn();
					}

					
				//}
				//else {
					//state_ = BoomerangState::Returning;
					//collider_.canCollide = true;
					//moveTimer_ = 0;
					//stayTimer_ = 0;
					//// Update target pos to be current pos for smooth return calculation if needed
					//targetPos_ = transform_.translate;
					
				//}
			}
		}
		else if (state_ == BoomerangState::Returning) {
			// Simple return logic: Move towards player
			Vector2 dir = ownerPos - transform_.translate;

			trailInfos_.push_back({ transform_, 10.f });

			if (Vector2::Length(dir) < 40.0f) {
				state_ = BoomerangState::Idle;
			}
			else {
				dir = Vector2::Normalize(dir);
				float speed = std::max(4.0f, 30.0f + float(damage_ + damageBonus_)/2.f);

				if (isHorizontal_) {
					// Only x moves back, y matches player
					transform_.translate.x += dir.x * speed;
					transform_.translate.y += dir.y * speed/2.f;
				}
				else {
					// Only y moves back, x matches player
					transform_.translate.y += dir.y * speed;
					transform_.translate.x = ownerPos.x;
				}
			}
		}

		//GameObject2D::Update(deltaTime);
		UpdateDrawComponent(deltaTime);

		// clamp to farthest distance
#ifdef _DEBUG
		Novice::ScreenPrintf(10, 60, "Farest X: %.2f Y: %.2f", farestDistance_.x, farestDistance_.y);
		Novice::ScreenPrintf(10, 80, "Current X: %.2f Y: %.2f", transform_.translate.x, transform_.translate.y);
#endif
		/*if (isHorizontal_) {
			transform_.translate.x = std::clamp(transform_.translate.x,
				std::min(ownerPos.x, farestDistance_.x),
				std::max(ownerPos.x, farestDistance_.x));
		}
		else {
			transform_.translate.y = std::clamp(transform_.translate.y,
				std::min(ownerPos.y, farestDistance_.y),
				std::max(ownerPos.y, farestDistance_.y));
		}*/

		if (IsTemporary() && IsIdle()) {
			Destroy();
		}

	}

	void UpdateDrawComponent(float deltaTime) override {
		if (!info_.isActive) return;
		bonusShakeTimer -= deltaTime;

		Vector2 bonusShakeOffset = { 0.f, 0.f };
		if (bonusShakeTimer <= 0.f) {
			bonusShakeTimer = 0.f;
			isAddBonusShake = false;
		} else {
			isAddBonusShake = true;
			bonusShakeOffset = { float(rand() % 200 - 100) / 100.f * 20  ,  float(rand() % 200 - 100) / 100.f * 20 };
		}
		
		float ShakeIntensity = std::max(0.f, float(damage_ + damageBonus_/10.f));
		RenderPos_.x = transform_.translate.x + float(rand() % 200 - 100) / 100.f * ShakeIntensity + bonusShakeOffset.x;
		RenderPos_.y = transform_.translate.y + float(rand() % 200 - 100) / 100.f * ShakeIntensity + bonusShakeOffset.y;

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
			//effectComp_->SetPosition(RenderPos_);
			effectComp_->SetPosition(transform_.translate + bonusShakeOffset);
			effectComp_->Update(deltaTime);
		}

		if (effectCompLv2_) {
			//effectCompLv2_->SetTransform(transform_);
			//effectCompLv2_->SetPosition(RenderPos_);
			effectCompLv2_->SetPosition(transform_.translate + bonusShakeOffset);
			effectCompLv2_->Update(deltaTime);
		}

		if (effectCompLv3_) {
			//effectCompLv3_->SetTransform(transform_);
			//effectCompLv3_->SetPosition(RenderPos_);
			effectCompLv3_->SetPosition(transform_.translate + bonusShakeOffset);
			effectCompLv3_->Update(deltaTime);
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

			for (auto& trail : trailInfos_) {
				float alpha = trail.lifeTime / 20.f;
				drawComp_->SetBaseColor({0.6f, 0.6f, 1.0f, alpha });
				drawComp_->SetTransform(trail.pos);	
				drawComp_->Draw(camera);
			}
			drawComp_->SetBaseColor({ 1.f, 1.f,1.f, 1.f });
		}

#ifdef _DEBUG
		Novice::ScreenPrintf(600, 360, "Boomerang Damage: %d + %d = %d", damage_, damageBonus_ , damageBonus_ + damage_);
#endif

		if (effectComp_ && (damageBonus_ + damage_) > 8) {
			float ratio = (float(damageBonus_ + damage_) - 8.f) / 10.f;
			ratio = (std::min)(ratio, 1.f);
			float alpha =  ratio;
			effectComp_->SetBaseColor({ 1.f, 1.f, 1.f, alpha });
			effectComp_->Draw(camera);
		}
		if (effectCompLv2_ && (damageBonus_ + damage_) > 18 ) {
			float ratio = float(damageBonus_ + damage_ - 18) / 10.f;
			ratio = (std::min)(ratio, 1.f);
			float alpha =  ratio;
			effectCompLv2_->SetBaseColor({ 1.f, 1.f, 1.f, alpha });
			effectCompLv2_->Draw(camera);
		}
		if (effectCompLv3_ && (damageBonus_ + damage_) > 28 ) {
			float ratio = float(damageBonus_ + damage_ - 28) / 10.f;
			ratio = (std::min)(ratio, 1.f);
			float alpha = ratio;
			effectCompLv3_->SetBaseColor({ 1.f, 1.f, 1.f, alpha });
			effectCompLv3_->Draw(camera);
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

			if (IsThrown()) {
				// On first hit during throw, immediately start returning
				/*state_ = BoomerangState::Returning;
				moveTimer_ = 0;
				stayTimer_ = 0;
				targetPos_ = transform_.translate;*/ 
				if (isGoing) {
					Vector2 dir;

					dir.x = transform_.translate.x > other->GetTransform().translate.x ? 1.f : -1.f;
					dir.y = 0.5f;
					dir = Vector2::Normalize(dir);
					Blocked(dir);
				}
				
			}


		}
		return 0;
	}

	void AddDamageBonus(int bonus) {
		damageBonus_ += bonus;
		ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
		ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
		bonusShakeTimer = bonusShakeDuration;
		SoundManager::GetInstance().PlaySe(SeId::StarSpawn);
		hitEnemies_.clear();
	}



};
