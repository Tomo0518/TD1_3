#include "GameShared.h"
#include <algorithm>

GameShared::GameShared() {
	LoadAllResources();

	// マネージャー生成
	particleManager_ = std::make_unique<ParticleManager>();

	// 共通テクスチャ読み込み
	LoadCommonTextures();

	// パーティクルのリソースもここで読み込む！
	particleManager_->LoadCommonResources();
}

void GameShared::LoadAllResources() {
	/* ===== 基本テクスチャ ===== */
	texWhite = Novice::LoadTexture("./NoviceResources/white1x1.png");

	/* ===== BGM読み込み ===== */

	// タイトルとステージセレクトのbgmは共通タイトルからステージセレクトに行く時にとぎれないようにするため
	bgmHandleTitle = Novice::LoadAudio("./Resources/sounds/BGM/title.mp3");
	bgmHandleStageSelect = bgmHandleTitle;
	bgmHandlePlay = Novice::LoadAudio("./Resources/sounds/BGM/stage.mp3");
	bgmHandleResult = Novice::LoadAudio("./Resources/sounds/BGM/result.mp3");

	/* ===== SE読み込み ===== */
	// プレイヤー関連
	seHandlePlayerShot = Novice::LoadAudio("./Resources/sounds/SE/playerShot.mp3");

	// 敵関連

	// アイテム関連

	// UI関連
	seHandleSelect = Novice::LoadAudio("./Resources/sounds/SE/moveSelect.mp3");
	seHandleDecide = Novice::LoadAudio("./Resources/sounds/SE/decide.mp3");
	seHandleBack = Novice::LoadAudio("./Resources/sounds/SE/cancel.mp3");
	seHandlePause = Novice::LoadAudio("./Resources/sounds/SE/cancel.mp3");
}

void GameShared::LoadCommonTextures() {
	Novice::GetMousePosition(&mouseX, &mouseY);
	prevMouseX = mouseX;
	prevMouseY = mouseY;
}

void GameShared::PlayExclusive_(BgmKind kind) {
	if (currentBgm_ == kind) {
		// 既に再生中の場合は音量のみ更新
		switch (kind) {
		case BgmKind::Title:
			if (IsPlayingHandle(playHandleTitleBgm))
				Novice::SetAudioVolume(playHandleTitleBgm, userBgmVolume);
			break;
		case BgmKind::Stage:
			if (IsPlayingHandle(playHandleStageBgm))
				Novice::SetAudioVolume(playHandleStageBgm, userBgmVolume);
			break;
		case BgmKind::Result:
			if (IsPlayingHandle(playHandleResultBgm))
				Novice::SetAudioVolume(playHandleResultBgm, userBgmVolume);
			break;
		default: break;
		}
		return;
	}

	// 全BGM停止
	if (IsPlayingHandle(playHandleTitleBgm)) Novice::StopAudio(playHandleTitleBgm);
	if (IsPlayingHandle(playHandleStageBgm)) Novice::StopAudio(playHandleStageBgm);
	if (IsPlayingHandle(playHandleResultBgm)) Novice::StopAudio(playHandleResultBgm);
	if (IsPlayingHandle(playHandleStageSelectBgm)) Novice::StopAudio(playHandleStageSelectBgm);

	playHandleTitleBgm = -1;
	playHandleStageBgm = -1;
	playHandleResultBgm = -1;
	playHandleStageSelectBgm = -1;
	currentBgm_ = BgmKind::None;

	// 新しいBGM再生
	switch (kind) {
	case BgmKind::Title:
		playHandleTitleBgm = Novice::PlayAudio(bgmHandleTitle, true, userBgmVolume);
		break;
	case BgmKind::Stage:
		playHandleStageBgm = Novice::PlayAudio(bgmHandlePlay, true, userBgmVolume);
		break;
	case BgmKind::Result:
		playHandleResultBgm = Novice::PlayAudio(bgmHandleResult, true, userBgmVolume);
		break;
	case BgmKind::None:
	default: break;
	}
	currentBgm_ = kind;
}

void GameShared::PlayTitleBgm() {
	PlayExclusive_(BgmKind::Title);
}

