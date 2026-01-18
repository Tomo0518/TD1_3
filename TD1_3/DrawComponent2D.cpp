#include "DrawComponent2D.h"
#include "Affine2D.h"
#include <algorithm>
#include "TextureManager.h"

// ========== コンストラクタ ==========

DrawComponent2D::DrawComponent2D(int graphHandle, int divX, int divY,
	int totalFrames, float speed, bool isLoop)
	: graphHandle_(graphHandle) {

	// 画像サイズを自動取得
	InitializeImageSize(divX, divY);

	// アニメーション作成
	int frameWidth = static_cast<int>(imageSize_.x);
	int frameHeight = static_cast<int>(imageSize_.y);
	animation_ = std::make_unique<Animation>(
		graphHandle, frameWidth, frameHeight, totalFrames, divX, speed, isLoop
	);
	animation_->Play();
}

DrawComponent2D::DrawComponent2D(TextureId textureId, int divX, int divY,
	int totalFrames, float speed, bool isLoop) {

	graphHandle_ = TextureManager::GetInstance().GetTexture(textureId);

	// 画像サイズを自動取得
	InitializeImageSize(divX, divY);

	// アニメーション作成
	int frameWidth = static_cast<int>(imageSize_.x);
	int frameHeight = static_cast<int>(imageSize_.y);
	animation_ = std::make_unique<Animation>(
		graphHandle_, frameWidth, frameHeight, totalFrames, divX, speed, isLoop
	);
	animation_->Play();
}

DrawComponent2D::DrawComponent2D(int graphHandle)
	: graphHandle_(graphHandle) {

	// 静止画として初期化（1x1分割、1フレーム）
	InitializeImageSize(1, 1);

	// 内部的には1フレームのアニメーションとして扱う
	int frameWidth = static_cast<int>(imageSize_.x);
	int frameHeight = static_cast<int>(imageSize_.y);
	animation_ = std::make_unique<Animation>(
		graphHandle, frameWidth, frameHeight, 1, 1, 0.0f, false
	);
}

DrawComponent2D::DrawComponent2D()
	: graphHandle_(-1), imageSize_{ 0.0f, 0.0f }, drawSize_{ 0.0f, 0.0f } {
}

// コピーコンストラクタ
DrawComponent2D::DrawComponent2D(const DrawComponent2D& other)
	: graphHandle_(other.graphHandle_)
	, imageSize_(other.imageSize_)
	, drawSize_(other.drawSize_)
	, transform_(other.transform_)
	, anchorPoint_(other.anchorPoint_)
	, baseColor_(other.baseColor_)
	, flipX_(other.flipX_)
	, flipY_(other.flipY_)
	, effect_(other.effect_) {

	if (other.animation_) {
		animation_ = std::make_unique<Animation>(*other.animation_);
	}
}

// ムーブコンストラクタ
DrawComponent2D::DrawComponent2D(DrawComponent2D&& other) noexcept
	: graphHandle_(other.graphHandle_)
	, imageSize_(other.imageSize_)
	, drawSize_(other.drawSize_)
	, transform_(other.transform_)
	, anchorPoint_(other.anchorPoint_)
	, baseColor_(other.baseColor_)
	, flipX_(other.flipX_)
	, flipY_(other.flipY_)
	, animation_(std::move(other.animation_))
	, effect_(std::move(other.effect_)) {
}

// コピー代入演算子
DrawComponent2D& DrawComponent2D::operator=(const DrawComponent2D& other) {
	if (this != &other) {
		graphHandle_ = other.graphHandle_;
		imageSize_ = other.imageSize_;
		drawSize_ = other.drawSize_;
		transform_ = other.transform_;
		anchorPoint_ = other.anchorPoint_;
		baseColor_ = other.baseColor_;
		flipX_ = other.flipX_;
		flipY_ = other.flipY_;
		effect_ = other.effect_;

		if (other.animation_) {
			animation_ = std::make_unique<Animation>(*other.animation_);
		}
		else {
			animation_.reset();
		}
	}
	return *this;
}

// ムーブ代入演算子
DrawComponent2D& DrawComponent2D::operator=(DrawComponent2D&& other) noexcept {
	if (this != &other) {
		graphHandle_ = other.graphHandle_;
		imageSize_ = other.imageSize_;
		drawSize_ = other.drawSize_;
		transform_ = other.transform_;
		anchorPoint_ = other.anchorPoint_;
		baseColor_ = other.baseColor_;
		flipX_ = other.flipX_;
		flipY_ = other.flipY_;
		animation_ = std::move(other.animation_);
		effect_ = std::move(other.effect_);
	}
	return *this;
}

