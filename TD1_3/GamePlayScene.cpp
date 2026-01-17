#include "GamePlayScene.h"
#include "SceneManager.h"
#include "DebugWindow.h"

#include <Novice.h>

#include "Player.h"
#include "WorldOrigin.h"
#include "ParticleManager.h"

#include "PhysicsManager.h"

#include "SceneUtilityIncludes.h"

GamePlayScene::GamePlayScene(SceneManager& mgr)
	: manager_(mgr) {

	Initialize();

	particleManager_ = new ParticleManager();

	const float groundY = 0.0f;
	particleManager_->SetGroundLevel(groundY);

	debugWindow_ = std::make_unique<DebugWindow>();
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
	fade_ = 0.0f;

	objectManager_.Clear();
	player_ = nullptr;
	worldOrigin_ = nullptr;

	// --- マップシステムの初期化 ---
		// 1. エディタ初期化（タイル定義のロード）
	mapEditor_.Initialize();

	// 2. マップデータ読み込み
	mapData_.Load("./Resources/data/stage1.json");

	// 3. マップ描画クラスの初期化
	mapChip_.Initialize();

	InitializeCamera();
	InitializeObjects(); // ここでObject生成（マップデータから自動生成）
	InitializeBackground();
}

void GamePlayScene::InitializeCamera() {
	const bool isWorldYUp = true;
	camera_ = std::make_unique<Camera2D>(Vector2{ 640.0f, 360.0f }, Vector2{ 1280.0f, 720.0f }, isWorldYUp);
	camera_->SetFollowSpeed(0.1f);
	camera_->SetDeadZone(150.0f, 100.0f);
	camera_->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
}

void GamePlayScene::InitializeObjects() {
	// MapDataからオブジェクトスポーン情報を取得
	const auto& spawns = mapData_.GetObjectSpawns();

	for (const auto& spawn : spawns) {
		SpawnObjectFromData(spawn);
	}

	// Playerが生成されていない場合はデフォルト位置に配置
	/*if (!player_) {
		Novice::ConsolePrintf("[GamePlayScene] No player spawn found, using default position\n");
		player_ = objectManager_.Spawn<Player>(nullptr, "Player");
		player_->SetPosition({ 640.0f, 560.0f });
	}*/

	// WorldOriginが見つからない場合はデフォルト位置に生成
	if (!worldOrigin_) {
		Novice::ConsolePrintf("[GamePlayScene] No WorldOrigin found, creating at default position\n");
		worldOrigin_ = objectManager_.Spawn<WorldOrigin>(nullptr,"WorldOrigin");
		worldOrigin_->SetPosition({ 640.0f, 360.0f });
	}

	// カメラ追従設定
	if (camera_ && player_) {
		camera_->SetTarget(&player_->GetPositionRef());
	}
}

void GamePlayScene::SpawnObjectFromData(const ObjectSpawnInfo& spawn) {
	switch (spawn.objectTypeId) {
	case 0: // WorldOrigin（拠点・ワールド座標の原点）
		if (!worldOrigin_) {
			worldOrigin_ = objectManager_.Spawn<WorldOrigin>(nullptr,"WorldOrigin");
			worldOrigin_->SetPosition(spawn.position);
			Novice::ConsolePrintf("[GamePlayScene] Spawned WorldOrigin at (%.1f, %.1f)\n",
				spawn.position.x, spawn.position.y);
		}
		break;

	case 100: // PlayerStart
		if (!player_) {
			player_ = objectManager_.Spawn<Player>(nullptr, "Player");
			player_->SetPosition(spawn.position);
			Novice::ConsolePrintf("[GamePlayScene] Spawned Player at (%.1f, %.1f)\n",
				spawn.position.x, spawn.position.y);
		}
		break;

		// 追加するオブジェクトタイプはここに追加
		// case 101: // Enemy_Normal
		//     auto* enemy = objectManager_.Spawn<Enemy>(nullptr, "Enemy");
		//     enemy->SetPosition(spawn.position);
		//     break;

	default:
		Novice::ConsolePrintf("[GamePlayScene] Unknown object type: %d\n", spawn.objectTypeId);
		break;
	}
}

