#pragma once
#include <Novice.h>
#include <memory>
#include <vector>
#include "DrawComponent2D.h"
#include "InputManager.h"
#include "TextureManager.h"

class UIManager {
private:
	// =================================================================
	// 内部クラス: Gauge
	// =================================================================
	class Gauge {
	public:
		// コンストラクタ
		Gauge(TextureId frameId, TextureId barId, const Vector2& offset = { 0.0f, 0.0f });

		// 更新処理（ゴースト追従、ピンチ演出）
		void Update(float deltaTime);

		// 描画処理
		void Draw(const Vector2& pos, float scale = 1.0f);

		// HP比率設定 (0.0f ~ 1.0f)
		void SetRatio(float ratio);

		// バーの色設定
		void SetColor(unsigned int color) { bar_.SetBaseColor(color); }

	private:
		DrawComponent2D frame_;    // 枠
		DrawComponent2D bar_;      // メインバー
		DrawComponent2D ghost_;    // ダメージ残像（リスクの可視化）

		Vector2 barOffset_;        // 枠に対するバーのズレ補正
		float currentRatio_ = 1.0f;
		float ghostRatio_ = 1.0f;
		float ghostTimer_ = 0.0f;  // ダメージを受けてから減り始めるまでのタメ
	};

public:
	// =================================================================
	// UIManager 本体
	// =================================================================
	static UIManager& Instance() {
		static UIManager instance;
		return instance;
	}

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// --- 外部操作インターフェース ---

	// プレイヤーHP更新
	void SetPlayerHP(float current, float max) {
		if (max > 0.0f) playerHP_->SetRatio(current / max);
	}

	// ボスHP更新
	void SetBossHP(float current, float max) {
		if (max > 0.0f) bossHP_->SetRatio(current / max);
	}

	// ゲーム状態設定
	void SetIsGamePlay(bool flag) { isGamePlay_ = flag; }
	void SetIsTitle(bool flag) { isTitle_ = flag; }

	// ポーズ制御
	bool IsPaused() const { return isPaused_; }
	void TogglePause();

	// リザルト表示
	void ShowResult(bool isClear);

private:
	UIManager() = default;
	~UIManager() = default;
	UIManager(const UIManager&) = delete;
	UIManager& operator=(const UIManager&) = delete;

	// --- UIパーツ ---
	std::unique_ptr<Gauge> playerHP_;
	std::unique_ptr<Gauge> bossHP_;

	// キーガイド（入力連動用）
	std::unique_ptr<DrawComponent2D> keyW_;
	std::unique_ptr<DrawComponent2D> keyA_;
	std::unique_ptr<DrawComponent2D> keyS_;
	std::unique_ptr<DrawComponent2D> keyD_;

	// ポーズ画面
	std::unique_ptr<DrawComponent2D> pauseBg_;   // 半透明黒
	std::unique_ptr<DrawComponent2D> pauseText_; // "PAUSE"

	// リザルト画面
	std::unique_ptr<DrawComponent2D> resultImage_;

	// --- 状態管理 ---
	bool isTitle_ = true;
	bool isGamePlay_ = false;
	bool isPaused_ = false;
	bool isResult_ = false;
	bool isGameClear_ = false;

	// ヘルパー: キーガイドの更新
	void UpdateKeyGuides();
};