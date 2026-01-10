#pragma once
#include <cstdint>
#include <array>
#include "Vector2.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

class Pad {
public:

	enum class Button : int {
		A, B, X, Y,
		DPadUp, DPadDown, DPadLeft, DPadRight,
		Start, Back,
		LShoulder, RShoulder,
		LThumb, RThumb,
		COUNT
	};

	explicit Pad(uint32_t index = 0);

	void Update();

	bool Press(Button b)   const;
	bool Trigger(Button b) const;
	bool Release(Button b) const;
	int  HoldFrames(Button b) const;

	float LeftX()  const { return leftX_; }
	float LeftY()  const { return leftY_; }

	Vector2 GetLeftStick() const { return { leftX_, leftY_ }; }
	Vector2 GetRightStick() const { return { rightX_, rightY_ }; }

	float RightX() const { return rightX_; }
	float RightY() const { return rightY_; }

	float LeftTrigger()  const { return leftTrigger_; }
	float RightTrigger() const { return rightTrigger_; }

	void StartVibration(float leftPower, float rightPower, int frames);
	void StopVibration();
	bool IsConnected() const { return connected_; }

private:
	uint32_t index_;
	bool connected_ = false;

	std::array<bool, static_cast<size_t>(Button::COUNT)> now_{};
	std::array<bool, static_cast<size_t>(Button::COUNT)> prev_{};
	std::array<int, static_cast<size_t>(Button::COUNT)> hold_{};

	float leftX_ = 0.0f, leftY_ = 0.0f;
	float rightX_ = 0.0f, rightY_ = 0.0f;
	float leftTrigger_ = 0.0f, rightTrigger_ = 0.0f;

	int   vibRemainFrames_ = 0;
	float vibLeft_ = 0.0f, vibRight_ = 0.0f;

	static float ApplyDeadZone(float v, float dz);
	void ApplyVibration();
	void ReadHardware();
	void ClearState();
};