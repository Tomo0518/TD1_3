#include "GamePlayScene.h"
#include "SceneManager.h"
#include "DebugWindow.h"

#include <Novice.h>

#include "Player.h"
#include "Usagi.hpp"
#include "KinokoSpawner.hpp"
#include "WorldOrigin.h"
#include "ObjectRegistry.h"
#include "ParticleManager.h"
#include "Button.hpp"
#include "Door.hpp"

#include "PhysicsManager.h"
#include "UIManager.h"

#include "SceneUtilityIncludes.h"

GamePlayScene::GamePlayScene(SceneManager& mgr)
	: manager_(mgr) {

	Initialize();

	particleManager_ = &ParticleManager::GetInstance();

	const float groundY = 0.0f;
	particleManager_->SetGroundLevel(groundY);

	debugWindow_ = std::make_unique<DebugWindow>();
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
	UIManager::GetInstance().SetIsGamePlay(true);

	fade_ = 0.0f;

	objectManager_.Clear();
	player_ = nullptr;
	worldOrigin_ = nullptr;

	// 背景マネージャー初期化
	backgroundManager_ = std::make_unique<BackgroundManager>();

	// シングルトンを使う
	auto& mapData = MapData::GetInstance();
	mapData.Load("./Resources/data/stage1.json");

	// 1. タイル、オブジェクト定義の初期化
	TileRegistry::Initialize();
	ObjectRegistry::Initialize();

#ifdef _DEBUG
	// --- マップシステムの初期化 ---
	// 1. エディタ初期化（タイル定義のロード）
	mapEditor_.Initialize(&mapManager_);
#endif
	ParticleManager::GetInstance().LoadParamsFromJson("./Resources/Data/particle_params.json");

	// 3. マップ描画クラスの初期化
	mapChip_.Initialize();

	// 4. マップマネージャー初期化
	mapManager_.Initialize();

#ifdef _DEBUG
	// エディタにマップマネージャーをセット
	mapEditor_.SetMapManager(&mapManager_);
#endif

	InitializeCamera();
	camera_->SetZoom(0.75f);

	InitializeObjects(); // ここでObject生成（マップデータから自動生成）
	InitializeBackground();

	// カメラ初期位置設定
	if (player_) {
		camera_->SetPosition(player_->GetPosition());
	}
}

void GamePlayScene::InitializeCamera() {
	const bool isWorldYUp = true;
	camera_ = std::make_unique<Camera2D>(Vector2{ 640.0f, 360.0f }, Vector2{ 1280.0f, 720.0f }, isWorldYUp);
	camera_->SetFollowSpeed(0.1f);
	camera_->SetDeadZone(150.0f, 100.0f);
	camera_->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
}