void GamePlayScene::InitializeBackground() {
	background_.clear();

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background0_0)));
	background_[0]->SetPosition({ -kWindowWidth, kWindowHeight * 2 });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::BackgroundBlack)));
	background_[1]->SetPosition({ 0.0f, kWindowHeight * 2 });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background0_2)));
	background_[2]->SetPosition({ kWindowWidth, kWindowHeight * 2 });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background1_0)));
	background_[3]->SetPosition({ -kWindowWidth, kWindowHeight });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background1_1)));
	background_[4]->SetPosition({ 0.0f, kWindowHeight });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background1_2)));
	background_[5]->SetPosition({ kWindowWidth, kWindowHeight });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background2_0)));
	background_[6]->SetPosition({ -kWindowWidth, 0.0f });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background2_1)));
	background_[7]->SetPosition({ 0.0f, 0.0f });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background2_2)));
	background_[8]->SetPosition({ kWindowWidth, 0.0f });
}

void GamePlayScene::Update(float dt, const char* keys, const char* pre) {
	if (fade_ < 1.0f) {
		fade_ += dt * 4.0f;
	}

	const bool openPause =
		Input().GetPad()->Trigger(Pad::Button::Start) ||
		(!pre[DIK_ESCAPE] && keys[DIK_ESCAPE]) ||
		(!pre[DIK_RETURN] && keys[DIK_RETURN]);

	if (openPause) {
		Sound().PlaySe(SeId::Decide);
		manager_.RequestOpenPause();
		return;
	}

#ifdef _DEBUG
	if (camera_) {
		if (Input().TriggerKey(keys[DIK_C])) {
			isDebugCameraMove_ = !isDebugCameraMove_;
		}

		camera_->DebugMove(isDebugCameraMove_, keys, pre);
	}
	else if (camera_) {
		camera_->DebugMove(false, keys, pre);
	}
#endif

	// GameObjectManager 経由で更新
	objectManager_.Update(dt);

	// 当たり判定（物理演算)
	// オブジェクトが移動した後、マップとのめり込みを解消
	if (player_) {
		PhysicsManager::ResolveMapCollision(player_, mapData_);
	}

	// GameObjectManager でオブジェクト更新（移動処理）
	objectManager_.Update(dt);

	// パーティクル
	particleManager_->Update(dt);

	// テスト入力（player_ は参照として使える）
	if (player_) {

		if (Input().TriggerKey(DIK_SPACE)) {
			particleManager_->Emit(ParticleType::Explosion, player_->GetPosition());
		}
		if (Input().TriggerKey(DIK_J)) {
			particleManager_->Emit(ParticleType::Debris, player_->GetPosition());
		}
		if (Input().TriggerKey(DIK_L)) {
			particleManager_->Emit(ParticleType::Hit, player_->GetPosition());
		}
	}

	if (camera_) {
		camera_->Update(dt);
	}
}

void GamePlayScene::Draw() {
	for (auto& background : background_) {
		background->Draw(*camera_);
	}

	// --- マップ描画 ---
	// オブジェクトより奥（背景より手前）に描画
	mapChip_.Draw(*camera_, mapData_);

	particleManager_->Draw(*camera_);

	// GameObjectManager 経由で描画
	if (camera_) {
		objectManager_.Draw(*camera_);
	}

	// --- エディタの更新（ImGui）---
	// ゲーム中でも編集できるようにする
	mapEditor_.UpdateAndDrawImGui(mapData_, *camera_);

#ifdef _DEBUG
	if (debugWindow_) {
		debugWindow_->DrawDebugGui();
		debugWindow_->DrawCameraDebugWindow(camera_.get());
		debugWindow_->DrawPlayerDebugWindow(player_);
		debugWindow_->DrawParticleDebugWindow(particleManager_, player_);
	}
#endif
}