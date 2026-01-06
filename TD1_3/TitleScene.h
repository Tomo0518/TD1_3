#pragma once
#include "GameSceneBase.h"
#include "Pad.h"
#include "TextRenderer.h"
#include "FontAtlas.h"
#include "ButtonManager.h"
#include "DrawComponent2D.h"
#include <memory>

class GameShared;
class SceneManager;


class TitleScene : public GameSceneBase {
public:
	explicit TitleScene(SceneManager& manager, GameShared& shared);

	void Update(float deltaTime, const char* keys, const char* preKeys) override;
	void Draw() override;

	void SetButtonTexture(int textureHandle) { grHandleButton_ = textureHandle; }

private:
	SceneManager& manager_;
	GameShared& shared_;

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

	// grHandleやコンポーネントの初期化
	void InitializeDrawComponents();
	void UpdateDrawComponents(float deltaTime);

	// ボタン用テクスチャ
	int grHandleButton_ = -1;

	// 背景テクスチャ
	int grHandleBackground_ = -1;
	DrawComponent2D drawCompBackground_;

	// ロゴテクスチャ
	int grHandleLogo_ = -1;
	DrawComponent2D drawCompLogo_;
};
