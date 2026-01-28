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
	DrawComponent2D drawCompLogo_;
};
