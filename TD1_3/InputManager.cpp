#include "InputManager.h"
#include <cstring> // memcpy用
#include <cmath>   // abs用

InputManager::InputManager() {
	// 初期化
	Novice::GetHitKeyStateAll(keys_);
	memcpy(preKeys_, keys_, 256);

	int x, y;
	Novice::GetMousePosition(&x, &y);
	mousePos_ = { (float)x, (float)y };
	preMousePos_ = mousePos_;
}

void InputManager::Update() {
	// 1. キーボード更新
	memcpy(preKeys_, keys_, 256);
	Novice::GetHitKeyStateAll(keys_);

	// 2. マウス更新
	int x, y;
	Novice::GetMousePosition(&x, &y);
	mousePos_ = { (float)x, (float)y };

	for (int i = 0; i < 3; i++) {
		preMouseBtn_[i] = currMouseBtn_[i];
		currMouseBtn_[i] = Novice::IsPressMouse(i);
	}


	// 移動量（Delta）の計算
	mouseDelta_.x = mousePos_.x - preMousePos_.x;
	mouseDelta_.y = mousePos_.y - preMousePos_.y;
	preMousePos_ = mousePos_;

	// ホイール更新（Novice::GetWheelはフレームごとの変位ではなく現在値を返す仕様の場合の対応）
	// ※Noviceの仕様によりますが、一般的に変位を取得するには差分を取るか、そのまま使うか確認が必要です。
	// ここではNovice::GetWheel()が「そのフレームの回転量」を返すと仮定して実装します。
	wheel_ = Novice::GetWheel();

	// 3. パッド更新
	pad_.Update();

	// 4. 入力モードの自動検知
	UpdateInputMode();
}

// ==========================================
// キーボード入力の実装
// ==========================================

bool InputManager::TriggerKey(int diKey) const {
	return keys_[diKey] != 0 && preKeys_[diKey] == 0;
}

bool InputManager::PressKey(int diKey) const {
	return keys_[diKey] != 0;
}

bool InputManager::ReleaseKey(int diKey) const {
	return keys_[diKey] == 0 && preKeys_[diKey] != 0;
}

// ==========================================
// マウス入力の実装
// ==========================================

bool InputManager::TriggerMouse(MouseButton button) const {
	return Novice::IsTriggerMouse(static_cast<int>(button));
}

bool InputManager::PressMouse(MouseButton button) const {
	return Novice::IsPressMouse(static_cast<int>(button));
}

bool InputManager::ReleaseMouse(MouseButton button) const {
	int idx = static_cast<int>(button);
	// 「前フレーム押されていた」かつ「今押されていない」
	return preMouseBtn_[idx] && !currMouseBtn_[idx];
}

Vector2 InputManager::GetMousePosition() const {
	return mousePos_;
}

int InputManager::GetWheel() const {
	return wheel_;
}

// ==========================================
// その他・便利機能
// ==========================================

void InputManager::SetCursorVisibility(bool visible) {
	Novice::SetMouseCursorVisibility(visible ? 1 : 0);
}

void InputManager::UpdateInputMode() {
	inputDetectionTimer_++;
	if (inputDetectionTimer_ < 5) return; // 5フレームに1回判定
	inputDetectionTimer_ = 0;

	// --- パッド入力検知 ---
	bool padInput = false;

	// スティック入力（デッドゾーン考慮）
	if (std::abs(pad_.LeftX()) > 0.2f || std::abs(pad_.LeftY()) > 0.2f ||
		std::abs(pad_.RightX()) > 0.2f || std::abs(pad_.RightY()) > 0.2f) {
		padInput = true;
	}
	// ボタン入力
	// Padクラスに「何か押されたか」があればそれを使うのが早いが、なければループ
	for (int i = 0; i < (int)Pad::Button::COUNT; ++i) {
		if (pad_.Press((Pad::Button)i)) {
			padInput = true;
			break;
		}
	}

	if (padInput) {
		currentInputMode_ = InputMode::Gamepad;
		return;
	}

	// --- キーボード＆マウス入力検知 ---
	bool keyMouseInput = false;

	// マウス移動
	if (std::abs(mouseDelta_.x) > 1.0f || std::abs(mouseDelta_.y) > 1.0f) {
		keyMouseInput = true;
	}
	// マウスボタン
	if (PressMouse(MouseButton::Left) || PressMouse(MouseButton::Right)) {
		keyMouseInput = true;
	}
	// キー入力（WASDなど主要キー）
	if (PressKey(DIK_W) || PressKey(DIK_A) || PressKey(DIK_S) || PressKey(DIK_D) ||
		PressKey(DIK_SPACE) || PressKey(DIK_RETURN)) {
		keyMouseInput = true;
	}

	if (keyMouseInput) {
		currentInputMode_ = InputMode::KeyboardMouse;
	}
}