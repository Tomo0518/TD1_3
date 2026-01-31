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
#include "UIManager.h"

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

	// ロゴパーツの初期化
	drawCompLogo_Lu_ = DrawComponent2D(TextureId::Logo_Lu);
	drawCompLogo_U_ = DrawComponent2D(TextureId::Logo_U);
	drawCompLogo_Na_ = DrawComponent2D(TextureId::Logo_Na);
	drawCompLogo_Ra_ = DrawComponent2D(TextureId::Logo_Ra);
	drawCompLogo_Nn_ = DrawComponent2D(TextureId::Logo_Nn);

	// ロゴパーツの設定
	drawCompLogo_Lu_.SetAnchorPoint({ 0.5f, 0.5f });
	drawCompLogo_U_.SetAnchorPoint({ 0.5f, 0.5f });
	drawCompLogo_Na_.SetAnchorPoint({ 0.5f, 0.5f });
	drawCompLogo_Ra_.SetAnchorPoint({ 0.5f, 0.5f });
	drawCompLogo_Nn_.SetAnchorPoint({ 0.5f, 0.5f });
}

void TitleScene::InitializeButtons() {
	// ボタンの位置とサイズ
	const float centerX = 1130.0f;
	const float startY = 500.0f;
	const float buttonSpacing = 80.0f;
	const Vector2 buttonSize = { 250.0f * 0.95f, 76.0f  * 0.95f};

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

	drawCompLogo_Lu_.Update(deltaTime);
	drawCompLogo_U_.Update(deltaTime);
	drawCompLogo_Na_.Update(deltaTime);
	drawCompLogo_Ra_.Update(deltaTime);
	drawCompLogo_Nn_.Update(deltaTime);
}

void TitleScene::Update(float dt, const char* keys, const char* pre) {

	UIManager::GetInstance().UpdateTitleControlUI(dt);

	Camera2D::GetInstance().DebugMove();
	Camera2D::GetInstance().Update(dt);

	// ロゴアニメーション更新
	UpdateLogoAnimation();

	// 描画コンポーネントを更新
	UpdateDrawComponents(dt);

	if (keys[DIK_I] && !pre[DIK_I]) {
		SoundManager::GetInstance().PlaySe(SeId::Decide);
	}

	// ボタンマネージャーを更新
	buttonManager_.Update(dt);

	// ImGuiデバッグウィンドウ
#ifdef _DEBUG
	ImGui::Begin("Title Logo Animation");
	ImGui::SliderFloat("LogoDrawOffset", &logoPos_Nn.x, 0.0f, 300.0f);
	ImGui::SliderFloat("squashDuration", &squashDuration_, 0.0f, 2.0f);
	ImGui::SliderFloat2("SquashScale", &logoSquashScale_.x, 0.0f, 2.0f);
	if (ImGui::TreeNode("DrawOffset")) {

		ImGui::SliderFloat2("DrawOffset_Lu", &logoDrawOffset_Lu_.x,-400.0f, 400.0f);
		ImGui::SliderFloat2("DrawOffset_U", &logoDrawOffset_U_.x, -400.0f, 400.0f);
		ImGui::SliderFloat2("DrawOffset_Ra", &logoDrawOffset_Ra_.x, -400.0f, 400.0f);
		ImGui::SliderFloat2("DrawOffset_Nn", &logoDrawOffset_Nn_.x, -400.0f, 400.0f);
	}
	ImGui::SliderInt("allFrameMax", &allFrameMax_, 0, 400);
	ImGui::End();
#endif
}

void TitleScene::UpdateLogoAnimation() {
	frameCount_++;

	{
		squashLogo_Lu_Frame_ = allFrameMax_ / 6 * 1;
		squashLogo2_U_Frame_ = allFrameMax_ / 6 * 2;
		squashLogo3_Na_Frame_ = allFrameMax_ / 6 * 3;
		squashLogo4_Ra_Frame_ = allFrameMax_ / 6 * 4;
		squashLogo5_Nn_Frame_ = allFrameMax_ / 6 * 5;
		squashLogoAllFrame_ = allFrameMax_;
	}

	if (frameCount_ == squashLogo_Lu_Frame_) {
		drawCompLogo_Lu_.StartSquash(logoSquashScale_, squashDuration_);
	}
	else if (frameCount_ == squashLogo2_U_Frame_) {
		drawCompLogo_U_.StartSquash(logoSquashScale_, squashDuration_);
	}
	else if (frameCount_ ==  squashLogo3_Na_Frame_) {
		drawCompLogo_Na_.StartSquash(logoSquashScale_, squashDuration_);
	}
	else if (frameCount_ == squashLogo4_Ra_Frame_) {
		drawCompLogo_Ra_.StartSquash(logoSquashScale_, squashDuration_);
	}
	else if (frameCount_ == squashLogo5_Nn_Frame_) {
		drawCompLogo_Nn_.StartSquash(logoSquashScale_, squashDuration_);
	}
	else if (frameCount_ >= squashLogoAllFrame_) {
		// 全てのパーツをスクアッシュする
		drawCompLogo_Lu_.StartSquash(logoSquashScale_, squashDuration_);
		drawCompLogo_U_.StartSquash(logoSquashScale_, squashDuration_);
		drawCompLogo_Na_.StartSquash(logoSquashScale_, squashDuration_);
		drawCompLogo_Ra_.StartSquash(logoSquashScale_, squashDuration_);
		drawCompLogo_Nn_.StartSquash(logoSquashScale_, squashDuration_);

		frameCount_ = 0;
	}
	else {
		// 何もしない
	}

	// Naが中心で他はそれぞれオフセット
	drawCompLogo_Na_.SetPosition(logoPos_Nn);
	drawCompLogo_U_.SetPosition(logoPos_Nn + logoDrawOffset_U_);
	drawCompLogo_Lu_.SetPosition(logoPos_Nn + logoDrawOffset_Lu_);
	drawCompLogo_Ra_.SetPosition(logoPos_Nn + logoDrawOffset_Ra_);
	drawCompLogo_Nn_.SetPosition(logoPos_Nn + logoDrawOffset_Nn_);
}

void TitleScene::Draw() {
	// 背景描画（スクリーン座標）
	drawCompBackground_.DrawScreen();

	// ロゴ描画（スクリーン座標、パルスエフェクト付き）
	//drawCompLogo_.DrawScreen();

	// ロゴパーツ描画（スクリーン座標）

	drawCompLogo_Lu_.DrawScreen();
	drawCompLogo_U_.DrawScreen();
	drawCompLogo_Na_.DrawScreen();
	drawCompLogo_Ra_.DrawScreen();
	drawCompLogo_Nn_.DrawScreen();

	// ボタン描画
	buttonManager_.Draw();

	UIManager::GetInstance().DrawTitleControlUI();
}