// ========== 初期化 ==========
void DrawComponent2D::InitializeImageSize(int divX, int divY) {
	// ハンドルが無効な場合は早期リターン
	if (graphHandle_ < 0) {
		imageSize_ = { 0.0f, 0.0f };
		drawSize_ = { 0.0f, 0.0f };
		return;
	}

	// divXとdivYのゼロチェック（ゼロ除算回避）
	if (divX <= 0) divX = 1;
	if (divY <= 0) divY = 1;

	// 画像全体のサイズを取得
	int fullWidth = 0, fullHeight = 0;
	Novice::GetTextureSize(graphHandle_, &fullWidth, &fullHeight);

	// GetTextureSizeが失敗した場合のチェック
	if (fullWidth <= 0 || fullHeight <= 0) {
		imageSize_ = { 0.0f, 0.0f };
		drawSize_ = { 0.0f, 0.0f };
#ifdef _DEBUG
		Novice::ConsolePrintf("DrawComponent2D::InitializeImageSize - Invalid texture size: %d x %d (handle: %d)\n",
			fullWidth, fullHeight, graphHandle_);
#endif
		return;
	}

	// 1フレーム分のサイズを計算
	imageSize_.x = static_cast<float>(fullWidth) / divX;
	imageSize_.y = static_cast<float>(fullHeight) / divY;

	// 描画サイズのデフォルトは画像サイズと同じ
	drawSize_ = imageSize_;
}

// ========== 更新 ==========

void DrawComponent2D::Update(float deltaTime) {
	// アニメーション更新
	if (animation_) {
		animation_->Update(deltaTime);
	}

	// エフェクト更新
	effect_.Update(deltaTime);
}

// ========== 描画 ==========
void DrawComponent2D::Draw(const Camera2D& camera) {
	Matrix3x3 vpMatrix = camera.GetVpVpMatrix();

	// カメラのY軸反転設定を確認してスケールを調整
	if (camera.IsWorldYUp()) {
		// Y軸反転が有効な場合、スケールのY成分を反転
		Vector2 originalScale = transform_.scale;
		transform_.scale.y *= -1.0f;

		DrawInternal(&vpMatrix);

		// スケールを元に戻す
		transform_.scale = originalScale;
	}
	else {
		DrawInternal(&vpMatrix);
	}
}

void DrawComponent2D::DrawWorld() {
	DrawInternal(nullptr);
}

void DrawComponent2D::DrawScreen() {
	// スクリーン座標用の変換（Y軸反転なし）
	DrawInternal(nullptr);
}

void DrawComponent2D::Initialize() {
	// アニメーションがある場合は再生を開始
	if (animation_) {
		animation_->Initialize();
	}

	// エフェクトをリセット
	effect_.StopAll();
}

void DrawComponent2D::DrawInternal(const Matrix3x3* vpMatrix) {
	if (graphHandle_ < 0) return;

	// 1. ソース矩形（テクスチャのどこを読むか）を計算
	int srcX, srcY, srcW, srcH;
	GetSourceRect(srcX, srcY, srcW, srcH); // アニメーション対応：現在のフレームの座標を取得

	// 2. クロップ計算（ソースと描画サイズの両方に適用する！）
	float currentSrcW = static_cast<float>(srcW);
	float currentSrcH = static_cast<float>(srcH);

	// 描画サイズも変更するための変数を用意
	float currentDrawW = drawSize_.x;
	float currentDrawH = drawSize_.y;

	if (cropDirection_ == CropDirection::Horizontal) {
		// 幅を削る
		currentSrcW *= cropRatio_; // テクスチャを削る
		currentDrawW *= cropRatio_; // 画面上の表示幅も削る
	}
	else {
		// 高さを削る
		currentSrcH *= cropRatio_;
		currentDrawH *= cropRatio_;
	}

	// 3. 頂点座標を計算（削った currentDrawW/H を使う）
	Vector2 localVertices[4];

	// アンカーポイントのオフセット計算
	// ※注意：HPバーの場合、AnchorPointは {0.0f, 0.5f} (左) に設定しないと、
	float anchorOffsetX = drawSize_.x * anchorPoint_.x;
	float anchorOffsetY = drawSize_.y * anchorPoint_.y;

	localVertices[0] = { -anchorOffsetX, -anchorOffsetY };                // 左上
	localVertices[1] = { currentDrawW - anchorOffsetX, -anchorOffsetY };  // 右上 (width変更)
	localVertices[2] = { currentDrawW - anchorOffsetX, currentDrawH - anchorOffsetY }; // 右下 (width/height変更)
	localVertices[3] = { -anchorOffsetX, currentDrawH - anchorOffsetY };  // 左下 (height変更)

	// --- ここから下は変更なし（行列計算など） ---

	// エフェクト適用後の変換行列を取得
	Matrix3x3 worldMatrix = GetFinalTransformMatrix();
	Matrix3x3 finalMatrix = worldMatrix;
	if (vpMatrix) {
		finalMatrix = Matrix3x3::Multiply(worldMatrix, *vpMatrix);
	}

	// 変換行列を適用
	Vector2 screenVertices[4];
	for (int i = 0; i < 4; ++i) {
		screenVertices[i] = Matrix3x3::Transform(localVertices[i], finalMatrix);
	}

	// 反転処理
	if (flipX_) {
		std::swap(screenVertices[0], screenVertices[1]);
		std::swap(screenVertices[2], screenVertices[3]);
	}
	if (flipY_) {
		std::swap(screenVertices[0], screenVertices[3]);
		std::swap(screenVertices[1], screenVertices[2]);
	}

	// 描画
	Novice::DrawQuad(
		static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
		static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
		static_cast<int>(screenVertices[3].x), static_cast<int>(screenVertices[3].y),
		static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
		srcX, srcY, // アニメーションの開始位置はそのまま
		static_cast<int>(currentSrcW), // 削った幅
		static_cast<int>(currentSrcH), // 削った高さ
		graphHandle_,
		GetFinalColor()
	);
}

