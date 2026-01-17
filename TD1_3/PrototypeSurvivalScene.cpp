#include "PrototypeSurvivalScene.h"
#include "SceneManager.h"
#include "WindowSize.h"
#include "Novice.h"

// 実装クラスをインクルード
#include "SurvivalObjects.h"
#include "SurvivalGameManager.h"

#include "SceneUtilityIncludes.h"

PrototypeSurvivalScene::PrototypeSurvivalScene(SceneManager& manager)
    : sceneManager_(&manager) {

    input_ = std::make_unique<InputManager>();
    gameObjectManager_ = std::make_unique<SurvivalGameObjectManager>();

    camera_ = std::make_unique<Camera2D>(Vector2(kWindowWidth / 2, kWindowHeight / 2), Vector2(kWindowWidth, kWindowHeight));

    const int whiteTex = Tex().GetTexture(TextureId::White1x1);

    auto player = std::make_shared<SurvivalPlayer>(input_.get());
    if (player->GetDrawComponent()) {
        player->GetDrawComponent()->SetGraphHandle(whiteTex);
    }
    gameObjectManager_->AddObject(player, "Player");
    gameObjectManager_->SetPlayer(player);

    auto debrisCtrl = std::make_shared<DebrisController>(gameObjectManager_.get(), input_.get(), player);
    if (debrisCtrl->GetDrawComponent()) {
        debrisCtrl->GetDrawComponent()->SetGraphHandle(whiteTex);
    }
    gameObjectManager_->AddObject(debrisCtrl, "DebrisController");
    gameObjectManager_->SetDebrisController(debrisCtrl);
}

PrototypeSurvivalScene::~PrototypeSurvivalScene() {}

void PrototypeSurvivalScene::Update(float deltaTime, const char* keys, const char* preKeys) {
	keys; preKeys; // 未使用防止
    input_->Update();

	if (Input().PressKey(DIK_ESCAPE)) {
        sceneManager_->RequestTransition(SceneType::Title);
    }

    // スポーン処理
    enemySpawnTimer_ += deltaTime;
    if (enemySpawnTimer_ > 1.0f) {
        enemySpawnTimer_ = 0.0f;
        SpawnEnemy();
    }

    // カメラ演出制御
    UpdateCamera(deltaTime);

    // 全オブジェクト更新＆判定
    gameObjectManager_->Update(deltaTime);

    // カメラ更新
    camera_->Update(deltaTime);
}

void PrototypeSurvivalScene::SpawnEnemy() {
    // 画面外からランダムスポーン
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    float dist = 800.0f;
    Vector2 spawnPos = {
        kWindowWidth / 2.0f + cosf(angle) * dist,
        kWindowHeight / 2.0f + sinf(angle) * dist
    };

    // タンク率 20%
    EnemyType type = (rand() % 5 == 0) ? EnemyType::Tank : EnemyType::Normal;

    auto player = gameObjectManager_->GetPlayer();
    auto enemy = std::make_shared<SurvivalEnemy>(spawnPos, type, player);
	int enemyTex = (type == EnemyType::Tank) ? Tex().GetTexture(TextureId::White1x1) : Tex().GetTexture(TextureId::White1x1);
    if (enemy->GetDrawComponent()) {
        enemy->GetDrawComponent()->SetGraphHandle(enemyTex);
	}

    gameObjectManager_->AddObject(enemy, "Enemy");                                       
}

void PrototypeSurvivalScene::UpdateCamera(float dt) {
    auto debris = gameObjectManager_->GetDebrisController();
    if (!debris) return;

    // 発散範囲に応じてカメラをズームイン・アウト
    // 最大半径まで広がっているときは、戦場全体を見渡すために少し引く
    float targetZoom = 1.0f;
    if (debris->GetCurrentRadius() > 200.0f) {
        targetZoom = 1.4f; // 引く
    }
    else {
        targetZoom = 1.0f; // 寄る
    }

    // 滑らかにズーム
    float currentZoom = camera_->GetZoom();
    float newZoom = currentZoom + (targetZoom - currentZoom) * 2.0f * dt;
    camera_->SetZoom(newZoom);

    // クリティカル攻撃時のシェイクなどはオブジェクト側からカメラを叩くか、
    // ここで状態を見て叩く
    if (debris->IsContracting() && debris->IsCritical()) {
        // 攻撃中はずっと揺らす（ドリル感）
        camera_->Shake(2.0f, 0.1f);
    }
}

void PrototypeSurvivalScene::Draw() {
    // 背景
    Novice::DrawBox(0, 0, (int)kWindowWidth, (int)kWindowHeight,0.0f, 0x222233FF, kFillModeSolid);

    // カメラを適用して全オブジェクト描画
    gameObjectManager_->Draw(*camera_);

    // UIデバッグ
    Novice::ScreenPrintf(10, 10, "Objects: %d", gameObjectManager_.get()->GetObjectsSize()); // リストサイズ取得メソッドがあれば表示推奨
    Novice::ScreenPrintf(10, 30, "WASD: Move, SPACE: Expand/Contract");
}