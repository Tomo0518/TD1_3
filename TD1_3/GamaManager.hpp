//#pragma once
//
//#include <Novice.h>
//#include "gameObject.hpp"
//#include "player.hpp"
//#include "enemy.hpp"
//#include "boss.hpp"
//#include "damageHitBox.hpp"
//#include "boss.hpp" // ボスのヘッダーをインクルード
//#include "Debug.hpp" // デバッグ機能をインクルード
//#include "camera.hpp"
//#include "collectible.hpp"
//#include "tail.hpp"
//#include "particle.hpp"
//#include "AnimationPlayer.h" // 追加: アニメーション再生用
//#include "UIManager.hpp"
//
//#ifdef _DEBUG
//#include <imgui.h>
//#endif
//
//struct Circle {
//	Vector2<float> center;
//	float radius;
//};
//
//std::vector<Circle> GetSubdividedCircles(const Transform& t, const Collider& c, int numCircles) {
//	std::vector<Circle> circles;
//	float longerEdge = max(c.size.x, c.size.y);
//	float shorterEdge = min(c.size.x, c.size.y);
//	float step = longerEdge / numCircles;
//	float halfLonger = longerEdge * 0.5f;
//	float r = shorterEdge * 0.5f; // radius for each small circle
//	for (int i = 0; i < numCircles; ++i) {
//		// Center of each circle along the rectangle's local longer axis
//		float localPos = -halfLonger + step * (i + 0.5f);
//		float localX, localY;
//		if (c.size.x >= c.size.y) {
//			localX = localPos;
//			localY = 0.0f; // Center line
//		}
//		else {
//			localX = 0.0f; // Center line
//			localY = localPos;
//		}
//		// Rotate local point by angle
//		float cosA = cosf(t.angle);
//		float sinA = sinf(t.angle);
//		Vector2<float> offset = {
//			localX * cosA - localY * sinA,
//			localX * sinA + localY * cosA
//		};
//		Vector2<float> center = t.position + offset;
//		circles.push_back({ center, r });
//	}
//	return circles;
//}
//
//bool CirclesCollide(const Circle& a, const Circle& b) {
//	float dx = a.center.x - b.center.x;
//	float dy = a.center.y - b.center.y;
//	float rSum = a.radius + b.radius;
//	return (dx * dx + dy * dy) <= (rSum * rSum);
//}
//
//bool checkCollision(GameObject* a, GameObject* b) {
//	const Transform& tA = a->GetTransform();
//	const Transform& tB = b->GetTransform();
//	const Collider& cA = a->GetCollider();
//	const Collider& cB = b->GetCollider();
//
//	float radiusA = max(cA.size.x, cA.size.y);
//	float radiusB = max(cB.size.x, cB.size.y);
//
//	// Broad-phase: bounding circles
//	float dx = tA.position.x - tB.position.x;
//	float dy = tA.position.y - tB.position.y;
//	float distanceSq = dx * dx + dy * dy;
//	float radiusSum = radiusA + radiusB;
//	if (distanceSq > radiusSum * radiusSum) return false;
//
//	// Narrow-phase: subdivide rectangles into circles
//	float longerEgdeA = max(cA.size.x, cA.size.y);
//	float longerEgdeB = max(cB.size.x, cB.size.y);
//	float shorterEdgeA = min(cA.size.x, cA.size.y);
//	float shorterEdgeB = min(cB.size.x, cB.size.y);
//	if (shorterEdgeA <= 0.0f) shorterEdgeA = 1.0f;
//	if (shorterEdgeB <= 0.0f) shorterEdgeB = 1.0f;
//
//	int numCirclesA = static_cast<int>(longerEgdeA / shorterEdgeA) + 1;
//	int numCirclesB = static_cast<int>(longerEgdeB / shorterEdgeB) + 1;
//	auto circlesA = GetSubdividedCircles(tA, cA, numCirclesA);
//	auto circlesB = GetSubdividedCircles(tB, cB, numCirclesB);
//
//	for (const auto& ca : circlesA) {
//		for (const auto& cb : circlesB) {
//			if (CirclesCollide(ca, cb)) return true;
//		}
//	}
//	return false;
//}
//
//class GameManager {
//private:
//	int frameCount_ = 0;
//
//	int nextGameObjectID_ = -1;
//	Camera camera_;
//
//	bool inTitle_ = true;
//	int titleInputCooldown_ = 0;
//
//	Player* player_ = nullptr;
//	Boss* boss_ = nullptr;
//	BigDummyEnemy* dummy_ = nullptr;
//	bool bossSpawned_ = false;
//
//	std::vector<GameObject*> enemies_;
//	std::vector<GameObject*> bullets_;
//	std::vector<Collectible*> items_;
//
//	int expSpawnTimer_ = 0;
//	int expSpawnInterval_ = 120;
//	float expSpawnRadius_ = 300.f;
//	bool enterPhase3_ = false;
//	bool inGamePlay_ = false;
//	float damageFlashAlpha_ = 0.f;
//	// --- 追加: タイトルアニメーション用 ---
//	SimpleAnimation::AnimationPlayer titlePlayer_;
//	bool isTitleAnimLoaded_ = false;
//
//	// background
//	Animator backgroundAnimator_;
//	Animator midgroundAnimator_;
//	Animator foregroundAnimator_;
//
//	void TriggerDamageEffect() {
//		//camera_.shake(25.0f, 10.0f, true);
//		damageFlashAlpha_ = 180.0f;
//	}
//
//	void handlePlayerBulletCollisions() {
//		for (GameObject* b : bullets_) {
//			if (!b->GetCollider().canCollide) continue;
//			if (b->GetStatus().StateFlag != 0) continue; // only player bullets
//			if (!b->IsActive()) continue;
//			// Boss interactions
//			if (boss_) {
//				boss_->ForEachTailNode([&](BossTailNode* node) {
//					if (!b->IsActive()) return;
//					if (node->IsActive() && checkCollision(b, node)) {
//						if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) bullet->Hit();
//						node->TakeDamage();
//						return;
//					}
//					node->ForEachPart([&](Enemy* part) {
//						if (!b->IsActive()) return;
//						if (part && part->IsActive() && checkCollision(b, part)) {
//							if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) bullet->Hit();
//							part->TakeDamage();
//						}
//						});
//					});
//				if (b->IsActive()) {
//					boss_->forEachPart([&](Enemy* part) {
//						if (!b->IsActive()) return;
//						if (part->IsActive() && checkCollision(b, part)) {
//							if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) bullet->Hit();
//							part->TakeDamage();
//						}
//						});
//				}
//				if (b->IsActive() && boss_->IsActive() && checkCollision(b, boss_)) {
//					if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) bullet->Hit();
//					boss_->TakeDamage();
//				}
//			}
//			if (!b->IsActive()) continue;
//			// Enemy interactions
//			for (GameObject* e : enemies_) {
//				if (!e->IsActive()) continue;
//				if (auto* normalEnemy = dynamic_cast<NormalEnemy*>(e)) {
//					bool hitTail = false;
//					normalEnemy->GetTail().forEachNode([&](EnemyTailNode* node) {
//						if (!hitTail && b->IsActive() && node->IsActive() && checkCollision(b, node)) {
//							if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) bullet->Hit();
//							normalEnemy->OnTailNodeHit(node, 1);
//							hitTail = true;
//							node->TakeDamage();
//						}
//						});
//					if (!b->IsActive()) continue;
//				}
//				else {
//					if (checkCollision(b, e)) {
//						if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) bullet->Hit();
//						if (auto* enemy = dynamic_cast<Enemy*>(e)) enemy->TakeDamage();
//					}
//				}
//			}
//		}
//	}
//
//	void handleEnemyBulletCollisions() {
//		for (GameObject* b : bullets_) {
//			if (!b->GetCollider().canCollide) continue;
//			if (b->GetStatus().StateFlag == 0) continue; // only enemy bullets
//			if (!player_ || !player_->IsActive() || !b->IsActive()) continue;
//			if (checkCollision(b, player_)) {
//				if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) {
//					bullet->Hit();
//					player_->TakeDamage(bullet->GetDamage());
//					// shake camera
//					camera_.shake(5.f, 15, true);
//					TriggerDamageEffect();
//				}
//			}
//			player_->forEachTailNode([&](TailNodes* node) {
//				if (b->IsActive() && checkCollision(b, node)) {
//
//					if (auto* bullet = dynamic_cast<DamageHitBox*>(b)) {
//						bullet->Hit();
//						node->TakeDamage(bullet->GetDamage());
//					}
//				}
//				});
//		}
//	}
//
//	void handleItemCollections() {
//		for (Collectible* item : items_) {
//			if (!item->IsActive() || !player_ || !player_->IsActive()) continue;
//			if (checkCollision(player_, item)) item->ApplyEffect(player_);
//		}
//	}
//
//	void handlePlayerEntityCollisions() {
//		if (!player_ || !player_->IsActive()) return;
//		for (GameObject* e : enemies_) {
//			if (!e || !e->IsActive() || !e->GetCollider().canCollide) continue;
//			if (checkCollision(e, player_)) {
//				if (auto* enemy = dynamic_cast<Enemy*>(e)) {
//					enemy->OnCollision(player_);
//					if (enemy != dummy_) {
//						camera_.shake(5.f, 15, true);
//						TriggerDamageEffect();
//					}
//				}
//			}
//		}
//		// Player tail nodes vs enemies: let enemy handle dying and damaging
//		player_->forEachTailNode([&](TailNodes* node) {
//			for (GameObject* e : enemies_) {
//				if (!e || !e->IsActive() || !e->GetCollider().canCollide) continue;
//				if (checkCollision(e, node)) {
//					if (auto* enemy = dynamic_cast<Enemy*>(e)) enemy->OnCollision(node);
//				}
//			}
//			});
//		if (boss_) {
//			boss_->ForEachTailNode([&](BossTailNode* node) {
//				if (!node || !node->IsActive()) return;
//				if (checkCollision(node, player_)) {
//					Vector2f dir = player_->GetTransform().position - node->GetTransform().position;
//					if (dir.length() < 0.001f) dir = { 1.f, 0.f };
//					dir.normalize();
//					float pushDist = 20.f;
//					player_->GetTransform().position += dir * pushDist;
//					player_->GetTransform().velocity *= 0.8f;
//				}
//				node->ForEachPart([&](Enemy* part) {
//					if (!part || !part->IsActive() || !part->GetCollider().canCollide) return;
//					if (checkCollision(part, player_)) part->OnCollision(player_);
//					});
//				});
//		}
//	}
//
//	void CheckCollisions() {
//		// Player bullet collisions
//		handlePlayerBulletCollisions();
//		// Enemy bullet collisions
//		handleEnemyBulletCollisions();
//		// Item pickups
//		handleItemCollections();
//		// Player collides with enemies and boss nodes/parts
//		handlePlayerEntityCollisions();
//	}
//
//	void SpawnNormalEnemy(Vector2f position) {
//		auto* enemy = new NormalEnemy(GetNextGameObjectID(), "Enemy");
//		enemy->Initialize();
//		enemy->GetTransform().position = position;
//		enemy->SetPlayer(player_);
//		enemies_.push_back(enemy);
//	}
//
//	void SpawnExpPoint() {
//		if (!player_ || !player_->IsActive()) return;
//		// Random angle and distance
//		float angle = (float)(rand()) / (float)RAND_MAX * 6.2831853f;
//		float distance = expSpawnRadius_ * (0.3f + (float)rand() / (float)RAND_MAX * 0.7f); // between 0.3R and 1.0R
//		Vector2f offset = { cosf(angle) * distance, sinf(angle) * distance };
//		Vector2f spawnPos = player_->GetTransform().position + offset;
//
//		ExpPoint* exp = new ExpPoint(GetNextGameObjectID(), "ExpPoint");
//		exp->Initialize();
//		exp->GetTransform().position = spawnPos;
//		items_.push_back(exp);
//	}
//
//	void UpdateItemSpawning() {
//		//expSpawnTimer_++;
//		if (expSpawnTimer_ >= expSpawnInterval_) {
//			expSpawnTimer_ = 0;
//			SpawnExpPoint();
//		}
//	}
//
//	void UpdateItems(float deltaTime) {
//		int i = 0;
//		for (Collectible* item : items_) {
//			item->Update(deltaTime);
//			i++;
//		}
//		//Novice::ScreenPrintf(300, 100 + i * 20, "Items: %d", static_cast<int>(items_.size()));
//	}
//
//	void ClearInactiveItems() {
//		items_.erase(
//			std::remove_if(items_.begin(), items_.end(), [](Collectible* item) {
//				if (!item->IsActive()) { delete item; return true; }
//				return false;
//				}),
//			items_.end());
//	}
//
//public:
//	bool isExiting_ = false;
//
//	void StartGame() {
//		// reset and initialize gameplay
//		ClearGameObjects();
//		inTitle_ = false;
//		enterPhase3_ = false;
//
//		Initialize();
//
//		UIManager::Instance().SetIsGamePlay(false);
//		UIManager::Instance().SetIsTitle(false);
//
//	}
//
//	void RestartBossBattle() {
//		ClearGameObjects();
//		inTitle_ = false;
//		enterPhase3_ = false;
//
//		Initialize(false); // don't spawn dummy, go straight to boss
//
//		SpawnRequest bossSpawnRequest;
//		bossSpawnRequest.type = SpawnType::Boss;
//		bossSpawnRequest.transform.position = { 1400.f, 360.f };
//		bossSpawnRequest.transform.velocity = { 0.0f, 0.f };
//		bossSpawnRequest.owner = nullptr;
//		g_spawnRequests.push_back(bossSpawnRequest);
//		bossSpawned_ = true;
//
//		SoundManager::Instance().NextBGMSet(SoundName::BGM_GamePlay1, true);
//
//		// UI状態を設定
//		UIManager::Instance().SetIsGamePlay(true);
//		UIManager::Instance().SetIsTitle(false);
//		UIManager::Instance().ResetPause(); // ポーズ状態解除
//		inGamePlay_ = true;
//	}
//
//	void ReturnToTitle() {
//		// clear all and set title flag
//		ClearGameObjects();
//		inTitle_ = true;
//		inGamePlay_ = false;
//		UIManager::Instance().SetIsGamePlay(false);
//		UIManager::Instance().SetIsTitle(true);
//
//		titleInputCooldown_ = 30;
//		UIManager::Instance().ResetPause(); // ポーズ状態解除
//
//		//SoundManager::Instance().StopBGM(SoundName::BGM_GamePlay1);
//		SoundManager::Instance().NextBGMSet(SoundName::BGM_Title, true);
//		// タイトルに戻ったときにアニメーションをリセットして再開（必要であれば）
//		if (isTitleAnimLoaded_) {
//			titlePlayer_.Play("dragon_enter", true);
//		}
//	}
//
//	void Initialize(bool spawnDummy = true) {
//		nextGameObjectID_ = -1;
//		enemies_.clear();
//		bullets_.clear();
//		items_.clear();
//
//		bossSpawned_ = false;
//		dummy_ = nullptr;
//		enterPhase3_ = false;
//
//		InputManager::Instance().setCamera(&camera_);
//
//		backgroundAnimator_ = Animator(
//			TextureManager::Instance().Get(TextureName::Background1),
//			1280, 720,
//			1
//		);
//		midgroundAnimator_ = Animator(
//			TextureManager::Instance().Get(TextureName::Background3),
//			1280, 720,
//			1
//		);
//
//		foregroundAnimator_ = Animator(
//			TextureManager::Instance().Get(TextureName::Background2),
//			1280, 720,
//			1
//		);
//
//
//		player_ = new Player(GetNextGameObjectID(), "Player");
//		player_->Initialize();
//		boss_ = nullptr;
//
//		// Spawn BigDummyEnemy instead of boss
//		if (spawnDummy) {
//			dummy_ = new BigDummyEnemy(GetNextGameObjectID(), "Enemy");
//			dummy_->Initialize();
//			dummy_->SetPlayer(player_);
//			enemies_.push_back(dummy_);
//		}
//
//		// Optionally spawn one normal enemy for demo
//		/*auto* enemy = new NormalEnemy(GetNextGameObjectID(), "Enemy");
//		enemy->Initialize();
//		enemy->SetPlayer(player_);
//		enemies_.push_back(enemy);*/
//	}
//
//	void SpawnEnemyOverTime(float deltaTime) {
//		// Example: spawn an enemy every 5 seconds
//		static float spawnTimer = 0.f;
//		spawnTimer += deltaTime;
//		if (spawnTimer >= 300.f) { // assuming deltaTime is in frames at 60fps
//			spawnTimer = 0.f;
//			SpawnNormalEnemy({ 1280.f, static_cast<float>(rand() % 720) });
//		}
//	}
//
//	void Update(float deltaTime = 1.f) {
//		frameCount_++;
//		InputManager::Instance().Update();
//		SoundManager::Instance().Update();
//
//		if (inTitle_) {
//			if (titleInputCooldown_ > 0) {
//				titleInputCooldown_--;
//			}
//
//			// --- 追加: タイトルアニメーションの更新 ---
//			if (InputManager::Instance().GetKeyState(DIK_ESCAPE)) {
//				isExiting_ = true;
//			}
//
//			if (!isTitleAnimLoaded_) {
//				titlePlayer_.Load("resources/AnimationData/title.json");
//				titlePlayer_.Play("dragon_enter"); // 最初は登場アニメーション
//				isTitleAnimLoaded_ = true;
//			}
//
//			// 登場アニメーションが終わったら待機アニメーションへ移行
//			if (titlePlayer_.GetCurrentClipName() == "dragon_enter" && titlePlayer_.IsFinished()) {
//				titlePlayer_.Play("dragon_idle", true);
//			}
//
//			titlePlayer_.Update();
//			// --------------------
//
//			UIManager::Instance().SetIsTitle(true);
//
//			// wait for space to start
//			if (titleInputCooldown_ <= 0) {
//				if (Novice::CheckHitKey(DIK_SPACE) ||
//					(Novice::IsTriggerButton(0, kPadButton9)) ||
//					(Novice::IsTriggerButton(0, kPadButton10)) ||
//					Novice::IsTriggerMouse(0)
//
//					) {
//					//SoundManager::Instance().StopBGM(SoundName::BGM_Title);
//					StartGame();
//				}
//			}
//			return; // skip game update while in title
//		}
//
//		// リザルト画面の更新と選択結果の取得
//		ResultSelection resultSelection = UIManager::Instance().UpdateResult();
//		if (resultSelection == ResultSelection::ReStart) {
//			RestartBossBattle(); // ボス戦からリスタート
//			UIManager::Instance().ResetResult();
//			return;
//		}
//		else if (resultSelection == ResultSelection::Title) {
//			ReturnToTitle(); // タイトルに戻る
//			UIManager::Instance().ResetResult();
//			return;
//		}
//
//		// リザルト中はゲーム更新をスキップ
//		if (UIManager::Instance().IsResultActive()) {
//			return;
//		}
//
//		// ポーズ画面の更新と選択結果の取得
//		Selection pauseSelection = UIManager::Instance().UpdatePause();
//
//		// ポーズ中の選択処理
//		if (pauseSelection == Selection::ReStart) {
//			RestartBossBattle(); // ボス戦からリスタート
//			return;
//		}
//		else if (pauseSelection == Selection::Title) {
//			ReturnToTitle(); // タイトルに戻る
//			return;
//		}
//
//		if (UIManager::Instance().IsPaused()) {
//			return; // ポーズ中はゲーム更新をスキップ
//		}
//
//		if (!enterPhase3_) {
//			if (boss_) {
//				if (boss_->isInPhase3()) {
//					enterPhase3_ = true;
//					SoundManager::Instance().NextBGMSet(SoundName::BGM_GamePlay2, true);
//					SoundManager::Instance().PlaySFX(SoundType::BossScream);
//				}
//			}
//		}
//
//		// Update damage flash effect
//		if (damageFlashAlpha_ > 0.0f) {
//			damageFlashAlpha_ -= 4.25f; // 255 / 60
//			if (damageFlashAlpha_ < 0.0f) damageFlashAlpha_ = 0.0f;
//		}
//
//		// Update camera to follow player
//		cameraUpdate(deltaTime);
//
//		// Update all game objects
//		GameObjectUpdate(deltaTime);
//		ParticleSystem::Instance().Update(deltaTime);
//
//		// scene-specific updates
//		UpdateItemSpawning();
//		//SpawnEnemyOverTime(deltaTime);
//
//		// Check for collisions
//		CheckCollisions();
//
//		// Handle spawn requests
//		HandleSpawnRequests();
//
//		// Clear inactive objects
//		ClearInactiveObjects();
//
//		// If player died, return to title
//		if (!player_ || player_->GetStatus().currentHP <= 0) {
//			UIManager::Instance().ShowResult(ResultState::Over);
//			SoundManager::Instance().NextBGMSet(SoundName::BGM_GameOver, true);
//		}
//
//		if (boss_) {
//			if (boss_->isDefeated()) {
//				//clear all enemies except player and boss
//				for (GameObject* e : enemies_) {
//					if (e != dummy_ && e != boss_) {
//						e->SetActive(false);
//					}
//				}
//				// Show result screen when boss is defeated
//				//UIManager::Instance().ShowResult(ResultState::Clear);
//
//				SoundManager::Instance().NextBGMSet(SoundName::BGM_GameClear, true);
//			}
//		}
//
//		if (!boss_ && !inTitle_ && inGamePlay_) {
//			// Boss defeated, return to title after a delay
//			static int bossDefeatTimer = 0;
//			bossDefeatTimer++;
//			if (bossDefeatTimer >= 80) { // 5 seconds at 60fps
//				//ReturnToTitle();
//				UIManager::Instance().ShowResult(ResultState::Clear);
//				bossDefeatTimer = 0;
//			}
//		}
//
//
//	}
//
//	void DrawBackground() {
//		backgroundAnimator_.update();
//
//		const float tileW = 1280.0f;
//		const float tileH = 720.0f;
//
//		// Normal camera matrices
//		Matrix3x3f viewMatrix = camera_.getViewMatrix();
//		Matrix3x3f orthoMatrix = camera_.getOrthoMatrix();
//
//		// Build a parallax view by scaling the camera translation
//		// Assuming Matrix3x3f is row-major: [ a b tx; c d ty; 0 0 1 ]
//		Matrix3x3f parallaxView = viewMatrix;
//		const float parallax = 0.05f; // 0.0 = fixed background, 1.0 = same speed as world
//		parallaxView.m[0][2] *= parallax; // tx
//		parallaxView.m[1][2] *= parallax; // ty
//
//		// World-space top-left of the parallax layer
//		const Vector2f camWorldTL = camera_.getRenderOffset();
//		const Vector2f bgOffset = camWorldTL * parallax;
//
//		// Tile-aligned anchor for the parallax layer
//		const float startX = std::floor(bgOffset.x / tileW) * tileW;
//		const float startY = std::floor(bgOffset.y / tileH) * tileH;
//
//		// Draw 3x3 tiles around the parallaxed anchor using the parallax view
//		for (int j = -1; j <= 1; ++j) {
//			for (int i = -1; i <= 1; ++i) {
//				const Vector2f center1 = {
//					startX + (i + 0.5f) * tileW,
//					startY + (j + 0.5f) * tileH
//				};
//				//backgroundAnimator_.setSpriteSheetOnly(TextureManager::Instance().Get(TextureName::Background1));
//				backgroundAnimator_.drawWithMatrix(
//					center1,
//					parallaxView, orthoMatrix,
//					{ 1.f, 1.f },
//					0.f,
//					0xFFFFFFFF
//				);
//			}
//		}
//	}
//
//	void DrawMidGround() {
//		midgroundAnimator_.update();
//		const float tileW = 1280.0f;
//		const float tileH = 720.0f;
//		// Normal camera matrices
//		Matrix3x3f viewMatrix = camera_.getViewMatrix();
//		Matrix3x3f orthoMatrix = camera_.getOrthoMatrix();
//		// Build a parallax view by scaling the camera translation
//		// Assuming Matrix3x3f is row-major: [ a b tx; c d ty; 0 0 1 ]
//		Matrix3x3f parallaxView = viewMatrix;
//		const float parallax = 0.7f; // 0.0 = fixed background, 1.0 = same speed as world
//		parallaxView.m[0][2] *= parallax; // tx
//		parallaxView.m[1][2] *= parallax; // ty
//		// World-space top-left of the parallax layer
//		const Vector2f camWorldTL = camera_.getRenderOffset();
//		const Vector2f bgOffset = camWorldTL * parallax;
//		// Tile-aligned anchor for the parallax layer
//		const float startX = std::floor(bgOffset.x / tileW) * tileW;
//		const float startY = std::floor(bgOffset.y / tileH) * tileH;
//		// Draw 3x3 tiles around the parallaxed anchor using the parallax view
//		for (int j = -1; j <= 1; ++j) {
//			for (int i = -1; i <= 1; ++i) {
//				const Vector2f center1 = {
//					startX + (i + 0.5f) * tileW,
//					startY + (j + 0.5f) * tileH
//				};
//				//midgroundAnimator_.setSpriteSheetOnly(TextureManager::Instance().Get(TextureName::Background2));
//				midgroundAnimator_.drawWithMatrix(
//					center1,
//					parallaxView, orthoMatrix,
//					{ 1.f, 1.f },
//					0.f,
//					0xFFFFFFAA
//				);
//			}
//		}
//	}
//
//	void DrawForeGround() {
//		foregroundAnimator_.update();
//		const float tileW = 1280.0f;
//		const float tileH = 720.0f;
//		// Normal camera matrices
//		Matrix3x3f viewMatrix = camera_.getViewMatrix();
//		Matrix3x3f orthoMatrix = camera_.getOrthoMatrix();
//		// Build a parallax view by scaling the camera translation
//		// Assuming Matrix3x3f is row-major: [ a b tx; c d ty; 0 0 1 ]
//		Matrix3x3f parallaxView = viewMatrix;
//		const float parallax = 0.9f; // 0.0 = fixed background, 1.0 = same speed as world
//		parallaxView.m[0][2] *= parallax; // tx
//		parallaxView.m[1][2] *= parallax; // ty
//		// World-space top-left of the parallax layer
//		const Vector2f camWorldTL = camera_.getRenderOffset();
//		const Vector2f bgOffset = camWorldTL * parallax;
//		// Tile-aligned anchor for the parallax layer
//		const float startX = std::floor(bgOffset.x / tileW) * tileW;
//		const float startY = std::floor(bgOffset.y / tileH) * tileH;
//		// Draw 3x3 tiles around the parallaxed anchor using the parallax view
//		for (int j = -1; j <= 1; ++j) {
//			for (int i = -1; i <= 1; ++i) {
//				const Vector2f center1 = {
//					startX + (i + 0.5f) * tileW,
//					startY + (j + 0.5f) * tileH
//				};
//				//foregroundAnimator_.setSpriteSheetOnly(TextureManager::Instance().Get(TextureName::Background3));
//				foregroundAnimator_.drawWithMatrix(
//					center1,
//					parallaxView, orthoMatrix,
//					{ 1.f, 1.f },
//					0.f,
//					0xFFFFFFEE
//				);
//			}
//		}
//	}
//
//	void Draw() {
//		DrawBackground();
//		DrawMidGround();
//		DrawForeGround();
//
//		if (inTitle_) {
//			if (isTitleAnimLoaded_) {
//				Matrix3x3f identityMatrix = Matrix3x3f::CreateIdentity();
//				titlePlayer_.Draw(identityMatrix, identityMatrix);
//				Novice::DrawBox(0, 0, 1280, 720, 0.f, 0x000000AA, kFillModeSolid); // 半透明の黒いオーバーレイ
//				Novice::DrawSprite(0, 0, TextureManager::Instance().Get(TextureName::TitleLogo), 1.f, 1.f, 0.f, 0xFFFFFFFF);
//
//				unsigned int color = 0xFFFFFF00;
//				float glint = abs(sinf(static_cast<float>(frameCount_ * 0.1f)) + 1.0f) / 2.0f;
//				color |= static_cast<unsigned int>(50.f + glint * 205.0f);
//
//				Novice::DrawSprite(0, 0, TextureManager::Instance().Get(TextureName::TitleStart), 1.f, 1.f, 0.f, color);
//				Novice::DrawSprite(740, 430, TextureManager::Instance().Get(TextureName::credit), 0.4f, 0.4f, 0.f, 0xFFFFFFFF);
//			}
//			return;
//		}
//		/*if (dummy_) {
//			Novice::ScreenPrintf(450, 220, "Left stick: Move");
//			Novice::ScreenPrintf(650, 220, "Right stick: Aim");
//		}*/
//		Matrix3x3f viewMatrix = camera_.getViewMatrix();
//		Matrix3x3f orthoMatrix = camera_.getOrthoMatrix();
//
//		for (GameObject* obj : enemies_) obj->Draw(viewMatrix, orthoMatrix);
//
//		if (boss_) boss_->Draw(viewMatrix, orthoMatrix);
//		for (Collectible* item : items_) item->Draw(viewMatrix, orthoMatrix);
//
//		//particles
//		ParticleSystem::Instance().Draw(viewMatrix, orthoMatrix);
//
//		//bullets
//		for (GameObject* obj : bullets_) obj->Draw(viewMatrix, orthoMatrix);
//
//		// Player
//		player_->Draw(viewMatrix, orthoMatrix);
//
//		// UI
//		// draw aim direction line
//		player_->DrawUI(viewMatrix, orthoMatrix);
//		if (damageFlashAlpha_ > 0.0f) {
//			unsigned int color = 0xFF000000 | (static_cast<unsigned int>(damageFlashAlpha_) & 0xFF);
//			// Novice::DrawBox(0, 0, 1280, 720, 0.f, color, kFillModeSolid);
//			Novice::DrawSprite(0, 0, TextureManager::Instance().Get(TextureName::PlayerDamageEffect), 1.f, 1.f, 0.f, color);
//
//		}
//		UIManager::Instance().Draw();
//
//		if (boss_) {
//			if (boss_->isInPhase3()) {
//				unsigned int alpha = static_cast<unsigned int>(100.f * sinf(frameCount_ * 0.1f));
//				unsigned int color = (alpha) | 0xFFFFFF00;
//				Novice::DrawSprite(
//					-200, 0,
//					TextureManager::Instance().Get(TextureName::laserAlert),
//					1.f, 1.f,
//					0.f,
//					color
//				);
//			}
//		}
//
//#ifdef _DEBUG
//		ImGui::Begin("Debug Manager");
//		if (ImGui::BeginTabBar("GameTabs")) {
//			if (player_) { if (ImGui::BeginTabItem("Player")) { DebugManager::Instance().Inspect(player_, "Player Info"); ImGui::EndTabItem(); } }
//			if (boss_ != nullptr) { if (ImGui::BeginTabItem("Boss")) { DebugManager::Instance().InspectBoss(boss_, "Boss Info"); ImGui::EndTabItem(); } }
//			ImGui::EndTabBar();
//		}
//		ImGui::End();
//#endif
//	}
//
//	int GetNextGameObjectID() { return ++nextGameObjectID_; }
//
//	void ClearGameObjects() {
//		for (GameObject* obj : enemies_) delete obj; enemies_.clear();
//		for (GameObject* obj : bullets_) delete obj; bullets_.clear();
//		for (Collectible* item : items_) delete item; items_.clear();
//		if (player_) delete player_; player_ = nullptr; boss_ = nullptr; nextGameObjectID_ = -1;
//	}
//
//	void ClearInactiveObjects() {
//		auto removeInactive = [](std::vector<GameObject*>& objects) {
//			objects.erase(std::remove_if(objects.begin(), objects.end(), [](GameObject* obj) {
//				if (!obj->IsActive()) { delete obj; return true; }
//				return false; }), objects.end()); };
//		if (boss_ && !boss_->IsActive()) { boss_ = nullptr; }
//		if (player_ && !player_->GetInfo().isActive) { delete player_; player_ = nullptr; }
//		// Monitor dummy
//		if (dummy_ && !dummy_->IsActive()) { dummy_ = nullptr; }
//		removeInactive(enemies_);
//		removeInactive(bullets_);
//		ClearInactiveItems();
//
//		// After cleanup, if dummy died and boss not spawned, spawn boss
//		if (!bossSpawned_ && !dummy_) {
//			SpawnRequest bossSpawnRequest;
//			bossSpawnRequest.type = SpawnType::Boss;
//			bossSpawnRequest.transform.position = { 1000.f, 360.f };
//			bossSpawnRequest.transform.velocity = { 0.f, 0.f };
//			bossSpawnRequest.owner = nullptr;
//			g_spawnRequests.push_back(bossSpawnRequest);
//			bossSpawned_ = true;
//			SoundManager::Instance().NextBGMSet(SoundName::BGM_GamePlay1, true);
//
//			UIManager::Instance().SetIsGamePlay(true);
//			inGamePlay_ = true;
//			// if player too far away for the pos(0,0), teleport player closer to the pos(0,0)
//			if (player_) {
//				Vector2f toBoss = player_->GetTransform().position * -1.f;
//				if (toBoss.length() > 400.f) {
//					player_->GetTransform().velocity = player_->GetTransform().position / -1.f;
//
//					// set player nodes positions accordingly
//					player_->forEachTailNode([&](TailNodes* node) {
//						node->GetTransform().position = player_->GetTransform().position;
//						node->GetTransform().velocity = { 0.f, 0.f };
//						});
//				}
//			}
//		}
//	}
//
//	Player* GetPlayer() const { return player_; }
//
//	void HandleSpawnRequests() {
//		for (const SpawnRequest& request : g_spawnRequests) {
//			GameObject* newObject = nullptr;
//			switch (request.type) {
//			case SpawnType::Enemy:
//				newObject = new NormalEnemy(GetNextGameObjectID(), "Enemy");
//				if (auto* e = dynamic_cast<NormalEnemy*>(newObject)) e->SetPlayer(player_);
//				// Register with boss if owned
//				if (request.owner) {
//					if (auto* boss = dynamic_cast<Boss*>(request.owner)) {
//
//						boss->RegisterPhase0Spawn(dynamic_cast<NormalEnemy*>(newObject));
//					}
//				}
//				break;
//			case SpawnType::MediumEnemy:
//				newObject = new MediumEnemy(GetNextGameObjectID(), "Enemy");
//				if (auto* e = dynamic_cast<NormalEnemy*>(newObject)) e->SetPlayer(player_);
//				// Register with boss if owned
//				if (request.owner) {
//					if (auto* boss = dynamic_cast<Boss*>(request.owner)) {
//
//						boss->RegisterPhase0Spawn(dynamic_cast<NormalEnemy*>(newObject));
//					}
//				}
//				break;
//			case SpawnType::Boss:
//				newObject = new Boss(GetNextGameObjectID(), "Enemy");
//				boss_ = dynamic_cast<Boss*>(newObject);
//				boss_->SetPlayer(player_);
//				break;
//			case SpawnType::Bullet:
//				newObject = new Bullet(GetNextGameObjectID(), "Bullet");
//				break;
//			case SpawnType::BossBullet:
//				newObject = new BossBullet(GetNextGameObjectID(), "Bullet");
//				break;
//			case SpawnType::EnemySmallBullet:
//				newObject = new EnemySmallBullet(GetNextGameObjectID(), "Bullet");
//				break;
//			case SpawnType::BossShortRangeBullet:
//				newObject = new BossShortRangeBullet(GetNextGameObjectID(), "Bullet");
//				break;
//			case SpawnType::BossLaser: {
//				auto* laser = new BossLaser(GetNextGameObjectID(), "Bullet");
//				newObject = laser;
//				break;
//			}
//			case SpawnType::ExpPoint:
//				newObject = new ExpPoint(GetNextGameObjectID(), "ExpPoint");
//				items_.push_back(dynamic_cast<Collectible*>(newObject));
//				break;
//			case SpawnType::DroppedNode: {
//				auto* dropped = new DroppedNode(GetNextGameObjectID(), "DroppedNode");
//				newObject = dropped;
//				items_.push_back(dropped);
//				break;
//			}
//			case SpawnType::boostItem:
//				newObject = new BoostItem(GetNextGameObjectID(), "BoostItem");
//				items_.push_back(dynamic_cast<Collectible*>(newObject));
//				break;
//			case SpawnType::DroppedBigNode: {
//				auto* dropped = new DroppedBigNode(GetNextGameObjectID(), "DroppedBigNode");
//				newObject = dropped;
//				items_.push_back(dropped);
//				break;
//			}
//			case SpawnType::ChargingEnemy:
//				newObject = new ChargingEnemy(GetNextGameObjectID(), "Enemy");
//				if (auto* e = dynamic_cast<Enemy*>(newObject)) e->SetPlayer(player_);
//				break;
//			default: break;
//			}
//			if (newObject) {
//				newObject->GetTransform() = request.transform;
//				newObject->Initialize();
//				if (newObject->GetInfo().tag == "Enemy") {
//					enemies_.push_back(newObject);
//
//				}
//				else if (newObject->GetInfo().tag == "Bullet") {
//					if (auto* damageHitBox = dynamic_cast<DamageHitBox*>(newObject)) damageHitBox->SetOwner(request.owner);
//					bullets_.push_back(newObject);
//				}
//			}
//		}
//		ClearSpawnRequests();
//	}
//
//	void cameraUpdate(float deltaTime = 1.f) {
//		//camera_.setTarget(player_->GetTransform().position);
//		Vector2f midPoint = player_->GetTail().GetMidPoint();
//		if (midPoint == Vector2f(0.f, 0.f)) midPoint = player_->GetTransform().position;
//		camera_.setTarget(midPoint);
//
//		// zoom in when dummy is alive
//		if (dummy_) {
//			camera_.setTarget(dummy_->GetTransform().position);
//			if (camera_.scale_.x > 0.7) {
//				camera_.setScale(camera_.scale_ - Vector2f(0.01f, 0.01f));
//			}
//		}
//		else {
//			if (camera_.scale_.x < 1.1) {
//				camera_.setTarget(Vector2f(250.f, 0.f));
//				camera_.setScale(camera_.scale_ + Vector2f(0.005f, 0.005f));
//			}
//		}
//		if (boss_) {
//			if (boss_->isInPhase3()) {
//				//Novice::ScreenPrintf(100, 100, "Phase 3 Zooming Out");
//				if (camera_.scale_.x < 1.6f) {
//					camera_.shake(5.f, 10, true);
//					camera_.setScale(camera_.scale_ + Vector2f(0.001f, 0.001f));
//				}
//			}
//			else {
//				if (camera_.scale_.x > 1.105f) {
//					camera_.setScale(camera_.scale_ - Vector2f(0.005f, 0.005f));
//				}
//			}
//		}
//		// keep the boss in view
//		/*
//		if (boss_) {
//			Vector2f BossScreenPos = camera_.worldToScreen(boss_->GetTransform().position);
//			Vector2f playerToBoss = boss_->GetTransform().position - player_->GetTransform().position;
//			float angleToPlayer = atan2f(playerToBoss.y, playerToBoss.x);
//			while (angleToPlayer < 0.f) angleToPlayer += 6.2831853f;
//			while (angleToPlayer >= 6.2831853f) angleToPlayer -= 6.2831853f;
//			// (45~ 0) ||(360~360-45) || (180-45 ~ 180+45) -> 横方向
//			// determine the angle in 0 ~ 360
//			if (
//				(angleToPlayer > 3 * 3.14159265f / 4 && angleToPlayer < 5 * 3.14159265f / 4) ||
//				(angleToPlayer < 3.14159265f / 4 && angleToPlayer > 0)		||
//				(angleToPlayer > 7 * 3.14159265f / 4 && angleToPlayer < 2 * 3.14159265f)
//			) {
//				Novice::ScreenPrintf(100, 120, "Zooming X");
//				if (BossScreenPos.x > 1280.f * 0.9f) {
//					camera_.setScale(camera_.scale_ * 1.01f);
//
//
//				}
//				else if (BossScreenPos.x < 1280.f * 0.1f) {
//					camera_.setScale(camera_.scale_ * 1.01f);
//				}
//				else if (BossScreenPos.x > 1280.f * 0.2f && BossScreenPos.x < 1280.f * 0.8f) {
//
//					if (camera_.scale_.x > 1.f && camera_.scale_.y > 1.f) {
//
//						camera_.setScale(camera_.scale_ * 0.99f);
//					}
//				}
//			}
//			else {
//				Novice::ScreenPrintf(100, 140, "Zooming Y");
//				if (BossScreenPos.y > 720.f * 0.9f) {
//					camera_.setScale(camera_.scale_ * 1.01f);
//				}
//				else if (BossScreenPos.y < 720.f * 0.1f) {
//					camera_.setScale(camera_.scale_ * 1.01f);
//				}
//				else if (BossScreenPos.y > 720.f * 0.2f && BossScreenPos.y < 720.f * 0.8f) {
//					if (camera_.scale_.x > 1.f && camera_.scale_.y > 1.f) {
//						camera_.setScale(camera_.scale_ * 0.99f);
//					}
//				}
//			}
//		}
//		*/
//		// Update camera
//		camera_.update(deltaTime);
//		InputManager::Instance().SetCameraOffset(camera_.getRenderOffset());
//
//		//if (Novice::CheckHitKey(DIK_V)) camera_.setScale(camera_.scale_ * 1.1f);
//		//if (Novice::CheckHitKey(DIK_C)) camera_.setScale(camera_.scale_ * 0.9f);
//	}
//
//	void GameObjectUpdate(float deltaTime = 1.f) {
//		player_->Update(deltaTime);
//		for (GameObject* obj : enemies_) obj->Update(deltaTime);
//		for (GameObject* obj : bullets_) obj->Update(deltaTime);
//		if (boss_) {
//			/*if (InputManager::Instance().GetKeyState(DIK_R) && !InputManager::Instance().GetPreKeyState(DIK_R)) {
//				boss_->SetActive(false);
//			}*/
//			boss_->Update(deltaTime);
//		}
//		UpdateItems(deltaTime);
//	}
//};