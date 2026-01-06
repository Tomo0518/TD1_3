#pragma once
#include "Player.h"
#include "Camera2D.h"
#include "Pad.h"
#include <random>
#include <cstdint>
#include <Novice.h>
#include <array>
#include "stageMaxNum.h"
#include <memory>
#include "ParticleManager.h"

enum class BgmKind {
	None,
	Title,
	Stage,
	Result
};

enum class OperationMode {
	KeyboardMouse,
	Gamepad
};

// ========================================
// ステージデータ構造体
// ========================================
struct StageData {
	bool isCleared = false;        // クリアしたか
	int bestScore = 0;             // ベストスコア
	int playCount = 0;             // プレイ回数（オプション）

	StageData() = default;
};

class GameShared {
public:

	GameShared();

	int lastPlayedStageIndex = 0;
	int stageSelectCurrentIndex = 0;
	bool returnFromExplanation = false;

	// ========================================
	// ステージ進行データ
	// ========================================
	std::array<StageData, kMaxStageNum> stageData_;  // 全ステージのデータ

	// ステージデータ操作用メソッド
	void UpdateStageScore(int stageIndex, int score);  // スコア更新
	void SetStageCleared(int stageIndex, bool cleared);  // クリアフラグ設定
	bool IsStageCleared(int stageIndex) const;  // クリア済みか確認
	int GetBestScore(int stageIndex) const;  // ベストスコア取得
	bool IsNewRecord(int stageIndex, int score) const;  // 新記録か判定

	// セーブ・ロード用（将来的に実装）
	void SaveStageData();  // データ保存
	void LoadStageData();  // データ読み込み

	// 共有オブジェクト
	Pad    pad{ 0 };                // XInput 0番
	OperationMode operationMode = OperationMode::KeyboardMouse;

	void UpdateMousePosition();
	int GetMouseX() const { return mouseX; }
	int GetMouseY() const { return mouseY; }


	/*-----------------------------------------*/
	/* Texture ゲーム内のテクスチャ
	/*-----------------------------------------*/
	int texWhite = Novice::LoadTexture("./NoviceResources/white1x1.png");

	/* --- Setting Volume -- */
	float userBgmVolume = 0.0f; // 0.0f～1.0f ユーザー設定
	float userSeVolume = 0.1f;  // 0.0f～1.0f ユーザー設定
	bool isViblation = true;

	/* --- BGM --- */
	int bgmHandleTitle = 0;
	int bgmHandleStageSelect = 0;
	int bgmHandlePlay = 0;
	int bgmHandleResult = 0;

	int playHandleStageBgm = -1;
	int playHandleResultBgm = -1;
	int playHandleTitleBgm = -1;
	int playHandleStageSelectBgm = -1;

	int seHandleSelect = 0;            // メニュー選択カーソル移動音
	int seHandleDecide = 0;            // メニュー決定音
	int seHandleBack = 0;              // メニューキャンセル音
	int seHandlePause = 0;             // ポーズメニュー開く音

	int seHandlePlayerShot = 0;       // プレイヤーショット音


	/* --- BGM再生メソッド --- */
	void PlayTitleBgm();
	void PlayStageSelectBgm();
	void PlayStageBgm();
	void PlayResultBgm();
	void StopAllBgm();

	/* --- SE再生メソッド --- */
	void PlaySelectSe();
	void PlayDecideSe();
	void PlayBackSe();
	void PlayPauseSe();

	void PlaySE_PlayerShot(float ShotRatio);

	bool vibrationEnabled = true;   // 設定画面で切替予定
	float vibrationStrength = 0.5f; // 0.0f～1.0f ユーザー設定（マスタースケール）

	int  prevMouseX = 0;
	int  prevMouseY = 0;
	int  mouseX = 0;
	int  mouseY = 0;
	float lastKeyboardMouseInputTime = 0.0f;
	float lastPadInputTime = 0.0f;
	float modeSwitchCooldown = 0.15f;

	void LoadAllResources();
	void LoadCommonTextures();
	void ApplyAudioSettings();

	bool IsPlayingHandle(int h) const {
		return (h != -1) && Novice::IsPlayingAudio(h);
	}

	void PlayExclusive_(BgmKind kind);
	BgmKind currentBgm_ = BgmKind::None;

	bool explanationViewed = false; // 説明画面を既に見たか

	void MarkExplanationViewed() { explanationViewed = true; }

public:

	// ========================================
	// パーティクル管理
	// ========================================
	std::unique_ptr<ParticleManager>particleManager_;

	// ========================================
	// 入力モード管理
	// ========================================

	enum class InputMode {
		KeyboardMouse,
		Pad
	};

	/// <summary>
	/// 入力モードを更新（自動検出）
	/// </summary>
	void UpdateInputMode(const char* keys, const char* preKeys);

	/// <summary>
	/// 現在の入力モードを取得
	/// </summary>
	InputMode GetInputMode() const { return currentInputMode_; }

	/// <summary>
	/// キーボード&マウス入力中か
	/// </summary>
	bool IsKeyboardMouseMode() const { return currentInputMode_ == InputMode::KeyboardMouse; }

	/// <summary>
	/// ゲームパッド入力中か
	/// </summary>
	bool IsGamepadMode() const { return currentInputMode_ == InputMode::Pad; }

private:
	// 入力モード
	InputMode currentInputMode_ = InputMode::KeyboardMouse;

	// 入力検出用のフレームカウンタ
	int inputDetectionFrameCount_ = 0;
	static constexpr int kInputDetectionInterval = 5; // 5フレームごとに検出

	// 前回のマウス位置（移動検出用）
	int prevInputCheckMouseX_ = 0;
	int prevInputCheckMouseY_ = 0;
};