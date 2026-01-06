#include "UiDrawComponent.h"
#include "Affine2D.h"
#include <utility>
#include "Novice.h"
#include "Easing.h"


namespace {
	// フォールバック白テクスチャを遅延ロード
	int GetFallbackWhiteTexture() {
		static int sHandle = [] {
			int h = Novice::LoadTexture("./NoviceResources/white1x1.png");
			return h;
			}();
		return sHandle;
	}
}

UiDrawComponent::UiDrawComponent() {
	pos_ = {};
	scale_ = { 1.0f,1.0f };
	rotation_ = 0.0f;
	size_ = {};
	anchor_ = { 0.5f,0.5f };
	alpha_ = 1.0f;
	layer_ = 0;
	onClick_ = nullptr;
	scaleEaseTime = 0.0f;
	scaleEaseDuration = 1.0f;
	scaleStart = { 1.0f,1.0f };
	scaleTarget = { 1.0f,1.0f };
	scaleT_ = 0.0f;
	scaleTUp_ = true;
	scaleSpeed_ = 0.05f;
	baseScale_ = 1.0f;
	scaleRange_ = 0.2f;
	color_ = 0xFFFFFFFF;
}

void UiDrawComponent::SetPosition(const Vector2& pos) { pos_ = pos; }
void UiDrawComponent::SetScale(const Vector2& scale) { scale_ = scale; }
void UiDrawComponent::SetRotation(float radians) { rotation_ = radians; }
void UiDrawComponent::SetSize(const Vector2& size) { size_ = size; }
void UiDrawComponent::SetAnchor(const Vector2& anchor) { anchor_ = anchor; }
void UiDrawComponent::SetAlpha(float alpha) { alpha_ = alpha; }
float UiDrawComponent::GetAlpha() const { return alpha_; }
void UiDrawComponent::SetLayer(int32_t layer) { layer_ = layer; }
int32_t UiDrawComponent::GetLayer() const { return layer_; }


void UiDrawComponent::SetOnClick(std::function<void()> callback) { onClick_ = std::move(callback); }
void UiDrawComponent::OnClick() { if (onClick_) onClick_(); }

void UiDrawComponent::Draw() const {
	// フォールバック選択
	int texHandle = (grHandle_ >= 0) ? grHandle_ : GetFallbackWhiteTexture();
	if (texHandle < 0) {
		return;
	}

	float easedT = Easing::easeInOutSine(scaleT_);
	float animScale = baseScale_ + scaleRange_ * easedT;
	Vector2 drawScale = { scale_.x * animScale, scale_.y * animScale };

	AffineMatrix2D transform = AffineMatrix2D::MakeAffine(drawScale, rotation_, pos_);

	Vector2 localCorners[4] = {
		{0,0},{size_.x,0},{size_.x,size_.y},{0,size_.y}
	};
	for (int i = 0; i < 4; ++i) {
		localCorners[i].x -= size_.x * anchor_.x;
		localCorners[i].y -= size_.y * anchor_.y;
	}
	Vector2 screenCorners[4];
	for (int i = 0; i < 4; ++i) {
		screenCorners[i] = Matrix3x3::Transform(localCorners[i], transform);
	}

	Novice::DrawQuad(
		(int)screenCorners[0].x, (int)screenCorners[0].y,
		(int)screenCorners[1].x, (int)screenCorners[1].y,
		(int)screenCorners[3].x, (int)screenCorners[3].y,
		(int)screenCorners[2].x, (int)screenCorners[2].y,
		0, 0,
		(int)size_.x, (int)size_.y,
		grHandle_,
		color_   // WHITE ではなく設定色
	);
}

void UiDrawComponent::Update() {
	if (scaleTUp_) {
		scaleT_ += scaleSpeed_;
		if (scaleT_ >= 1.0f) { scaleT_ = 1.0f; scaleTUp_ = false; }
	} else {
		scaleT_ -= scaleSpeed_;
		if (scaleT_ <= 0.0f) { scaleT_ = 0.0f; scaleTUp_ = true; }
	}
}