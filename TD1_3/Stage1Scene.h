#pragma once
#include "BaseStageScene.h"

/// <summary>
/// ステージ1のシーン
/// </summary>
class Stage1Scene : public BaseStageScene {
public:
	Stage1Scene(SceneManager& manager);

protected:

	// ステージ固有の初期化（オプション）
	void InitializeStage() override;

	// ステージ固有の更新処理（オプション）
	void UpdateStage(float dt, const char* keys, const char* pre) override;

	// ステージ固有の描画処理（オプション）
	void DrawStage() override;
};