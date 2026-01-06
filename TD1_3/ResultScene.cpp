#include "ResultScene.h"
#include <Novice.h>
#include "SceneManager.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

ResultScene::ResultScene(SceneManager& mgr, GameShared& shared)
	: manager_(mgr), shared_(shared) {

	// フォント読み込み
	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}

	// 描画コンポーネントを初期化
	InitializeDrawComponents();

	// ボタンを初期化
	InitializeButtons();

	// リザルトBGMを再生
	shared_.PlayExclusive_(BgmKind::Result);
}

ResultScene::~ResultScene() {
	delete drawCompBackground_;
	delete drawCompClearLabel_;
}

void ResultScene::InitializeDrawComponents() {
	// ========== 背景（アニメーション） ==========
	int bgTexture = Novice::LoadTexture("./Resources/images/result/result_sky.png");

	// 4x4分割、16フレームのアニメーション
	drawCompBackground_ = new DrawComponent2D(bgTexture, 4, 4, 16, 0.13f, true);

	// 背景の設定
	drawCompBackground_->SetPosition({ 400.0f, 400.0f });
	drawCompBackground_->SetDrawSize(800.0f, 800.0f);
	drawCompBackground_->SetAnchorPoint({ 0.5f, 0.5f });

	// オプション: 背景に微妙なエフェクトを追加
	// drawCompBackground_->StartPulse(0.98f, 1.02f, 0.8f, true);

	// ========== クリアラベル ==========
	int clearTexture = Novice::LoadTexture("./Resources/images/result/clear.png");

	// 静止画として作成
	drawCompClearLabel_ = new DrawComponent2D(clearTexture);

	// クリアラベルの設定
	drawCompClearLabel_->SetPosition({ 400.0f, 360.0f });
	drawCompClearLabel_->SetDrawSize(397.0f, 251.0f);
	drawCompClearLabel_->SetAnchorPoint({ 0.5f, 0.5f });

	// パルスエフェクト（拡大縮小）を開始
	drawCompClearLabel_->StartPulse(0.95f, 1.05f, 0.2f, true);

	// オプション: 出現エフェクト
	// drawCompClearLabel_->StartSpawnEffect();
}

void ResultScene::InitializeButtons() {
	// ボタン用の白いテクスチャ
	grHandleButton_ = shared_.texWhite;

	// ボタンの位置とサイズ
	const float centerX = 1080.0f;
	const float startY = 500.0f;
	const float buttonSpacing = 80.0f;
	const Vector2 buttonSize = { 270.0f, 60.0f };

	// ========== ボタンのコールバック ==========

	// リトライ
	auto retry = [&]() {
		shared_.StopAllBgm();
		manager_.RequestStageRestart();
		};

	// タイトルへ
	auto backToTitle = [&]() {
		shared_.StopAllBgm();
		manager_.RequestTransition(SceneType::Title);
		};

	// ゲーム終了
	auto quit = [&]() {
		shared_.StopAllBgm();
		manager_.RequestQuit();
		};

	// ========== ボタンを追加 ==========

	buttonManager_.AddButton({ centerX, startY }, buttonSize, "RETRY", retry);
	buttonManager_.AddButton({ centerX, startY + buttonSpacing }, buttonSize, "TITLE", backToTitle);
	buttonManager_.AddButton({ centerX, startY + buttonSpacing * 2 }, buttonSize, "QUIT", quit);

	// ========== SE設定 ==========

	buttonManager_.SetOnSelectSound([&]() {
		shared_.PlaySelectSe();
		});

	buttonManager_.SetOnDecideSound([&]() {
		shared_.PlayDecideSe();
		});
}

void ResultScene::UpdateDrawComponents(float deltaTime) {
	// 背景を更新（アニメーション）
	if (drawCompBackground_) {
		drawCompBackground_->Update(deltaTime);
	}

	// クリアラベルを更新（パルスエフェクト）
	if (drawCompClearLabel_) {
		drawCompClearLabel_->Update(deltaTime);
	}
}

void ResultScene::Update(float dt, const char* keys, const char* pre) {
	shared_.pad.Update();

	// 描画コンポーネントを更新
	UpdateDrawComponents(dt);

	// ボタンマネージャーを更新
	buttonManager_.Update(dt, keys, pre, shared_.pad);
}

void ResultScene::Draw() {
	// ========== 背景描画（アニメーション） ==========
	if (drawCompBackground_) {
		drawCompBackground_->DrawScreen();
	}

	// ========== クリアラベル描画（パルスエフェクト） ==========
	if (drawCompClearLabel_) {
		drawCompClearLabel_->DrawScreen();
	}

	// ========== ボタン描画 ==========
	if (fontReady_) {
		buttonManager_.Draw(grHandleButton_, &font_, &text_);
	}

	// ========== デバッグ情報 ==========
#ifdef _DEBUG
	ImGui::Begin("Result Scene Debug");

	ImGui::Text("=== Scene State ===");
	ImGui::Text("Font Ready: %s", fontReady_ ? "Yes" : "No");

	ImGui::Separator();

	ImGui::Text("=== Background ===");
	if (drawCompBackground_) {
		Vector2 bgPos = drawCompBackground_->GetPosition();
		Vector2 bgSize = drawCompBackground_->GetDrawSize();
		ImGui::Text("Position: (%.1f, %.1f)", bgPos.x, bgPos.y);
		ImGui::Text("Draw Size: (%.1f, %.1f)", bgSize.x, bgSize.y);
		ImGui::Text("Animation Playing: %s",
			drawCompBackground_->IsAnimationPlaying() ? "Yes" : "No");

		if (ImGui::Button("Add Background Pulse")) {
			drawCompBackground_->StartPulse(0.98f, 1.02f, 0.8f, true);
		}

		if (ImGui::Button("Stop Background Effects")) {
			drawCompBackground_->StopAllEffects();
		}
	}

	ImGui::Separator();

	ImGui::Text("=== Clear Label ===");
	if (drawCompClearLabel_) {
		Vector2 labelPos = drawCompClearLabel_->GetPosition();
		Vector2 labelSize = drawCompClearLabel_->GetDrawSize();
		ImGui::Text("Position: (%.1f, %.1f)", labelPos.x, labelPos.y);
		ImGui::Text("Draw Size: (%.1f, %.1f)", labelSize.x, labelSize.y);
		ImGui::Text("Scale Effect Active: %s",
			drawCompClearLabel_->IsScaleEffectActive() ? "Yes" : "No");

		if (ImGui::Button("Reset Label Pulse")) {
			drawCompClearLabel_->StopScale();
			drawCompClearLabel_->StartPulse(0.95f, 1.05f, 0.2f, true);
		}

		if (ImGui::Button("Flash Label")) {
			drawCompClearLabel_->StartFlash(ColorRGBA::White(), 0.3f, 0.8f);
		}

		if (ImGui::Button("Spawn Effect")) {
			drawCompClearLabel_->StartSpawnEffect();
		}
	}

	ImGui::Separator();

	ImGui::Text("=== Buttons ===");
	ImGui::Text("Button Count: %zu", buttonManager_.GetButtonCount());
	ImGui::Text("Selected Index: %d", buttonManager_.GetSelectedIndex());

	ImGui::Separator();

	ImGui::Text("=== Controls ===");
	ImGui::Text("W/S or Up/Down: Navigate");
	ImGui::Text("Space/Enter or A: Select");

	ImGui::End();
#endif
}