void GamePlayScene::InitializeObjects() {
	// シングルトンを使う
	auto& mapData = MapData::GetInstance();
	const auto& spawns = mapData.GetObjectSpawns();

	for (const auto& spawn : spawns) {
		SpawnObjectFromData(spawn);
	}

	// Playerが生成されていない場合はデフォルト位置に配置
	if (!player_) {
		player_ = objectManager_.Spawn<Usagi>(nullptr, "Player");
		player_->SetPosition({ 12000.0f, 12000.0f });
		//player_->SetManager(&objectManager_);
	}

	// WorldOriginが見つからない場合はデフォルト位置に生成
	if (!worldOrigin_) {
		Novice::ConsolePrintf("[GamePlayScene] No WorldOrigin found, creating at default position\n");
		worldOrigin_ = objectManager_.Spawn<WorldOrigin>(nullptr, "WorldOrigin");
		worldOrigin_->SetPosition({ 12000.0f, 12000.0f });
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
			worldOrigin_ = objectManager_.Spawn<WorldOrigin>(nullptr, "WorldOrigin");
			worldOrigin_->SetPosition(spawn.position);
			Novice::ConsolePrintf("[GamePlayScene] Spawned WorldOrigin at (%.1f, %.1f)\n",
				spawn.position.x, spawn.position.y);
		}
		break;

	case 100: // PlayerStart
		if (!player_) {
			player_ = objectManager_.Spawn<Usagi>(nullptr, "Player");
			player_->SetPosition(spawn.position);
			player_->Initialize();
			Novice::ConsolePrintf("[GamePlayScene] Spawned Player at (%.1f, %.1f)\n",
				spawn.position.x, spawn.position.y);
			//player_->SetManager(&objectManager_);
		}
		break;

		// 追加するオブジェクトタイプはここに追加
	case 101: { // Enemy_Normal
		auto* enemy = objectManager_.Spawn<Enemy>(nullptr, "Enemy");
		enemy->SetPosition(spawn.position);
		enemy->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Enemy at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}

	case 102: { // Enemy_spawner
		auto* spawner = objectManager_.Spawn<KinokoSpawner>(nullptr, "KinokoSpawner");
		spawner->SetPosition(spawn.position);
		spawner->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned KinokoSpawner at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 103: { // AttackEnemy
		auto* enemy = objectManager_.Spawn<AttackEnemy>(nullptr, "Enemy");
		enemy->SetPosition(spawn.position);
		enemy->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned AttackEnemy at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 104: { // AttackEnemySpawner
		auto* spawner = objectManager_.Spawn<AttackKinokoSpawner>(nullptr, "KinokoSpawner");
		spawner->SetPosition(spawn.position);
		spawner->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned AttackEnemySpawner at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 300: { // Button1
		auto* button = objectManager_.Spawn<Button1>(nullptr, "Button");
		button->SetPosition(spawn.position);
		button->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Button at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 400: {
		auto* door = objectManager_.Spawn<Door1>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 301: { // Button2
		auto* button = objectManager_.Spawn<Button2>(nullptr, "Button");
		button->SetPosition(spawn.position);
		button->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Button at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 401: {
		auto* door = objectManager_.Spawn<Door2>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}

	default:
		Novice::ConsolePrintf("[GamePlayScene] Unknown object type: %d\n", spawn.objectTypeId);
		break;
	}
}

void GamePlayScene::InitializeBackground() {
	backgroundManager_->Clear();

	// ベース背景
	backgroundManager_->AddLayer(
		TextureId::Background_Base,
		0.0f,    // カメラの0%の速度
		1280.0f  // テクスチャ幅
	);

	// レイヤー1: 洞窟の一番遠いレイヤー
	backgroundManager_->AddLayer(
		TextureId::Background_Far,
		0.1f,    // カメラの10%の速度
		1280.0f  // テクスチャ幅
	);

	// レイヤー2: 洞窟の中景
	backgroundManager_->AddLayer(
		TextureId::Background_Middle,
		0.3f,
		1280.0f
	);

	// レイヤー3: 洞窟の近景
	backgroundManager_->AddLayer(
		TextureId::Background_Near,
		0.6f,
		1280.0f
	);

	// プレイヤースポーン後、カメラ初期位置を設定
	Vector2 spawnPos = { player_->GetPosition() };
	backgroundManager_->SetInitialCameraPosition(spawnPos);
}

void GamePlayScene::Update(float dt, const char* keys, const char* pre) {
	UIManager::GetInstance().Update(dt);

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
		if (Input().TriggerKey(DIK_X)) {
			isDebugCameraMove_ = !isDebugCameraMove_;
			camera_->SetDebugCamera(isDebugCameraMove_);
		}
		camera_->DebugMove();
		Novice::ScreenPrintf(10, 40, "(Z)Debug Camera Move: %s", isDebugCameraMove_ ? "ON" : "OFF");
	}
#endif

	// 動的タイルの更新(カリングとアニメーション更新)
	mapManager_.Update(dt, *camera_);

	// GameObjectManager 経由で更新
	objectManager_.Update(dt);

	// 当たり判定（物理演算)
	CheckCollisions();
	/*auto& mapData = MapData::GetInstance();
	if (player_) {
		PhysicsManager::ResolveMapCollision(player_, mapData);
	}*/

	// GameObjectManager でオブジェクト更新（移動処理）
	//objectManager_.Update(dt);

	// パーティクル
	particleManager_->Update(dt);



	// テスト入力（player_ は参照として使える）
	if (player_) {
		/*if (Input().TriggerKey(DIK_SPACE)) {
			particleManager_->Emit(ParticleType::Explosion, player_->GetPosition());
		}*/

		if (Input().TriggerKey(DIK_J)) {
			particleManager_->Emit(ParticleType::Debris, player_->GetPosition());
		}
		if (Input().TriggerKey(DIK_L)) {
			particleManager_->Emit(ParticleType::Hit, player_->GetPosition());
		}
	}

#ifdef _DEBUG
	if(Input().TriggerKey(DIK_H)) {
		player_->TakeDamage(10.0f);
	}
#endif

	float playerHp = player_ ? player_->GetCurrentHp() : 0.0f;
	float playerMaxHp = player_ ? player_->GetMaxHp() : 1.0f;

	UIManager::GetInstance().SetPlayerHP(playerHp, playerMaxHp);
	UIManager::GetInstance().Update(dt);

	if (camera_) {
		camera_->Update(dt);
	}
}

void GamePlayScene::Draw() {
	auto& mapData = MapData::GetInstance();

	// 背景描画
	backgroundManager_->Draw(*camera_);

	// 背景装飾用マップチップ描画
	mapChip_.DrawBackgroundDecorationBlock(*camera_, mapData);

	//========================
	// マップ描画 
	// =======================
	// 動的タイル描画
	mapManager_.Draw(*camera_);

	// 静的タイル描画
	mapChip_.Draw(*camera_, mapData);

	particleManager_->Draw(*camera_);

	// GameObjectManager 経由で描画
	if (camera_) {
		objectManager_.Draw(*camera_);
	}

	UIManager::GetInstance().Draw();

#ifdef _DEBUG

	// エディタにもシングルトンを渡す
	mapEditor_.UpdateAndDrawImGui(mapData, *camera_);

	if (debugWindow_) {
		debugWindow_->DrawDebugGui();
		debugWindow_->DrawCameraDebugWindow(camera_.get());
		debugWindow_->DrawPlayerDebugWindow(player_);
		debugWindow_->DrawParticleDebugWindow(particleManager_, player_);
	}
#endif
}

void GamePlayScene::CheckCollisions() {
	std::vector<GameObject2D*> objects = objectManager_.GetAllObjects();

	PhysicsManager::ResolveObjectsCollisions(objects);

}