#include "TitleScene.h"
#include "GameShared.h"
#include "Easing.h"
#include <Novice.h>
#include <cstring>
#include <algorithm>

#include "SceneManager.h"
#include "SceneUtilityIncludes.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

#include "Camera2D.h"

//TitleScene::TitleScene(SceneManager& manager, GameShared& shared)
//	: manager_(manager),
//	shared_(shared) {
//
//	// フォント読み込み
//	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
//		text_.SetFont(&font_);
//		fontReady_ = true;
//	}
//
//	// 描画コンポーネントを初期化
//	InitializeDrawComponents();
//
//	// ボタン初期化
//	InitializeButtons();
//
//	// BGMを再生
//	shared_.PlayExclusive_(BgmKind::Title);
//}

TitleScene::TitleScene(SceneManager& manager)
	: sceneManager_(manager){

	// フォント読み込み
	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}

	// 描画コンポーネントを初期化
	InitializeDrawComponents();

	// ボタン初期化
	InitializeButtons();

	player_.Initialize();

	// BGMを再生
	SoundManager::GetInstance().PlayBgm(BgmId::Title);
}

void TitleScene::InitializeDrawComponents() {
	// ========== 背景 ==========
	int bgTexture = Novice::LoadTexture("./Resources/images/title/background_ver1.png");
	drawCompBackground_ = DrawComponent2D(bgTexture);

	// 背景の設定
	drawCompBackground_.SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
	drawCompBackground_.SetDrawSize(1280.0f, 720.0f);
	drawCompBackground_.SetAnchorPoint({ 0.5f, 0.5f });

	// ========== ロゴ ==========
	int logoTexture = Novice::LoadTexture("./Resources/images/title/logo_ver1.png");
	drawCompLogo_ = DrawComponent2D(logoTexture);

	// ロゴの設定
	drawCompLogo_.SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
	drawCompLogo_.SetAnchorPoint({ 0.5f, 0.5f });

	// ロゴにパルスエフェクトを追加（拡大縮小）
	drawCompLogo_.StartPulse(0.9f, 1.1f, 7.5f, true);

	// テクスチャハンドルを保存（デバッグ用）
	grHandleBackground_ = bgTexture;
	grHandleLogo_ = logoTexture;
}

void TitleScene::InitializeButtons() {
	// ボタン用の白いテクスチャ
	//grHandleButton_ = shared_.texWhite;

	// ボタンの位置とサイズ
	const float centerX = 1080.0f;
	const float startY = 400.0f;
	const float buttonSpacing = 80.0f;
	const Vector2 buttonSize = { 270.0f, 60.0f };

	// ボタンのコールバック
	auto goToStageSelect = [&]() {
		sceneManager_.RequestTransition(SceneType::StageSelect);
		};

	auto goToGamePlay = [&]() {
		sceneManager_.RequestTransition(SceneType::GamePlay);
		};

	auto goToSettings = [&]() {
		sceneManager_.RequestOpenSettings();
		};

	auto quitGame = [&]() {
		sceneManager_.RequestQuit();
		};

	// 3つのボタンを追加

	buttonManager_.AddButton({ centerX, startY }, buttonSize, "GamePlay", goToGamePlay);

	buttonManager_.AddButton({ centerX, startY + buttonSpacing }, buttonSize, "StageSelect", goToStageSelect);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing *2},
		buttonSize,
		"SETTING",
		goToSettings
	);
	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing * 3 },
		buttonSize,
		"QUIT",
		quitGame
	);

	// 各ボタンにテクスチャを設定
	for (size_t i = 0; i < buttonManager_.GetButtonCount(); ++i) {
		buttonManager_.SetButtonTexture(grHandleButton_);
	}

	// SE設定
	buttonManager_.SetOnSelectSound([&]() {
		SoundManager::GetInstance().PlaySe(SeId::PlayerShot);
		});

	buttonManager_.SetOnDecideSound([&]() {
		SoundManager::GetInstance().PlaySe(SeId::PlayerShot);
		});

	// 初期選択をリセット
	buttonManager_.SetFirstFrame(true);
}

void TitleScene::UpdateDrawComponents(float deltaTime) {
	// 新しい DrawComponent2D の Update() を使用
	drawCompBackground_.Update(deltaTime);
	drawCompLogo_.Update(deltaTime);
}

void TitleScene::Update(float dt, const char* keys, const char* pre) {

	Camera2D::GetInstance().DebugMove(true, keys, pre);
	Camera2D::GetInstance().Update(dt);

	// 描画コンポーネントを更新
	UpdateDrawComponents(dt);

	if (keys[DIK_I] && !pre[DIK_I]) {
		SoundManager::GetInstance().PlaySe(SeId::Decide);
	}

	player_.Update(dt);
	
	// ボタンマネージャーを更新
	buttonManager_.Update(dt, keys, pre, *InputManager::GetInstance().GetPad());
}

void TitleScene::Draw() {
	// 背景描画（スクリーン座標）
	drawCompBackground_.DrawScreen();

	// ロゴ描画（スクリーン座標、パルスエフェクト付き）
	drawCompLogo_.DrawScreen();

	// ボタン描画
	buttonManager_.Draw(
		grHandleButton_,
		&font_,
		&text_
	);

	player_.Draw(Camera2D::GetInstance());

#ifdef _DEBUG
	// デバッグ情報
	ImGui::Begin("Title Scene Debug");

	ImGui::Text("=== Background ===");
	Vector2 bgPos = drawCompBackground_.GetPosition();
	Vector2 bgSize = drawCompBackground_.GetDrawSize();
	ImGui::Text("Position: (%.1f, %.1f)", bgPos.x, bgPos.y);
	ImGui::Text("Draw Size: (%.1f, %.1f)", bgSize.x, bgSize.y);

	ImGui::Separator();

	ImGui::Text("=== Logo ===");
	Vector2 logoPos = drawCompLogo_.GetPosition();
	Vector2 logoSize = drawCompLogo_.GetDrawSize();
	Vector2 logoImageSize = drawCompLogo_.GetImageSize();
	ImGui::Text("Position: (%.1f, %.1f)", logoPos.x, logoPos.y);
	ImGui::Text("Image Size: (%.1f, %.1f)", logoImageSize.x, logoImageSize.y);
	ImGui::Text("Draw Size: (%.1f, %.1f)", logoSize.x, logoSize.y);
	ImGui::Text("Scale Effect Active: %s",
		drawCompLogo_.IsScaleEffectActive() ? "Yes" : "No");

	ImGui::Separator();

	if (ImGui::Button("Reset Logo Pulse")) {
		drawCompLogo_.StopScale();
		drawCompLogo_.StartPulse(0.9f, 1.1f, 0.5f, true);
	}

	if (ImGui::Button("Stop All Effects")) {
		drawCompLogo_.StopAllEffects();
	}

	ImGui::End();
#endif
}