// ========== 内部処理 ==========

Matrix3x3 DrawComponent2D::GetFinalTransformMatrix() const {
	Vector2 finalPos = GetFinalPosition();
	Vector2 finalScale = GetFinalScale();
	float finalRotation = GetFinalRotation();

	return AffineMatrix2D::MakeAffine(finalScale, finalRotation, finalPos);
}

Vector2 DrawComponent2D::GetFinalPosition() const {
	Vector2 pos = transform_.translate;
	Vector2 offset = effect_.GetPositionOffset();
	return { pos.x + offset.x, pos.y + offset.y };
}

Vector2 DrawComponent2D::GetFinalScale() const {
	Vector2 effectScale = effect_.GetScaleMultiplier();
	return { transform_.scale.x * effectScale.x, transform_.scale.y * effectScale.y };
}

float DrawComponent2D::GetFinalRotation() const {
	return transform_.rotation + effect_.GetRotationOffset();
}

Vector2 DrawComponent2D::GetFinalDrawSize() const {
	Vector2 finalScale = GetFinalScale();
	return { drawSize_.x * finalScale.x, drawSize_.y * finalScale.y };
}

unsigned int DrawComponent2D::GetFinalColor() const {
	return effect_.GetFinalColor(baseColor_);
}

void DrawComponent2D::GetSourceRect(int& srcX, int& srcY, int& srcW, int& srcH) const {
	if (animation_) {
		srcX = animation_->GetSrcX();
		srcY = animation_->GetSrcY();
		srcW = animation_->GetSrcW();
		srcH = animation_->GetSrcH();
	}
	else {
		srcX = 0;
		srcY = 0;
		srcW = static_cast<int>(imageSize_.x);
		srcH = static_cast<int>(imageSize_.y);
	}
}

// ========== 画像設定 ==========

void DrawComponent2D::SetGraphHandle(int handle) {
	graphHandle_ = handle;
	// 画像サイズを再計算
	if (animation_) {
		int divX = animation_->GetTotalFrames() > 0 ?
			(animation_->GetTotalFrames() / animation_->GetFrameHeight()) : 1;
		int divY = animation_->GetFrameHeight() > 0 ?
			(animation_->GetFrameHeight() / animation_->GetFrameWidth()) : 1;
		InitializeImageSize(divX, divY);
	}
}

// ========== アニメーション制御 ==========

void DrawComponent2D::PlayAnimation() {
	if (animation_) animation_->Play();
}

void DrawComponent2D::StopAnimation() {
	if (animation_) animation_->Stop();
}

void DrawComponent2D::PauseAnimation() {
	if (animation_) animation_->Pause();
}

void DrawComponent2D::ResumeAnimation() {
	if (animation_) animation_->Resume();
}

void DrawComponent2D::SetAnimationFrame(int frame) {
	if (animation_) animation_->SetFrame(frame);
}

bool DrawComponent2D::IsAnimationPlaying() const {
	return animation_ ? animation_->IsPlaying() : false;
}

int DrawComponent2D::GetCurrentFrame() const {
	return animation_ ? animation_->GetCurrentFrame() : 0;
}

