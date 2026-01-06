#pragma once
#include "Affine2D.h"
#include <functional>
#include <cstdint>

class UiDrawComponent {
public:
	UiDrawComponent();

	void SetPosition(const Vector2& pos);
	Vector2 GetPosition() const { return pos_; }
	void SetScale(const Vector2& scale);
	void SetRotation(float radians);
	void SetSize(const Vector2& size);
	void SetAnchor(const Vector2& anchor);
	void SetAlpha(float alpha);

	Vector2 GetScale() const { return scale_; }

	float GetAlpha() const;
	void SetLayer(int32_t layer);
	int32_t GetLayer() const;
	void SetOnClick(std::function<void()> callback);
	void OnClick();
	void Draw() const;
	void Update();

	void SetColor(uint32_t col) { color_ = col; }
	uint32_t GetColor() const { return color_; }

	int grHandle_ = -1;

	float baseScale_ = 1.0f;
	float scaleRange_ = 0.2f;
	uint32_t color_ = 0xFFFFFFFF;
	Vector2 GetSize() const { return size_; }

private:
	Vector2 pos_;
	Vector2 scale_{ 1.0f, 1.0f };
	float rotation_;
	Vector2 size_;
	Vector2 anchor_;
	float alpha_;
	int32_t layer_;
	std::function<void()> onClick_;

	float scaleEaseTime;
	float scaleEaseDuration;
	Vector2 scaleStart, scaleTarget;

	float scaleT_ = 0.0f;
	bool scaleTUp_ = true;
	float scaleSpeed_ = 0.05f;
};