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
#include "UsagiCheckPoint.hpp"

#include "PhysicsManager.h"
#include "UIManager.h"

#include "SceneUtilityIncludes.h"


// Tips System
#include "TipsManager.h"
#include "TipsTrigger.hpp"

GamePlayScene::GamePlayScene(SceneManager& mgr)
	: manager_(mgr) {

	Initialize();

	particleManager_ = &ParticleManager::GetInstance();

	const float groundY = 0.0f;
	particleManager_->SetGroundLevel(groundY);

	debugWindow_ = std::make_unique<DebugWindow>();

	SoundManager::GetInstance().PlayBgm(BgmId::Tutorial);
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {

	fade_ = 0.0f;

	objectManager_.Clear();
	player_ = nullptr;
	worldOrigin_ = nullptr;

	// 背景マネージャー初期化
	backgroundManager_ = std::make_unique<BackgroundManager>();


	UIManager::GetInstance().SetGamePlay(true);


	// 1. タイル、オブジェクト定義の初期化
	TileRegistry::Initialize();
	ObjectRegistry::Initialize();


	// Tips System 初期化
	InitializeTipsSystem();

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
	camera_ = Camera2D::GetInstancePtr();
	camera_->Initialize({ 640.0f, 360.0f }, { 1280.0f, 720.0f }, isWorldYUp);
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
	case 99: {
		auto* checkPoint = objectManager_.Spawn<UsagiCheckPoint>(nullptr, "UsagiCheckPoint");
		checkPoint->SetPosition(spawn.position);
		checkPoint->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned UsagiCheckPoint at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}

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
	case 105: { // FatEnemy
		auto* enemy = objectManager_.Spawn<FatEnemy>(nullptr, "Enemy");
		enemy->SetPosition(spawn.position);
		enemy->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned FatEnemy at (%.1f, %.1f)\n",
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
	case 302: { // Button3
		auto* button = objectManager_.Spawn<Button3>(nullptr, "Button");
		button->SetPosition(spawn.position);
		button->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Button at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 402: {
		auto* door = objectManager_.Spawn<Door3>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 303: { // Button4
		auto* button = objectManager_.Spawn<Button4>(nullptr, "Button");
		button->SetPosition(spawn.position);
		button->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Button at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 403: {
		auto* door = objectManager_.Spawn<Door4>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 304: { // Button5
		auto* button = objectManager_.Spawn<Button5>(nullptr, "Button");
		button->SetPosition(spawn.position);
		button->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Button at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 404: {
		auto* door = objectManager_.Spawn<Door5>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 305: { // Button6
		auto* button = objectManager_.Spawn<Button6>(nullptr, "Button");
		button->SetPosition(spawn.position);
		button->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Button at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 405: {
		auto* door = objectManager_.Spawn<Door6>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 306: { // EnemyEvent
		auto* event = objectManager_.Spawn<EnemyEvent>(nullptr, "Button");
		event->SetPosition(spawn.position);
		event->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 406: { // Door_EnemyEvent
		auto* door = objectManager_.Spawn<EventDoor>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door_EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 307: { // EnemyEvent
		auto* event = objectManager_.Spawn<EnemyEvent2>(nullptr, "Button");
		event->SetPosition(spawn.position);
		event->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 407: { // Door_EnemyEvent
		auto* door = objectManager_.Spawn<EventDoor2>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door_EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 308: { // EnemyEvent
		auto* event = objectManager_.Spawn<EnemyEvent3>(nullptr, "Button");
		event->SetPosition(spawn.position);
		event->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 408: { // Door_EnemyEvent
		auto* door = objectManager_.Spawn<EventDoor3>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door_EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 309: { // EnemyEvent
		auto* event = objectManager_.Spawn<EnemyEvent4>(nullptr, "Button");
		event->SetPosition(spawn.position);
		event->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	case 409: { // Door_EnemyEvent
		auto* door = objectManager_.Spawn<EventDoor4>(nullptr, "Door");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawned Door_EnemyEvent at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}

	case 500: { // TipsTrigger
		// customDataからtipsIdを取得
		int tipsId = 1; // デフォルト値
		if (spawn.customData.contains("tipsId")) {
			tipsId = spawn.customData["tipsId"].get<int>();
		}

		// Spawn<T>(owner, tag, コンストラクタの引数...)
		auto* trigger = objectManager_.Spawn<TipsTrigger>(
			nullptr,              // owner
			"TipsTrigger_1",        // tag
			spawn.objectTypeId,   // id (TipsTriggerのコンストラクタ第1引数)
			"TipsTrigger_1",        // name (TipsTriggerのコンストラクタ第2引数)
			spawn.position,       // position (TipsTriggerのコンストラクタ第3引数)
			tipsId                // tipsId (TipsTriggerのコンストラクタ第4引数)
		);

		// プレイヤーへの参照を設定
		if (player_) {
			trigger->SetPlayer(player_);
		}

		Novice::ConsolePrintf("[GamePlayScene] Spawned TipsTrigger (ID:%d) at (%.1f, %.1f)\n",
			tipsId, spawn.position.x, spawn.position.y);
		break;
	}

	case 501: { // TipsTrigger (別ID)
		// customDataからtipsIdを取得
		int tipsId = 2; // デフォルト値
		if (spawn.customData.contains("tipsId")) {
			tipsId = spawn.customData["tipsId"].get<int>();
		}
		// Spawn<T>(owner, tag, コンストラクタの引数...)
		auto* trigger = objectManager_.Spawn<TipsTrigger>(
			nullptr,              // owner
			"TipsTrigger_2",        // tag
			spawn.objectTypeId,   // id (TipsTriggerのコンストラクタ第1引数)
			"TipsTrigger_2",        // name (TipsTriggerのコンストラクタ第2引数)
			spawn.position,       // position (TipsTriggerのコンストラクタ第3引数)
			tipsId                // tipsId (TipsTriggerのコンストラクタ第4引数)
		);
		// プレイヤーへの参照を設定
		if (player_) {
			trigger->SetPlayer(player_);
		}
		Novice::ConsolePrintf("[GamePlayScene] Spawned TipsTrigger (ID:%d) at (%.1f, %.1f)\n",
			tipsId, spawn.position.x, spawn.position.y);
		break;
	}

	case 502: { // TipsTrigger (別ID)
		// customDataからtipsIdを取得
		int tipsId = 3; // デフォルト値
		if (spawn.customData.contains("tipsId")) {
			tipsId = spawn.customData["tipsId"].get<int>();
		}
		// Spawn<T>(owner, tag, コンストラクタの引数...)
		auto* trigger = objectManager_.Spawn<TipsTrigger>(
			nullptr,              // owner
			"TipsTrigger_3",        // tag
			spawn.objectTypeId,   // id (TipsTriggerのコンストラクタ第1引数)
			"TipsTrigger_3",        // name (TipsTriggerのコンストラクタ第2引数)
			spawn.position,       // position (TipsTriggerのコンストラクタ第3引数)
			tipsId                // tipsId (TipsTriggerのコンストラクタ第4引数)
		);
		// プレイヤーへの参照を設定
		if (player_) {
			trigger->SetPlayer(player_);
		}
		Novice::ConsolePrintf("[GamePlayScene] Spawned TipsTrigger (ID:%d) at (%.1f, %.1f)\n",
			tipsId, spawn.position.x, spawn.position.y);
		break;
	}

	case 503: { // TipsTrigger (別ID)
		// customDataからtipsIdを取得
		int tipsId = 4; // デフォルト値
		if (spawn.customData.contains("tipsId")) {
			tipsId = spawn.customData["tipsId"].get<int>();
		}
		// Spawn<T>(owner, tag, コンストラクタの引数...)
		auto* trigger = objectManager_.Spawn<TipsTrigger>(
			nullptr,              // owner
			"TipsTrigger_4",        // tag
			spawn.objectTypeId,   // id (TipsTriggerのコンストラクタ第1引数)
			"TipsTrigger_4",        // name (TipsTriggerのコンストラクタ第2引数)
			spawn.position,       // position (TipsTriggerのコンストラクタ第3引数)
			tipsId                // tipsId (TipsTriggerのコンストラクタ第4引数)
		);
		// プレイヤーへの参照を設定
		if (player_) {
			trigger->SetPlayer(player_);
		}
		Novice::ConsolePrintf("[GamePlayScene] Spawned TipsTrigger (ID:%d) at (%.1f, %.1f)\n",
			tipsId, spawn.position.x, spawn.position.y);
		break;
	}

	case 999:{
		auto* door = objectManager_.Spawn<EndButton>(nullptr, "EndButton");
		door->SetPosition(spawn.position);
		door->Initialize();
		Novice::ConsolePrintf("[GamePlayScene] Spawn point for debugging at (%.1f, %.1f)\n",
			spawn.position.x, spawn.position.y);
		break;
	}
	default:
		Novice::ConsolePrintf("[GamePlayScene] Unknown object type: %d\n", spawn.objectTypeId);
		break;
	}
}

void GamePlayScene::InitializeTipsSystem() {
	// TipsManagerを初期化（データ層、起動時に毎回リセット）
	TipsManager::GetInstance().Initialize();

	// TipsUIDrawerを初期化（表示層）
	tipsUIDrawer_ = std::make_unique<TipsUIDrawer>();
	tipsUIDrawer_->Initialize();

	// TipsCollectionUIを初期化（一覧画面）
	tipsCollectionUI_ = std::make_unique<TipsCollectionUI>();
	tipsCollectionUI_->Initialize();
}

void GamePlayScene::InitializeBackground() {
	backgroundManager_->Clear();

	// ベース背景
	backgroundManager_->AddLayer(
		TextureId::Background_Base,
		0.0f,    // カメラの0%の速度
		"base",
		1280.0f  // テクスチャ幅
	);

	// レイヤー1: 洞窟の一番遠いレイヤー
	backgroundManager_->AddLayer(
		TextureId::Background_Far,
		0.1f,    // カメラの10%の速度
		"far",
		1280.0f  // テクスチャ幅
	);

	// レイヤー2: 洞窟の中景
	backgroundManager_->AddLayer(
		TextureId::Background_Middle,
		0.3f,
		"middle",
		1280.0f
	);

	// レイヤー3: 洞窟の近景
	/*backgroundManager_->AddLayer(
		TextureId::Background_Near,
		0.6f,
		"near",
		1280.0f
	);*/

	backgroundManager_->AddLayer(
		TextureId::Background_Near,
		0.8f,  // X方向スクロール速度
		0.3f,  // Y方向スクロール速度（30%）
		"near",
		1280.0f,  // X方向の繰り返し幅
		1280.0f    // Y方向の繰り返し高さ
	);

	// レイヤー4: 前景の岩
	/*backgroundManager_->AddLayer(
		TextureId::Background_Foreground,
		0.8f,
		"foreground",
		1280.0f
	);*/



	// プレイヤースポーン後、カメラ初期位置を設定
	Vector2 spawnPos = { player_->GetPosition() };
	backgroundManager_->SetInitialCameraPosition(spawnPos);
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

	// Tips UI更新  
	if (tipsUIDrawer_) {  
		tipsUIDrawer_->Update(dt);
	} 

	// Tips一覧UI更新
	if (tipsCollectionUI_) {
		tipsCollectionUI_->Update(dt);
	}


	// ***************** START check if game finished **************************
	auto buttons = objectManager_.GetObjectsByTag("EndButton");
	bool allPressed = true;
	int count = 0;
	for (auto& buttonObj : buttons) {
		Button* button = dynamic_cast<Button*>(buttonObj);
		if (button) {
			count++;
			if (!button->IsPressed()) {
				allPressed = false;
				break;
			}
		}
	}
	if (count == 0) allPressed = false; // No buttons found with the target ID
	if (allPressed) manager_.RequestTransition(SceneType::Result);
		
	// ************************** END check if game finished ***********************************


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


	// ========= UI更新 =========
	//UIManager::GetInstance().SetPlayerHP((float)player_->GetStatus().currentHP, (float)player_->GetStatus().maxHP);
	//UIManager::GetInstance().Update(dt);
	//UIManager::GetInstance().UpdateIcons(dt, player_ ? player_->GetSkillState() : PlayerSkillState{});
	 // プレイヤーのスキル状態を取得
	if (player_) {
		PlayerSkillState skillState = player_->GetSkillState();
		UIManager::GetInstance().UpdateIcons(dt, skillState);
	}

	UIManager::GetInstance().Update(dt);
	UIManager::GetInstance().DrawImGui(); // ImGui描画

	// プレイヤーHPの更新
	if (player_) {
		float hpRatio = (float)player_->GetStatus().currentHP / (float)player_->GetStatus().maxHP;
		UIManager::GetInstance().SetPlayerHP(hpRatio);
	}

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
	mapManager_.Draw(*camera_,DrawLayer::Background);

	// 静的タイル描画
	mapChip_.Draw(*camera_, mapData, 1/*mapEditor_.GetBlockLayerAlpha()*/);

	particleManager_->Draw(*camera_);

	// GameObjectManager 経由で描画
	if (camera_) {
		objectManager_.Draw(*camera_);
	}


	// 動的タイル描画
	mapManager_.Draw(*camera_, DrawLayer::Foreground);

	// 前景背景描画
	backgroundManager_->Draw(*camera_, "foreground");


	UIManager::GetInstance().Draw();

	// Tips UI描画（最前面）
	if (tipsUIDrawer_) { 
		tipsUIDrawer_->Draw(); 
	} 
	// Tips一覧UI描画（最前面）
	if (tipsCollectionUI_) {
		tipsCollectionUI_->Draw();
	}


#ifdef _DEBUG

	// エディタにもシングルトンを渡す
	mapEditor_.UpdateAndDrawImGui(mapData, *camera_);

	if (debugWindow_) {
		debugWindow_->DrawDebugGui();
		debugWindow_->DrawCameraDebugWindow(camera_);
		debugWindow_->DrawPlayerDebugWindow(player_);
		debugWindow_->DrawParticleDebugWindow(particleManager_, player_);
	}
#endif

	// Tips一覧UIのデバッグウィンドウ
	if (tipsCollectionUI_) {
		tipsCollectionUI_->DrawImGui();
	}
}

void GamePlayScene::CheckCollisions() {
	std::vector<GameObject2D*> objects = objectManager_.GetAllObjects();

	PhysicsManager::ResolveObjectsCollisions(objects);

}