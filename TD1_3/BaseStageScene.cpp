#include "BaseStageScene.h"
#include "SceneManager.h"
#include "WindowSize.h"
#include <Novice.h>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

BaseStageScene::BaseStageScene(SceneManager& manager, GameShared& shared, int stageIndex)
	: manager_(manager), shared_(shared), stageIndex_(stageIndex) {

	// 背景を初期化
	InitializeBackground();

	// カメラを初期化
	InitializeCamera();

	// BGM再生
	shared_.PlayExclusive_(BgmKind::Stage);
}

BaseStageScene::~BaseStageScene() {
	delete drawCompBackground_;
	delete camera_;
}

void BaseStageScene::InitializeBackground() {
	// 背景テクスチャをロード
	int bgTexture = Novice::LoadTexture("./Resources/images/gamePlay/background_ver1.png");

	// 新しい DrawComponent2D で背景を作成（静止画）
	drawCompBackground_ = new DrawComponent2D(bgTexture);

	// 背景の設定
	drawCompBackground_->SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
	drawCompBackground_->SetDrawSize(1280.0f, 720.0f);
	drawCompBackground_->SetAnchorPoint({ 0.5f, 0.5f });

	// オプション: 背景に微妙なエフェクトを追加
	// drawCompBackground_->StartPulse(0.99f, 1.01f, 1.0f, true);
}

void BaseStageScene::InitializeCamera() {
	// カメラを作成（画面中央、画面サイズ）
	camera_ = new Camera2D({ kWindowWidth / 2.0f, kWindowHeight / 2.0f },
		{ (float)kWindowWidth, (float)kWindowHeight });

	// カメラの初期設定
	camera_->SetZoom(1.0f);

	// 境界設定（必要に応じて派生クラスで変更）
	// camera_->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
}

void BaseStageScene::UpdateBackground(float deltaTime) {
	if (drawCompBackground_) {
		drawCompBackground_->Update(deltaTime);
	}
}

void BaseStageScene::Update(float dt, const char* keys, const char* pre) {
	// 初回のみ初期化
	if (!initialized_) {
		InitializeStage();
		initialized_ = true;
	}

	// ========================================
	// シーン遷移入力（共通）
	// ========================================

	// ESCキーでポーズ
	if (keys[DIK_ESCAPE] && !pre[DIK_ESCAPE]) {
		manager_.RequestPause();
		return;
	}

	// パッドのStartボタンでポーズ
	shared_.pad.Update();
	if (shared_.pad.Trigger(Pad::Button::Start)) {
		manager_.RequestPause();
		return;
	}

#ifdef _DEBUG
	// デバッグ用：ステージセレクトに戻る
	if (keys[DIK_F1] && !pre[DIK_F1]) {
		manager_.RequestTransition(SceneType::StageSelect);
	}

	// デバッグ用：リスタート
	if (keys[DIK_F5] && !pre[DIK_F5]) {
		initialized_ = false; // 再初期化フラグ
	}
#endif

	// 背景を更新
	UpdateBackground(dt);

	// カメラを更新
	if (camera_) {
		camera_->Update(dt);
	}

	// ========================================
	// ステージ固有の更新処理
	// ========================================
	UpdateStage(dt, keys, pre);
}

void BaseStageScene::Draw() {
	if (!initialized_) {
		return;
	}

	// ========================================
	// 背景描画（スクリーン座標）
	// ========================================
	if (drawCompBackground_) {
		drawCompBackground_->DrawScreen();
	}

	// ========================================
	// ステージ固有の描画処理（カメラを使用）
	// ========================================
	DrawStage();

#ifdef _DEBUG
	// デバッグ情報
	ImGui::Begin("Base Stage Debug");

	ImGui::Text("=== Stage Info ===");
	ImGui::Text("Stage Index: %d", stageIndex_);
	ImGui::Text("Initialized: %s", initialized_ ? "Yes" : "No");

	ImGui::Separator();

	ImGui::Text("=== Background ===");
	if (drawCompBackground_) {
		Vector2 bgPos = drawCompBackground_->GetPosition();
		Vector2 bgSize = drawCompBackground_->GetDrawSize();
		ImGui::Text("Position: (%.1f, %.1f)", bgPos.x, bgPos.y);
		ImGui::Text("Size: (%.1f, %.1f)", bgSize.x, bgSize.y);

		if (ImGui::Button("Add Background Pulse")) {
			drawCompBackground_->StartPulse(0.99f, 1.01f, 1.0f, true);
		}

		if (ImGui::Button("Stop Background Effects")) {
			drawCompBackground_->StopAllEffects();
		}
	}

	ImGui::Separator();

	ImGui::Text("=== Camera ===");
	if (camera_) {
		Vector2 camPos = camera_->GetPosition();
		float camZoom = camera_->GetZoom();
		ImGui::Text("Position: (%.1f, %.1f)", camPos.x, camPos.y);
		ImGui::Text("Zoom: %.2f", camZoom);

		if (ImGui::Button("Reset Camera")) {
			camera_->SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
			camera_->SetZoom(1.0f);
		}

		if (ImGui::Button("Camera Shake")) {
			camera_->Shake(10.0f, 0.5f);
		}
	}

	ImGui::Separator();

	ImGui::Text("=== Controls ===");
	ImGui::Text("ESC / Start: Pause");
	ImGui::Text("F1: Stage Select (Debug)");
	ImGui::Text("F5: Restart (Debug)");

	ImGui::End();
#endif
}