void GameShared::PlayStageSelectBgm() {
	if (IsPlayingHandle(playHandleStageSelectBgm)) {
		Novice::SetAudioVolume(playHandleStageSelectBgm, userBgmVolume);
		return;
	}
	StopAllBgm();
	playHandleStageSelectBgm = Novice::PlayAudio(bgmHandleStageSelect, true, userBgmVolume);
}

void GameShared::PlayStageBgm() {
	PlayExclusive_(BgmKind::Stage);
}

void GameShared::PlayResultBgm() {
	PlayExclusive_(BgmKind::Result);
}

void GameShared::StopAllBgm() {
	PlayExclusive_(BgmKind::None);
}

void GameShared::ApplyAudioSettings() {
	// ========================================
	// BGM音量の更新
	// ========================================

	// PlayExclusive_で管理されているBGM
	switch (currentBgm_) {
	case BgmKind::Title:
		if (IsPlayingHandle(playHandleTitleBgm))
			Novice::SetAudioVolume(playHandleTitleBgm, userBgmVolume);
		break;
	case BgmKind::Stage:
		if (IsPlayingHandle(playHandleStageBgm))
			Novice::SetAudioVolume(playHandleStageBgm, userBgmVolume);
		break;
	case BgmKind::Result:
		if (IsPlayingHandle(playHandleResultBgm))
			Novice::SetAudioVolume(playHandleResultBgm, userBgmVolume);
		break;
	default:
		break;
	}

	// ステージセレクトBGM（独立管理）
	if (IsPlayingHandle(playHandleStageSelectBgm)) {
		Novice::SetAudioVolume(playHandleStageSelectBgm, userBgmVolume);
	}

	// ========================================
	// デバッグ出力（設定反映確認用）
	// ========================================
#ifdef _DEBUG
	char debugMsg[256];
	snprintf(debugMsg, sizeof(debugMsg),
		"[GameShared] Audio settings applied: BGM=%.2f, SE=%.2f, Vibration=%s (%.2f)\n",
		userBgmVolume, userSeVolume,
		vibrationEnabled ? "ON" : "OFF", vibrationStrength);
	Novice::ConsolePrintf(debugMsg);
#endif
}

/* --- SE再生メソッド実装 --- */
void GameShared::PlaySelectSe() {
	Novice::PlayAudio(seHandleSelect, false, userSeVolume);
}

void GameShared::PlayDecideSe() {
	Novice::PlayAudio(seHandleDecide, false, userSeVolume);
}

void GameShared::PlayBackSe() {
	Novice::PlayAudio(seHandleBack, false, userSeVolume);
}

void GameShared::PlayPauseSe() {
	Novice::PlayAudio(seHandlePause, false, userSeVolume);
}

// =======================================
// ゲーム内のSE再生メソッド実装
// =======================================

/// プレイヤーショット音再生
void GameShared::PlaySE_PlayerShot(float ShotRatio) {
	Novice::PlayAudio(seHandlePlayerShot, false, userSeVolume * ShotRatio * 0.5f);
}

// ========================================
// ステージデータ操作メソッド
// ========================================

/// <summary>
/// ステージのスコアを更新（ベストスコアより高い場合のみ）
/// </summary>
void GameShared::UpdateStageScore(int stageIndex, int score) {
	// インデックスチェック（1始まり → 0始まりに変換）
	int index = stageIndex - 1;
	if (index < 0 || index >= kMaxStageNum) return;

	// ベストスコアより高い場合のみ更新
	if (score > stageData_[index].bestScore) {
		stageData_[index].bestScore = score;
	}

	// プレイ回数をインクリメント
	stageData_[index].playCount++;
}

/// <summary>
/// ステージのクリアフラグを設定
/// </summary>
void GameShared::SetStageCleared(int stageIndex, bool cleared) {
	int index = stageIndex - 1;
	if (index < 0 || index >= kMaxStageNum) return;

	stageData_[index].isCleared = cleared;
}

/// <summary>
/// ステージがクリア済みか確認
/// </summary>
bool GameShared::IsStageCleared(int stageIndex) const {
	int index = stageIndex - 1;
	if (index < 0 || index >= kMaxStageNum) return false;

	return stageData_[index].isCleared;
}

