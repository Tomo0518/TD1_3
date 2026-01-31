#include "Pad.h"
#include <Windows.h>
#include <Xinput.h>
#include <algorithm>
#include <cmath>
#include "InputManager.h"

#pragma comment(lib, "Xinput.lib")

Pad::Pad(uint32_t index) : index_(index) {};

float Pad::ApplyDeadZone(float v, float dz) {
	if (std::fabs(v) < dz) return 0.0f;
	return v;
}

void Pad::ClearState() {
	now_.fill(false);
	prev_.fill(false);
	hold_.fill(0);
	leftX_ = leftY_ = rightX_ = rightY_ = 0.0f;
	leftTrigger_ = rightTrigger_ = 0.0f;
}

void Pad::ReadHardware() {
	XINPUT_STATE state{};
	DWORD res = XInputGetState(index_, &state);
	connected_ = (res == ERROR_SUCCESS);
	if (!connected_) {
		ClearState();
		return;
	}

	// ⭐ 前フレームの状態を保存
	prev_ = now_;

	unsigned short b = state.Gamepad.wButtons;

	// ⭐ ボタン状態設定のラムダ関数（修正版）
	auto setBtn = [&](Button bt, bool on) {
		size_t i = static_cast<size_t>(bt);

		// ⭐ 現在の状態を設定（押されているか否か）
		now_[i] = on;

		// ⭐ ホールドフレーム数の更新
		if (on) {
			// ボタンが押されている場合
			if (prev_[i]) {
				// 前フレームも押されていた → ホールド継続
				hold_[i] = hold_[i] + 1;
			} else {
				// 前フレームは押されていなかった → 新規押下
				hold_[i] = 1;
			}
		} else {
			// ボタンが押されていない → ホールドリセット
			hold_[i] = 0;
		}
		};

	// ⭐ 各ボタンの状態を設定
	setBtn(Button::A, (b & XINPUT_GAMEPAD_A) != 0);
	setBtn(Button::B, (b & XINPUT_GAMEPAD_B) != 0);
	setBtn(Button::X, (b & XINPUT_GAMEPAD_X) != 0);
	setBtn(Button::Y, (b & XINPUT_GAMEPAD_Y) != 0);
	setBtn(Button::DPadUp, (b & XINPUT_GAMEPAD_DPAD_UP) != 0);
	setBtn(Button::DPadDown, (b & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
	setBtn(Button::DPadLeft, (b & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
	setBtn(Button::DPadRight, (b & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
	setBtn(Button::Start, (b & XINPUT_GAMEPAD_START) != 0);
	setBtn(Button::LShoulder, (b & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
	setBtn(Button::RShoulder, (b & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
	setBtn(Button::LThumb, (b & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
	setBtn(Button::RThumb, (b & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);

	// スティック正規化
	const float stickNorm = 1.0f / 32767.0f;
	leftX_ = ApplyDeadZone(state.Gamepad.sThumbLX * stickNorm, 0.15f);
	leftY_ = ApplyDeadZone(state.Gamepad.sThumbLY * stickNorm, 0.15f);
	rightX_ = ApplyDeadZone(state.Gamepad.sThumbRX * stickNorm, 0.15f);
	rightY_ = ApplyDeadZone(state.Gamepad.sThumbRY * stickNorm, 0.15f);

	prevLeftTrigger_ = leftTrigger_;
	prevRightTrigger_ = rightTrigger_;

	// トリガ
	leftTrigger_ = state.Gamepad.bLeftTrigger / 255.0f;
	rightTrigger_ = state.Gamepad.bRightTrigger / 255.0f;
	if (leftTrigger_ < 0.05f) leftTrigger_ = 0.0f;
	if (rightTrigger_ < 0.05f) rightTrigger_ = 0.0f;
}

void Pad::ApplyVibration() {
	if (!connected_) return;

	if (vibRemainFrames_ == 0) {
		XINPUT_VIBRATION vib{};
		XInputSetState(index_, &vib);
		return;
	}

	if (vibRemainFrames_ > 0) {
		--vibRemainFrames_;
	}

	XINPUT_VIBRATION vib{};
	vib.wLeftMotorSpeed = static_cast<WORD>(20000 * std::clamp(vibLeft_, 0.0f, 1.0f));
	vib.wRightMotorSpeed = static_cast<WORD>(20000 * std::clamp(vibRight_, 0.0f, 1.0f));
	XInputSetState(index_, &vib);
}

void Pad::Update() {
	ReadHardware();
	ApplyVibration();
}

bool Pad::Press(Button b) const {
	return now_[static_cast<size_t>(b)];
}

bool Pad::Trigger(Button b) const {
	return !prev_[static_cast<size_t>(b)] && now_[static_cast<size_t>(b)];
}

bool Pad::Release(Button b)const {
	return prev_[static_cast<size_t>(b)] && !now_[static_cast<size_t>(b)];
}

int Pad::HoldFrames(Button b)const {
	return hold_[static_cast<size_t>(b)];
}

void Pad::StartVibration(float leftPower, float rightPower, int frames) {
	if (!InputManager::GetInstance().IsVibrationEnabled()) return;
	vibLeft_ = leftPower * InputManager::GetInstance().GetVibrationStrength();
	vibRight_ = rightPower * InputManager::GetInstance().GetVibrationStrength();
	vibRemainFrames_ = frames;
}

void Pad::StopVibration() {
	vibRemainFrames_ = 0;
}