int DrawComponent2D::GetTotalFrames() const {
	return animation_ ? animation_->GetTotalFrames() : 1;
}

void DrawComponent2D::Setup(int graphHandle, int divX, int divY, int totalFrames, float speed, bool isLoop) {
	// 画像ハンドルを設定
	graphHandle_ = graphHandle;

	// ゼロチェック
	if (divX <= 0) divX = 1;
	if (divY <= 0) divY = 1;
	if (totalFrames <= 0) totalFrames = 1;

	// 画像サイズを再計算
	InitializeImageSize(divX, divY);

	// 画像サイズが無効な場合は処理を中断
	if (imageSize_.x <= 0.0f || imageSize_.y <= 0.0f) {
#ifdef _DEBUG
		Novice::ConsolePrintf("DrawComponent2D::Setup - Failed to initialize image size\n");
#endif
		return;
	}

	// アニメーションを再生成
	int frameWidth = static_cast<int>(imageSize_.x);
	int frameHeight = static_cast<int>(imageSize_.y);

	// フレームサイズが無効な場合もチェック
	if (frameWidth <= 0 || frameHeight <= 0) {
#ifdef _DEBUG
		Novice::ConsolePrintf("DrawComponent2D::Setup - Invalid frame size: %d x %d\n", frameWidth, frameHeight);
#endif
		return;
	}

	if (totalFrames > 1 || (divX > 1 || divY > 1)) {
		animation_ = std::make_unique<Animation>(
			graphHandle, frameWidth, frameHeight, totalFrames, divX, speed, isLoop
		);
		if (animation_) {
			animation_->Play();
		}
	}
	else {
		// 静止画の場合は1フレームのアニメーションとして扱う
		animation_ = std::make_unique<Animation>(
			graphHandle, frameWidth, frameHeight, 1, 1, 0.0f, false
		);
	}

	// エフェクトをリセット
	effect_.StopAll();

	// フェードエフェクトの色を明示的に初期化
	effect_ = Effect(); // コンストラクタで再初期化

	// 基本パラメータをリセット
	transform_.translate = { 0.0f, 0.0f };
	transform_.scale = { 1.0f, 1.0f };
	transform_.rotation = 0.0f;

	anchorPoint_ = { 0.5f, 0.5f };
	baseColor_ = 0xFFFFFFFF;
	flipX_ = false;
	flipY_ = false;
}

// ========== デバッグ ==========

#ifdef _DEBUG
void DrawComponent2D::DrawDebugWindow(const char* windowName) {
	ImGui::Begin(windowName);

	ImGui::Text("=== Transform ===");
	ImGui::DragFloat2("Position", &transform_.translate.x, 1.0f);
	ImGui::DragFloat2("Scale", &transform_.scale.x, 0.01f);
	ImGui::SliderAngle("Rotation", &transform_.rotation);
	ImGui::DragFloat2("Anchor", &anchorPoint_.x, 0.01f, 0.0f, 1.0f);

	ImGui::Text("=== Size ===");
	ImGui::Text("Image Size: %.0f x %.0f", imageSize_.x, imageSize_.y);
	ImGui::DragFloat2("Draw Size", &drawSize_.x, 1.0f);
	if (ImGui::Button("Reset Draw Size")) {
		ResetDrawSize();
	}

	ImGui::Text("=== Color ===");
	ColorRGBA color = ColorRGBA::FromUInt(baseColor_);
	float rgba[4] = { color.r, color.g, color.b, color.a };
	if (ImGui::ColorEdit4("Base Color", rgba)) {
		baseColor_ = ColorRGBA(rgba[0], rgba[1], rgba[2], rgba[3]).ToUInt();
	}

	ImGui::Text("=== Flip ===");
	ImGui::Checkbox("Flip X", &flipX_);
	ImGui::Checkbox("Flip Y", &flipY_);

	ImGui::Text("=== Animation ===");
	if (HasAnimation()) {
		ImGui::Text("Frame: %d / %d", GetCurrentFrame(), GetTotalFrames());
		ImGui::Text("Playing: %s", IsAnimationPlaying() ? "Yes" : "No");
	}
	else {
		ImGui::Text("No Animation");
	}

	ImGui::Text("=== Effects ===");
	ImGui::Text("Any Active: %s", IsAnyEffectActive() ? "Yes" : "No");
	ImGui::Text("Shake: %s", IsShakeActive() ? "Active" : "Inactive");
	ImGui::Text("Rotation: %s", IsRotationActive() ? "Active" : "Inactive");
	ImGui::Text("Fade: %s", IsFadeActive() ? "Active" : "Inactive");

	ImGui::End();
}
#endif