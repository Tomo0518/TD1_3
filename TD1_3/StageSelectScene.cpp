#include "StageSelectScene.h"
#include "SceneManager.h"
#include "SceneType.h"
#include <Novice.h>

#include "SceneUtilityIncludes.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

//StageSelectScene::StageSelectScene(SceneManager& manager, GameShared& shared)
//	: manager_(manager), shared_(shared) {
//
//	// フォント読み込み
//	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
//		text_.SetFont(&font_);
//		fontReady_ = true;
//	}
//
//	// 描画コンポーネント初期化
//	InitializeDrawComponents();
//
//	// ボタン初期化
//	InitializeButtons();
//
//	// BGMを再生
//	shared_.PlayExclusive_(BgmKind::Title);
//
//	// 入力遅延タイマーを設定
//	inputDelayTimer_ = kInputDelay_;
//	inputEnabled_ = false;
//}

StageSelectScene::StageSelectScene(SceneManager& manager)
	: manager_(manager){

	// フォント読み込み
	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}

	// 描画コンポーネント初期化
	InitializeDrawComponents();

	// ボタン初期化
	InitializeButtons();

	// BGMを再生
	SoundManager::GetInstance().PlayBgm(BgmId::None);

	// 入力遅延タイマーを設定
	inputDelayTimer_ = kInputDelay_;
	inputEnabled_ = false;
}

StageSelectScene::~StageSelectScene() {
	delete drawCompBackground_;
}

void StageSelectScene::InitializeDrawComponents() {
	// 背景テクスチャをロード
	int bgTexture = Novice::LoadTexture("./Resources/images/stageSelect/background_ver1.png");

	// 新しい DrawComponent2D で背景を作成
	drawCompBackground_ = new DrawComponent2D(bgTexture);

	// 背景の設定
	drawCompBackground_->SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
	drawCompBackground_->SetDrawSize(1280.0f, 720.0f);
	drawCompBackground_->SetAnchorPoint({ 0.5f, 0.5f });

	// オプション: 背景にエフェクトを追加
	// drawCompBackground_->StartPulse(0.98f, 1.02f, 0.3f, true);
}

void StageSelectScene::InitializeButtons() {
	// ボタン用の白いテクスチャ
	grHandleButton_ = TextureManager::GetInstance().GetTexture(TextureId::White1x1);

	// ボタンの位置とサイズ
	const float centerX = 640.0f;
	const float startY = 250.0f;
	const float buttonSpacing = 100.0f;
	const Vector2 buttonSize = { 400.0f, 80.0f };

	// ========== ボタンのコールバック ==========

	// ステージ1へ遷移
	auto goToStage1 = [&]() {
		//shared_.lastPlayedStageIndex = 0;
		manager_.RequestStage(1);
		};

	// ゲームプレイシーン（テスト用）
	auto goToGamePlay = [&]() {
		//shared_.lastPlayedStageIndex = 0;
		manager_.RequestTransition(SceneType::GamePlay);
		};

	// タイトルに戻る
	auto backToTitle = [&]() {
		SoundManager::GetInstance().PlaySe(SeId::Decide);
		manager_.RequestTransition(SceneType::Title);
		};

	// ========== ボタンを追加 ==========

	buttonManager_.AddButton(
		Vector2{ centerX, startY },
		buttonSize,
		"STAGE 1",
		goToStage1
	);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing },
		buttonSize,
		"GAMEPLAY TEST",
		goToGamePlay
	);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing * 2 },
		buttonSize,
		"BACK TO TITLE",
		backToTitle
	);

	// ========== SE設定 ==========

	buttonManager_.SetOnSelectSound([&]() {
		SoundManager::GetInstance().PlaySe(SeId::Select);
		});

	buttonManager_.SetOnDecideSound([&]() {
		SoundManager::GetInstance().PlaySe(SeId::Decide);
		});

	// 初期選択をリセット
	buttonManager_.SetFirstFrame(true);
}

void StageSelectScene::UpdateDrawComponents(float deltaTime) {
	if (drawCompBackground_) {
		drawCompBackground_->Update(deltaTime);
	}
}

void StageSelectScene::Update(float dt, const char* keys, const char* pre) {
	// 入力受付の遅延処理（秒単位）
	if (inputDelayTimer_ > 0.0f) {
		inputDelayTimer_ -= dt;
		if (inputDelayTimer_ <= 0.0f) {
			inputEnabled_ = true;
		}
	}

	// 描画コンポーネントを更新
	UpdateDrawComponents(dt);

	// 入力が有効な場合のみ処理
	if (inputEnabled_) {
		// ボタンマネージャーの更新
		buttonManager_.Update(dt, keys, pre, *InputManager::GetInstance().GetPad());

		// Escapeキーで戻る
		if (!pre[DIK_ESCAPE] && keys[DIK_ESCAPE]) {
			SoundManager::GetInstance().PlaySe(SeId::Back);
			manager_.RequestTransition(SceneType::Title);
		}
	}
}

void StageSelectScene::Draw() {
	// ========== 背景描画 ==========
	if (drawCompBackground_) {
		drawCompBackground_->DrawScreen();
	}

	// ========== ボタン描画 ==========
	if (fontReady_) {
		buttonManager_.Draw(grHandleButton_, &font_, &text_);
	}

	// ========== デバッグ情報 ==========
#ifdef _DEBUG
	ImGui::Begin("Stage Select Debug");

	ImGui::Text("=== Scene State ===");
	ImGui::Text("Input Enabled: %s", inputEnabled_ ? "Yes" : "No");
	ImGui::Text("Input Delay Timer: %.2f", inputDelayTimer_);

	ImGui::Separator();

	ImGui::Text("=== Background ===");
	if (drawCompBackground_) {
		Vector2 bgPos = drawCompBackground_->GetPosition();
		Vector2 bgSize = drawCompBackground_->GetDrawSize();
		ImGui::Text("Position: (%.1f, %.1f)", bgPos.x, bgPos.y);
		ImGui::Text("Size: (%.1f, %.1f)", bgSize.x, bgSize.y);

		if (ImGui::Button("Add Pulse Effect")) {
			drawCompBackground_->StartPulse(0.98f, 1.02f, 0.5f, true);
		}

		if (ImGui::Button("Stop All Effects")) {
			drawCompBackground_->StopAllEffects();
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
	ImGui::Text("ESC or Back: Return to Title");

	ImGui::End();
#endif
}