#pragma once
#include "IGameScene.h"
#include "Pad.h"
#include "TextRenderer.h"
#include "FontAtlas.h"
#include "ButtonManager.h"
#include "DrawComponent2D.h"
#include <memory>

class GameShared;
class SceneManager;

class TitleScene : public IScene {
public:
	TitleScene(SceneManager& manager);

	void Update(float deltaTime, const char* keys, const char* preKeys) override;
	void Draw() override;

private:
	SceneManager& sceneManager_;
//	GameShared& shared_;

	// Logoアニメーション用
	int frameCount_ = 0;
	int allFrameMax_ = 177;
	int squashLogo_Lu_Frame_ = allFrameMax_ / 6 * 1;
	int squashLogo2_U_Frame_ = allFrameMax_ / 6 * 2;
	int squashLogo3_Na_Frame_ = allFrameMax_ / 6 * 3;
	int squashLogo4_Ra_Frame_ = allFrameMax_ / 6 * 4;
	int squashLogo5_Nn_Frame_ = allFrameMax_ / 6 * 5;
	int squashLogoAllFrame_ = allFrameMax_;

	// ボタンマネージャー
	ButtonManager buttonManager_;
	void InitializeButtons(); // ボタン初期化

	// フォント
	FontAtlas font_;
	TextRenderer text_;
	bool fontReady_ = false;

	//=========================
	// 描画類
	//=========================

	// コンポーネントの初期化
	void InitializeDrawComponents();
	void UpdateDrawComponents(float deltaTime);

	// 背景テクスチャ
	DrawComponent2D drawCompBackground_;

	// ロゴテクスチャ
	void UpdateLogoAnimation();

	DrawComponent2D drawCompLogo_;

	DrawComponent2D drawCompLogo_Lu_;
	DrawComponent2D drawCompLogo_U_;
	DrawComponent2D drawCompLogo_Na_;
	DrawComponent2D drawCompLogo_Ra_;
	DrawComponent2D drawCompLogo_Nn_;

	Vector2 logoPos_Nn = { kWindowWidth / 2.0f, kWindowHeight / 2.0f - 50.0f };
	Vector2 logoDrawOffset_Lu_ = { -374.0f, 0.0f };
	Vector2 logoDrawOffset_U_ = { -154.0f, 0.0f };
	Vector2 logoDrawOffset_Ra_ = { 180.0f, 0.0f };
	Vector2 logoDrawOffset_Nn_ = { 348.f, 0.0f };
	Vector2 logoSquashScale_ = { 0.75f, 1.2f };

	float squashDuration_ = 0.4f;
};