/// <summary>
/// ステージのベストスコアを取得
/// </summary>
int GameShared::GetBestScore(int stageIndex) const {
	int index = stageIndex - 1;
	if (index < 0 || index >= kMaxStageNum) return 0;

	return stageData_[index].bestScore;
}

/// <summary>
/// 新記録かどうか判定
/// </summary>
bool GameShared::IsNewRecord(int stageIndex, int score) const {
	int index = stageIndex - 1;
	if (index < 0 || index >= kMaxStageNum) return false;

	// 初回プレイまたはベストスコアより高い場合は新記録
	return stageData_[index].bestScore == 0 || score > stageData_[index].bestScore;
}

/// <summary>
/// ステージデータを保存（将来的に実装）
/// </summary>
void GameShared::SaveStageData() {
	// TODO: ファイルI/Oでデータを保存
	// 例: JSON形式でResources/save/stageData.jsonに保存
}

/// <summary>
/// ステージデータを読み込み（将来的に実装）
/// </summary>
void GameShared::LoadStageData() {
	// TODO: ファイルI/Oでデータを読み込み
	// 例: JSON形式でResources/save/stageData.jsonから読み込み
}


void GameShared::UpdateMousePosition() {
	Novice::GetMousePosition(&mouseX, &mouseY);
}

void GameShared::UpdateInputMode(const char* keys, const char* preKeys) {

	preKeys; // 未使用

	inputDetectionFrameCount_++;

	// 5フレームごとに入力を検出
	if (inputDetectionFrameCount_ < kInputDetectionInterval) {
		return;
	}

	inputDetectionFrameCount_ = 0;

	// ========================================
	// ゲームパッド入力の検出
	// ========================================
	if (pad.IsConnected()) {
		const float stickThreshold = 0.2f;
		const float triggerThreshold = 0.1f;

		// スティック入力
		bool hasStickInput =
			std::abs(pad.LeftX()) > stickThreshold ||
			std::abs(pad.LeftY()) > stickThreshold ||
			std::abs(pad.RightX()) > stickThreshold ||
			std::abs(pad.RightY()) > stickThreshold;

		// トリガー入力
		bool hasTriggerInput =
			pad.LeftTrigger() > triggerThreshold ||
			pad.RightTrigger() > triggerThreshold;

		// ボタン入力（すべてのボタンをチェック）
		bool hasButtonInput = false;
		for (int i = 0; i < static_cast<int>(Pad::Button::COUNT); ++i) {
			if (pad.Press(static_cast<Pad::Button>(i))) {
				hasButtonInput = true;
				break;
			}
		}

		// いずれかのゲームパッド入力があればゲームパッドモードに切り替え
		if (hasStickInput || hasTriggerInput || hasButtonInput) {
			currentInputMode_ = InputMode::Pad;
			return;
		}
	}

	// ========================================
	// キーボード&マウス入力の検出
	// ========================================

	// キーボード入力チェック（WASD + よく使うキー）
	bool hasKeyboardInput =
		keys[DIK_W] || keys[DIK_A] || keys[DIK_S] || keys[DIK_D] ||
		keys[DIK_SPACE] ||
		keys[DIK_ESCAPE] || keys[DIK_RETURN];

	// マウス移動検出
	int currentMouseX, currentMouseY;
	Novice::GetMousePosition(&currentMouseX, &currentMouseY);

	int mouseDeltaX = currentMouseX - prevInputCheckMouseX_;
	int mouseDeltaY = currentMouseY - prevInputCheckMouseY_;

	const int mouseMovementThreshold = 3; // ピクセル
	bool hasMouseMovement =
		std::abs(mouseDeltaX) > mouseMovementThreshold ||
		std::abs(mouseDeltaY) > mouseMovementThreshold;

	prevInputCheckMouseX_ = currentMouseX;
	prevInputCheckMouseY_ = currentMouseY;

	// マウスボタン入力
	bool hasMouseButtonInput =
		Novice::IsTriggerMouse(0) || Novice::IsTriggerMouse(1) || Novice::IsTriggerMouse(2);

	// いずれかのキーボード&マウス入力があればキーボード&マウスモードに切り替え
	if (hasKeyboardInput || hasMouseMovement || hasMouseButtonInput) {
		currentInputMode_ = InputMode::KeyboardMouse;
		return;
	}
}