#pragma once

#include "GameShared.h"
#include "TextRenderer.h"
#include "GameSceneBase.h"
#include "DrawComponent2D.h"
#include "ButtonManager.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

class SceneManager;

/// <summary>
/// リザルトシーン
/// 新しい DrawComponent2D と Effect システムに対応
/// </summary>
class ResultScene : public GameSceneBase {
public:
	ResultScene(SceneManager& mgr, GameShared& shared);
	~ResultScene();

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

private:
	void InitializeButtons();
	void InitializeDrawComponents();
	void UpdateDrawComponents(float deltaTime);

	SceneManager& manager_;
	GameShared& shared_;

	// ========== ボタン管理 ==========
	ButtonManager buttonManager_;
	int grHandleButton_ = 0;

	// ========== フォント ==========
	bool fontReady_ = false;
	FontAtlas font_;
	TextRenderer text_;

	// ========== 描画コンポーネント ==========
	DrawComponent2D* drawCompBackground_ = nullptr;      // 背景（アニメーション）
	DrawComponent2D* drawCompClearLabel_ = nullptr;      // クリアラベル（パルスエフェクト）
};