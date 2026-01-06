#include "Button.h"
#include "Easing.h"
#include <Novice.h>
#include <algorithm>

Button::Button(const Vector2& position, const Vector2& size,
	const std::string& label, std::function<void()> callback)
	: position_(position)
	, size_(size)
	, label_(label)
	, callback_(callback) {

	// テクスチャは外部から設定するため、ここではnullptrのまま
	drawComp_ = nullptr;
}

// デストラクタを追加
Button::~Button() {
	if (drawComp_) {
		delete drawComp_;
		drawComp_ = nullptr;
	}
}

// コピーコンストラクタ（重要！）
Button::Button(const Button& other)
	: position_(other.position_)
	, size_(other.size_)
	, anchor_(other.anchor_)
	, label_(other.label_)
	, callback_(other.callback_)
	, colorNormal_(other.colorNormal_)
	, colorSelected_(other.colorSelected_)
	, colorFrame_(other.colorFrame_)
	, colorFrameSelected_(other.colorFrameSelected_)
	, colorText_(other.colorText_)
	, colorTextSelected_(other.colorTextSelected_)
	, textScale_(other.textScale_)
	, textScaleSelected_(other.textScaleSelected_)
	, scaleMin_(other.scaleMin_)
	, scaleMax_(other.scaleMax_)
	, easeSpeed_(other.easeSpeed_)
	, easeT_(other.easeT_)
	, isSelected_(other.isSelected_) {

	// DrawComponent2Dを深くコピー
	if (other.drawComp_) {
		drawComp_ = new DrawComponent2D(*other.drawComp_);
	}
	else {
		drawComp_ = nullptr;
	}
}

// ムーブコンストラクタ（オプション、推奨）
Button::Button(Button&& other) noexcept
	: position_(other.position_)
	, size_(other.size_)
	, anchor_(other.anchor_)
	, label_(std::move(other.label_))
	, callback_(std::move(other.callback_))
	, colorNormal_(other.colorNormal_)
	, colorSelected_(other.colorSelected_)
	, colorFrame_(other.colorFrame_)
	, colorFrameSelected_(other.colorFrameSelected_)
	, colorText_(other.colorText_)
	, colorTextSelected_(other.colorTextSelected_)
	, textScale_(other.textScale_)
	, textScaleSelected_(other.textScaleSelected_)
	, scaleMin_(other.scaleMin_)
	, scaleMax_(other.scaleMax_)
	, easeSpeed_(other.easeSpeed_)
	, easeT_(other.easeT_)
	, isSelected_(other.isSelected_)
	, drawComp_(other.drawComp_) {

	other.drawComp_ = nullptr; // 所有権を移動
}

// テクスチャを設定するメソッドを追加
void Button::SetTexture(int textureHandle) {
	if (drawComp_) {
		delete drawComp_;
	}
	drawComp_ = new DrawComponent2D(textureHandle);
	InitializeDrawComponent();
}

void Button::InitializeDrawComponent() {
	if (!drawComp_) return;

	// 位置・サイズを設定
	drawComp_->SetPosition(position_);
	drawComp_->SetDrawSize(size_);
	drawComp_->SetAnchorPoint(anchor_);

	// ベースカラーを設定（通常時の色）
	drawComp_->SetBaseColor(colorNormal_);
}

void Button::UpdateDrawComponent() {
	if (!drawComp_) return;
	// 位置・サイズを更新
	drawComp_->SetPosition(position_);
	drawComp_->SetDrawSize(size_);
	drawComp_->SetAnchorPoint(anchor_);
}

void Button::SetPosition(const Vector2& position) {
	position_ = position;
	if (drawComp_) {
		drawComp_->SetPosition(position_);
	}
}

void Button::SetSize(const Vector2& size) {
	size_ = size;
	if (drawComp_) {
		drawComp_->SetDrawSize(size_);
	}
}

void Button::Update(float deltaTime, bool isSelected) {
	bool wasSelected = isSelected_;
	isSelected_ = isSelected;

	// 選択状態が変わった時の処理
	if (isSelected && !wasSelected) {
		// 選択された時: パルスエフェクト開始
		if (drawComp_) {
			drawComp_->StartPulse(scaleMin_, scaleMax_, 2.0f, true);
			drawComp_->StartColorTransition(
				ColorRGBA::FromUInt(colorSelected_),
				0.2f
			);
		}
	}
	else if (!isSelected && wasSelected) {
		// 選択解除された時: エフェクト停止
		if (drawComp_) {
			drawComp_->StopScale();
			drawComp_->StartColorTransition(
				ColorRGBA::FromUInt(colorNormal_),
				0.2f
			);
		}
	}

	// イージングのターゲット値を設定
	float target = isSelected_ ? 1.0f : 0.0f;

	// イージングでスケールを変更（スムーズな遷移）
	easeT_ += (target - easeT_) * std::clamp(easeSpeed_ * deltaTime, 0.0f, 1.0f);

	// DrawComponent2D を更新
	if (drawComp_) {
		drawComp_->Update(deltaTime);
	}
}

void Button::Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer) const {
	textureHandle; // 未使用パラメータ回避
	if (!drawComp_) return;

	// ========== 背景描画（DrawComponent2D使用） ==========
	drawComp_->DrawScreen();

	// ========== 枠線描画 ==========
	Vector2 finalScale = drawComp_->GetScale();
	float w = size_.x * finalScale.x;
	float h = size_.y * finalScale.y;

	// アンカーに基づいて左上の座標を計算
	float left = position_.x - w * anchor_.x;
	float top = position_.y - h * anchor_.y;

	uint32_t frameColor = isSelected_ ? colorFrameSelected_ : colorFrame_;
	Novice::DrawBox(
		static_cast<int>(left),
		static_cast<int>(top),
		static_cast<int>(w),
		static_cast<int>(h),
		0.0f,
		frameColor,
		kFillModeWireFrame
	);

	// ========== テキスト描画 ==========
	if (font && textRenderer) {
		float labelScale = isSelected_ ? textScaleSelected_ : textScale_;
		uint32_t textColor = isSelected_ ? colorTextSelected_ : colorText_;

		// テキストの幅を計算して中央に配置
		int textWidth = textRenderer->MeasureWidth(label_.c_str(), labelScale);
		int textHeight = static_cast<int>(font->GetLineHeight() * labelScale);

		int textX = static_cast<int>(left + (w - textWidth) * 0.5f);
		int textY = static_cast<int>(top + (h - textHeight) * 0.5f);

		textRenderer->DrawTextLabel(textX, textY, label_.c_str(), textColor, labelScale);
	}
}

void Button::Execute() {
	if (callback_) {
		callback_();
	}
}