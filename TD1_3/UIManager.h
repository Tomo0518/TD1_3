#pragma once
#include <Novice.h>
#include <memory>
#include <vector>
#include "DrawComponent2D.h"
#include "InputManager.h"
#include "TextureManager.h"
#include <string>

struct PlayerSkillState {
	bool isDashing = false;
	bool canDash = false;

	enum class BoomerangMode {
		Idle,       // 待機中
		Throwing,   // 投げる準備/投げられる
		Recalling   // 回収できる状態
	};
	BoomerangMode boomerangMode = BoomerangMode::Idle;
	bool canUseBoomerang = false;
};


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


	/// <summary>
	/// スキルの仕様に伴いリアクションを与えるアイコン管理クラス
	/// </summary>
	class SkillIcon {
	public:
		SkillIcon(const Vector2& pos) {
			iconPos_ = pos;
		}


		void Update(float deltaTime, bool useSkill, bool canUseSkill,bool isWaiting = false) {
			// アイコンの更新
			for (auto& iconComp : icon_.first) {
				iconComp->Update(deltaTime);
			}

			// 使用不可状態なら半透明にする
			if (!canUseSkill || isWaiting) {
				for (auto& iconComp : icon_.first) {
					iconComp->SetBaseColor(0x4e76af77); // 半透明
				}
			}
			else {
				for (auto& iconComp : icon_.first) {
					
					iconComp->SetBaseColor(0xFFFFFFFF); // 不透明
				}
			}

			// 状態変化を検出（false→true または true→false）
			if (useSkill != prevUseSkill_) {
				// アイコンを次に切り替え
				icon_.second = (icon_.second + 1) % std::max(1, (int)icon_.first.size());

				// アイコンにリアクションを与える
				if (icon_.second < icon_.first.size()) {
					icon_.first[icon_.second]->StartSquash({ 1.2f, 0.7f }, 0.2f);
				}
			}

			// 前フレームの状態を保存
			prevUseSkill_ = useSkill;
		}

		void Draw() {
			if (!icon_.first.empty()) {
				// 現在のアイコンを描画
				auto& iconComp = icon_.first[icon_.second];
				iconComp->SetPosition(iconPos_);
				iconComp->DrawScreen();
			}
		}

		void AddIconTexture(TextureId textureId, int iconId = 0) {
			auto iconComp = std::make_unique<DrawComponent2D>(textureId, 1, 1, 1, 0.0f);
			icon_.first.push_back(std::move(iconComp));
			icon_.second = iconId;
		}

	private:
		std::pair<std::vector<std::unique_ptr<DrawComponent2D>>,int> icon_;   // アイコンの配列(二形態ある場合,表示するかどうか)
		Vector2 iconPos_;    // アイコンの位置
		bool prevUseSkill_ = false;

		int currentIconReactionFrame_ = 0;
		const int maxIconReactionFrame_ = 20;
	};

public:
	// =================================================================
	// UIManager 本体
	// =================================================================
	static UIManager& GetInstance() {
		static UIManager instance;
		return instance;
	}

	// 初期化
	void Initialize();

	// 更新
	void Update(float dt = 1.0f);

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

	// アイコン更新・描画
	void UpdateIcons(float dt, const PlayerSkillState& state);
	void DrawIcons();

private:
	UIManager() = default;
	~UIManager() = default;
	UIManager(const UIManager&) = delete;
	UIManager& operator=(const UIManager&) = delete;

	// --- UIパーツ ---
	std::unique_ptr<Gauge> playerHP_;
	std::unique_ptr<Gauge> bossHP_;

	// ========== Icon ================
	std::unique_ptr<DrawComponent2D> iconJump_;
	std::unique_ptr<DrawComponent2D> iconDash_;
	std::unique_ptr<DrawComponent2D> iconDashEmpty_;
	std::unique_ptr<DrawComponent2D> iconBoomerangThrow_;
	std::unique_ptr<DrawComponent2D> iconBoomerangReturn_;

	std::unique_ptr<SkillIcon> skillIconDash_;
	std::unique_ptr<SkillIcon> skillIconBoomerang_;
	Vector2 skillIconPos_ = { 1050.0f,580.0f };
	Vector2 skillIconOffset_ = { 120.0f,0.0f };
	Vector2 controllKeyOffset_{0.0f,80.0f};

	// ================================

	// キーガイド（入力連動用）
	std::unique_ptr<DrawComponent2D> keyW_;
	std::unique_ptr<DrawComponent2D> keyA_;
	std::unique_ptr<DrawComponent2D> keyS_;
	std::unique_ptr<DrawComponent2D> keyD_;
	std::unique_ptr<DrawComponent2D> keyK_;
	std::unique_ptr<DrawComponent2D> keyJ_;
	

	// Jump, Dash, Throwのキーガイド
	std::unique_ptr<DrawComponent2D> keyJump_;
	std::unique_ptr<DrawComponent2D> keyDash_;
	std::unique_ptr<DrawComponent2D> keyBoomerangReturn_;
	std::unique_ptr<DrawComponent2D> keyBoomerangThrow_;

	

	// ========= Padキーガイド =========
	std::unique_ptr<DrawComponent2D> padStickUp_;
	std::unique_ptr<DrawComponent2D> padStickDown_;
	std::unique_ptr<DrawComponent2D> padStickLeft_;
	std::unique_ptr<DrawComponent2D> padStickRight_;

	std::unique_ptr<DrawComponent2D> padButtonA_;
	std::unique_ptr<DrawComponent2D> padButtonB_;

	std::unique_ptr<DrawComponent2D> padButtonJump_;
	std::unique_ptr<DrawComponent2D> padButtonDash_;
	std::unique_ptr<DrawComponent2D> padButtonBoomerang_;

	// ポーズ画面
	std::unique_ptr<DrawComponent2D> pauseBg_;   // 半透明黒
	std::unique_ptr<DrawComponent2D> pauseText_; // "PAUSE"

	// リザルト画面
	std::unique_ptr<DrawComponent2D> resultImage_;

	// --- 状態管理 ---
	bool isTitle_ = false;
	bool isGamePlay_ = false;
	bool isPaused_ = false;
	bool isResult_ = false;
	bool isGameClear_ = false;

	// ヘルパー: キーガイドの更新
	void UpdateKeyGuides();
};