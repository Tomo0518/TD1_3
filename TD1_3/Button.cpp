#include "Button.h"
#include "Easing.h"
#include <Novice.h>
#include <algorithm>

Button::Button(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback)
	: position_(position),
	size_(size),
	label_(label),
	callback_(callback),
	isImageButton_(false) {
}

Button::Button(const Vector2& position, const Vector2& size, int normalTexture, int selectedTexture, std::function<void()> callback)
	: position_(position),
	size_(size),
	callback_(callback),
	normalTexture_(normalTexture),
	selectedTexture_(selectedTexture),
	isImageButton_(true) {

	// 元画像のサイズを取得
	int texW = 0, texH = 0;
	Novice::GetTextureSize(normalTexture, &texW, &texH);

	// 基本スケールを計算（元画像サイズとボタンサイズの比率）
	if (texW > 0 && texH > 0) {
		baseScaleX_ = size_.x / static_cast<float>(texW);
		baseScaleY_ = size_.y / static_cast<float>(texH);
	}

	// 通常時のDrawComponent2D初期化
	drawCompNormal_ = DrawComponent2D(normalTexture_, 1, 1, 1, 1.0f, false);
	drawCompNormal_.SetPosition(position_);
	drawCompNormal_.SetScale({ baseScaleX_, baseScaleY_ });
	drawCompNormal_.SetAnchorPoint({ 0.5f, 0.5f });

	// 選択時のDrawComponent2D初期化
	drawCompSelected_ = DrawComponent2D(selectedTexture_, 1, 1, 1, 1.0f, false);
	drawCompSelected_.SetPosition(position_);
	drawCompSelected_.SetScale({ baseScaleX_, baseScaleY_ });
	drawCompSelected_.SetAnchorPoint({ 0.5f, 0.5f });

	// アニメーション停止
	drawCompNormal_.StopAnimation();
	drawCompSelected_.StopAnimation();
}

void Button::Update(float deltaTime, bool isSelected) {
	isSelected_ = isSelected;

	// イージングのターゲット値を設定
	float target = isSelected_ ? 1.0f : 0.0f;
	easeT_ += (target - easeT_) * std::clamp(easeSpeed_ * deltaTime, 0.0f, 1.0f);

	// イージングを適用してアニメーションスケールを計算（0.9～1.1）
	float eased = Easing::EaseOutQuad(easeT_);
	float animScale = std::lerp(scaleMin_, scaleMax_, eased);

	// 画像ボタンの場合、基本スケールにアニメーションスケールを掛ける
	if (isImageButton_) {
		float finalScaleX = baseScaleX_ * animScale;
		float finalScaleY = baseScaleY_ * animScale;

		drawCompNormal_.SetScale({ finalScaleX, finalScaleY });
		drawCompNormal_.SetPosition(position_);

		drawCompSelected_.SetScale({ finalScaleX, finalScaleY });
		drawCompSelected_.SetPosition(position_);
	}
}

void Button::Draw() {
	if (isImageButton_) {
		// 画像ボタンの描画：選択状態に応じて切り替え
		if (isSelected_) {
			drawCompSelected_.DrawScreen();
		}
		else {
			// 半透明で描画
			drawCompNormal_.SetBaseColor(colorSelected_);
			drawCompNormal_.DrawScreen();
			drawCompNormal_.SetBaseColor(colorNormal_);
		}
	}
	else {
		// テキストボタン：シンプルな矩形描画のみ
		uint32_t fillColor = isSelected_ ? colorSelected_ : colorNormal_;

		float eased = Easing::EaseOutQuad(easeT_);
		float scale = std::lerp(scaleMin_, scaleMax_, eased);
		float w = size_.x * scale;
		float h = size_.y * scale;

		float left = position_.x - w * anchor_.x;
		float top = position_.y - h * anchor_.y;

		// 塗りつぶし
		Novice::DrawBox(
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(w),
			static_cast<int>(h),
			0.0f,
			fillColor,
			kFillModeSolid
		);

		// 枠線
		uint32_t frameColor = isSelected_ ? 0xFFFFFFFF : 0x888888FF;
		Novice::DrawBox(
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(w),
			static_cast<int>(h),
			0.0f,
			frameColor,
			kFillModeWireFrame
		);

		// ラベル（Noviceのデフォルトフォントで描画）
		if (!label_.empty()) {
			int textX = static_cast<int>(position_.x - label_.length() * 3);
			int textY = static_cast<int>(position_.y - 8);
			Novice::ScreenPrintf(textX, textY, "%s", label_.c_str());
		}
	}
}

void Button::Execute() {
	if (callback_) {
		callback_();
	}
}

bool Button::IsPointInside(const Vector2& point) const {
	float halfW = size_.x / 2.0f;
	float halfH = size_.y / 2.0f;

	return (point.x >= position_.x - halfW &&
		point.x <= position_.x + halfW &&
		point.y >= position_.y - halfH &&
		point.y <= position_.y + halfH);
}