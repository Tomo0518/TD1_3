#include "ResultScene.h"
#include <Novice.h>
#include "SceneManager.h"
#include "SceneUtilityIncludes.h"

using namespace GameServices;

#ifdef _DEBUG
#include <imgui.h>
#endif

ResultScene::ResultScene(SceneManager& mgr)
	: manager_(mgr) {

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
	Sound().PlayBgm(BgmId::Result);

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
	// ボタンの位置とサイズ
	const float centerX = 1080.0f;
	const float startY = 500.0f;
	const float buttonSpacing = 80.0f;
	const Vector2 buttonSize = { 270.0f, 60.0f };

	// ========== ボタンのコールバック ==========

	// リトライ
	auto retry = [&]() {
		Sound().StopBgm();

		manager_.RequestTransition(SceneType::GamePlay);
		};

	// タイトルへ
	auto backToTitle = [&]() {
		Sound().StopBgm();
		manager_.RequestTransition(SceneType::Title);
		};

	// ゲーム終了
	auto quit = [&]() {
		Sound().StopBgm();
		manager_.RequestQuit();
		};

	// ========== ボタンを追加 ==========

	buttonManager_.AddButton({ centerX, startY }, buttonSize, "RETRY", retry);
	buttonManager_.AddButton({ centerX, startY + buttonSpacing }, buttonSize, "TITLE", backToTitle);
	buttonManager_.AddButton({ centerX, startY + buttonSpacing * 2 }, buttonSize, "QUIT", quit);

	// ========== SE設定 ==========

	buttonManager_.SetOnSelectSound([&]() {
		Sound().PlaySe(SeId::Select);
		});

	buttonManager_.SetOnDecideSound([&]() {
		Sound().PlaySe(SeId::Decide);
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
	keys; pre; // 未使用

	// 描画コンポーネントを更新
	UpdateDrawComponents(dt);

	// ボタンマネージャーを更新
	buttonManager_.Update(dt);
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
		buttonManager_.Draw();
	}
}