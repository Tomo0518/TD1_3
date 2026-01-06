#pragma once
#include <Novice.h>
#include "Vector2.h"
#include "Pad.h"     // 既存のPadクラスを統合

// マウスボタンのエイリアス（マジックナンバー防止）
enum class MouseButton {
	Left = 0,
	Right = 1,
	Middle = 2
};

// 入力モード（どちらで操作しているか）
enum class InputMode {
	KeyboardMouse,
	Gamepad
};

class InputManager {
public:
	// コンストラクタ
	InputManager();

	// 毎フレーム呼ぶ更新処理
	void Update();

	// ==========================================
	// キーボード入力
	// ==========================================

	// キーが押された瞬間だけ true
	bool TriggerKey(int diKey) const;

	// キーが押されている間 true
	bool PressKey(int diKey) const;

	// キーが離された瞬間だけ true
	bool ReleaseKey(int diKey) const;

	// ==========================================
	// マウス入力
	// ==========================================

	// マウスボタンが押された瞬間だけ true
	bool TriggerMouse(MouseButton button) const;

	// マウスボタンが押されている間 true
	bool PressMouse(MouseButton button) const;

	// マウスボタンが離された瞬間だけ true
	bool ReleaseMouse(MouseButton button) const;

	// マウスの位置を取得
	Vector2 GetMousePosition() const;

	// マウスの移動量（前フレームからの差分）を取得
	// ※カメラ操作などに便利
	Vector2 GetMouseDelta() const { return mouseDelta_; }

	// マウスホイールの回転量を取得（奥: +, 手前: -）
	int GetWheel() const;

	// ==========================================
	// ゲームパッド & モード管理
	// ==========================================

	// パッドを取得（直接操作したい場合用）
	Pad* GetPad() { return &pad_; }

	// 現在の入力モードを取得（自動切り替え）
	InputMode GetInputMode() const { return currentInputMode_; }

	// カーソル表示・非表示の切り替え（便利機能）
	void SetCursorVisibility(bool visible);

private:
	// キーボード状態
	char keys_[256] = { 0 };
	char preKeys_[256] = { 0 };

	// マウス状態
	int wheel_ = 0;
	int prevWheel_ = 0; // ホイールの変化量用
	Vector2 mousePos_ = { 0, 0 };
	Vector2 preMousePos_ = { 0, 0 }; // 移動量計算用
	Vector2 mouseDelta_ = { 0, 0 };

	// マウスボタンの状態保存用（左、右、中）
	bool preMouseBtn_[3] = { false, false, false };
	bool currMouseBtn_[3] = { false, false, false };

	// パッド
	Pad pad_;

	// 入力モード管理
	InputMode currentInputMode_ = InputMode::KeyboardMouse;
	int inputDetectionTimer_ = 0;

	// モード自動切り替えの内部処理
	void UpdateInputMode();
};