#pragma once

#include "IGameScene.h"
#include "GameShared.h"
#include "Pad.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include "ButtonManager.h"
#include "DrawComponent2D.h"
#include "WindowSize.h"

class SceneManager;

/// <summary>
/// ステージセレクトシーン
/// </summary>
class StageSelectScene : public IScene {
public:
	StageSelectScene(SceneManager& manager);
	~StageSelectScene();

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

private:
	SceneManager& manager_;
	//GameShared& shared_;

	// ========== ボタン管理 ==========
	ButtonManager buttonManager_;
	void InitializeButtons();

	// ========== フォント ==========
	FontAtlas font_;
	TextRenderer text_;
	bool fontReady_ = false;

	// ========== 描画コンポーネント ==========
	void InitializeDrawComponents();
	void UpdateDrawComponents(float deltaTime);

	int grHandleButton_ = -1;
	DrawComponent2D* drawCompBackground_ = nullptr;

	// ========== 入力制御 ==========
	float inputDelayTimer_ = 0.0f;
	const float kInputDelay_ = 0.5f;  // 秒単位の遅延
	bool inputEnabled_ = false;
};