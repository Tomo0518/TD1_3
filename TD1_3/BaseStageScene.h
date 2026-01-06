#pragma once
#include <memory>
#include "GameSceneBase.h"
#include "GameShared.h"
#include "DrawComponent2D.h"
#include "Camera2D.h"

class SceneManager;

/// <summary>
/// ステージシーンの基底クラス
/// 新しい DrawComponent2D と Camera2D システムに対応
/// - 背景描画
/// - カメラ管理
/// - 共通の更新処理
/// </summary>
class BaseStageScene : public GameSceneBase {
public:
	BaseStageScene(SceneManager& manager, GameShared& shared, int stageIndex);
	virtual ~BaseStageScene();

	int GetStageIndex() const override { return stageIndex_; }

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

protected:
	// ========================================
	// 派生クラスでオーバーライド可能なメソッド
	// ========================================

	/// <summary>
	/// ステージ固有の初期化処理
	/// </summary>
	virtual void InitializeStage() {}

	/// <summary>
	/// ステージ固有の更新処理
	/// </summary>
	virtual void UpdateStage(float dt, const char* keys, const char* pre) {
		(void)dt; (void)keys; (void)pre;
	}

	/// <summary>
	/// ステージ固有の描画処理
	/// </summary>
	virtual void DrawStage() {}

	// ========================================
	// 共通メンバ変数（派生クラスからアクセス可能）
	// ========================================

	SceneManager& manager_;          // シーンマネージャー
	GameShared& shared_;             // 共有リソース
	int stageIndex_ = 0;             // ステージ番号
	bool initialized_ = false;       // 初期化フラグ

	// カメラ（派生クラスで使用可能）
	Camera2D* camera_ = nullptr;

private:
	// ========================================
	// 内部処理
	// ========================================

	void InitializeBackground();
	void InitializeCamera();
	void UpdateBackground(float deltaTime);

	// ========================================
	// 描画コンポーネント
	// ========================================

	DrawComponent2D* drawCompBackground_ = nullptr;  // 背景
};