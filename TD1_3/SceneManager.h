#pragma once
#include "IGameScene.h"
#include "SceneType.h"
#include <memory>
#include <optional>
#include <vector>

// シーン遷移情報を保持する構造体
struct SceneTransition {
	SceneType targetScene = SceneType::Title;
};

class SceneManager {
public:
	SceneManager();

	void Update(float dt, const char* keys, const char* pre);
	void Draw();

	// ゲーム終了判定
	bool ShouldQuit() const { return shouldQuit_; }

	// 現在のシーン情報取得
	SceneType GetCurrentSceneType() const { return currentSceneType_; }

	// シーン遷移リクエスト（各シーンから呼ばれる）
	void RequestTransition(SceneType targetScene);
	void RequestQuit() { shouldQuit_ = true; }

	// プレイをリトライ(ポーズのボタンから使用)
	void RequestRetry();

	// ポーズからタイトルへの処理
	void RequestPauseToTitle();

	// ======================
	// ポーズシステム
	// ======================
	void RequestOpenPause(); // ポーズ画面を開く
	void RequestClosePause(); // ポーズから復帰

	// ======================
	// 設定画面オーバーレイ
	// ======================
	void RequestOpenSettings(); // 設定画面を開く（オーバーレイ）
	void RequestCloseSettings(); // 設定画面を閉じる

	// オーバーレイシーン管理（設定画面など）
	void PushOverlay(std::unique_ptr<IScene> overlay);
	void PopOverlay();
	bool HasOverlay() const { return !overlayScenes_.empty(); }

private:
	// 現在のシーン
	std::unique_ptr<IScene> currentScene_;

	SceneType currentSceneType_ = SceneType::Title;

	// オーバーレイシーンスタック（設定画面など）
	std::vector<std::unique_ptr<IScene>> overlayScenes_;

	// 遷移リクエスト
	std::optional<SceneTransition> pendingTransition_;
	bool pendingOverlayClear_ = false;

	// ゲーム終了フラグ
	bool shouldQuit_ = false;

	// ステージ管理用
	int currentStageIndex_ = -1; // 現在プレイ中のステージ (-1 = なし)
	int pendingStageIndex_ = -1; // 遷移先ステージ番号 (RequestStage用)

	// リザルト用データ
	int resultScore_ = 0; // リザルト画面に渡すスコア

	// 説明画面の戻り先
	SceneType explanationReturnTo_ = SceneType::Title;

	// 内部処理
	void ProcessSceneTransition();
	SceneType StageIndexToSceneType(int stageIndex) const;


	void ChangeScene(SceneType type);
};