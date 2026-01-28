#include "TitleScene.h"
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
#include "TextureManager.h"

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
	drawCompBackground_ = DrawComponent2D(TextureId::TitleBackground);

	// 背景の設定
	drawCompBackground_.SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
	drawCompBackground_.SetDrawSize(1280.0f, 720.0f);
	drawCompBackground_.SetAnchorPoint({ 0.5f, 0.5f });

	// ========== ロゴ ==========
	drawCompLogo_ = DrawComponent2D(TextureId::TitleLogo);

	// ロゴの設定
	drawCompLogo_.SetPosition({ kWindowWidth / 2.0f, kWindowHeight / 2.0f });
	drawCompLogo_.SetAnchorPoint({ 0.5f, 0.5f });

	// ロゴにパルスエフェクトを追加（拡大縮小）
	drawCompLogo_.StartPulse(0.9f, 1.1f, 7.5f, true);
}

void TitleScene::InitializeButtons() {
	// ボタンの位置とサイズ
	const float centerX = 1130.0f;
	const float startY = 400.0f;
	const float buttonSpacing = 100.0f;
	const Vector2 buttonSize = { 250.0f, 76.0f };

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

	buttonManager_.AddButton(
		{ centerX, startY }, buttonSize, 
		Tex().GetTexture(TextureId::UI_Button_Play),
		Tex().GetTexture(TextureId::UI_Button_Play_Selected),
		goToGamePlay);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing *1},
		buttonSize,
		Tex().GetTexture(TextureId::UI_Button_Settings),
		Tex().GetTexture(TextureId::UI_Button_Settings_Selected),
		goToSettings
	);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing * 2 },
		buttonSize,
		Tex().GetTexture(TextureId::UI_Button_Quit),
		Tex().GetTexture(TextureId::UI_Button_Quit_Selected),
		quitGame
	);

	// SE設定
	buttonManager_.SetOnSelectSound([&]() {
		SoundManager::GetInstance().PlaySe(SeId::Select);
		});

	buttonManager_.SetOnDecideSound([&]() {
		SoundManager::GetInstance().PlaySe(SeId::Decide);
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

	Camera2D::GetInstance().DebugMove();
	Camera2D::GetInstance().Update(dt);

	// 描画コンポーネントを更新
	UpdateDrawComponents(dt);

	if (keys[DIK_I] && !pre[DIK_I]) {
		SoundManager::GetInstance().PlaySe(SeId::Decide);
	}

	player_.Update(dt);
	
	// ボタンマネージャーを更新
	buttonManager_.Update(dt);
}

void TitleScene::Draw() {
	// 背景描画（スクリーン座標）
	drawCompBackground_.DrawScreen();

	// ロゴ描画（スクリーン座標、パルスエフェクト付き）
	drawCompLogo_.DrawScreen();

	// ボタン描画
	buttonManager_.Draw();

	player_.Draw(Camera2D::GetInstance());
}