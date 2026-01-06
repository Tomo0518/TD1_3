#include "Background.h"

// ========================================
// Background クラスの実装
// ========================================

Background::Background(int textureHandle, const Vector2& position) {
	// DrawComponent2D を作成（静止画）
	drawComp_ = std::make_unique<DrawComponent2D>(textureHandle);

	// 位置を設定
	drawComp_->SetPosition(position);

	// アンカーポイントを左上に設定（背景用）
	drawComp_->SetAnchorPoint({ 0.0f, 0.0f });
}

Background::Background(int textureHandle, int divX, int divY, int totalFrames,
	float speed, const Vector2& position, bool isLoop) {
	// DrawComponent2D を作成（アニメーション）
	drawComp_ = std::make_unique<DrawComponent2D>(
		textureHandle, divX, divY, totalFrames, speed, isLoop
	);

	// 位置を設定
	drawComp_->SetPosition(position);

	// アンカーポイントを左上に設定（背景用）
	drawComp_->SetAnchorPoint({ 0.0f, 0.0f });
}

void Background::Update(float deltaTime) {
	if (drawComp_) {
		drawComp_->Update(deltaTime);
	}
}

void Background::Draw(const Camera2D& camera) {
	if (!drawComp_) return;

	//// カリングチェック
	//if (cullingEnabled_ && !IsVisible(camera)) {
	//	return;
	//}

	// カメラを使って描画
	drawComp_->Draw(camera);
}

void Background::DrawScreen() {
	if (drawComp_) {
		drawComp_->DrawScreen();
	}
}

void Background::DrawWithOffset(const Vector2& offset, unsigned int color) {
	if (!drawComp_) return;

	// 一時的に色を変更
	unsigned int originalColor = drawComp_->GetBaseColor();
	drawComp_->SetBaseColor(color);

	// オフセットを適用した位置で描画
	Vector2 originalPos = drawComp_->GetPosition();
	drawComp_->SetPosition({ originalPos.x - offset.x, originalPos.y - offset.y });

	drawComp_->DrawWorld();

	// 元に戻す
	drawComp_->SetPosition(originalPos);
	drawComp_->SetBaseColor(originalColor);
}

void Background::SetPosition(const Vector2& pos) {
	if (drawComp_) {
		drawComp_->SetPosition(pos);
	}
}

Vector2 Background::GetPosition() const {
	return drawComp_ ? drawComp_->GetPosition() : Vector2{ 0.0f, 0.0f };
}

void Background::SetDrawSize(float width, float height) {
	if (drawComp_) {
		drawComp_->SetDrawSize(width, height);
	}
}

void Background::SetDrawSize(const Vector2& size) {
	SetDrawSize(size.x, size.y);
}

Vector2 Background::GetDrawSize() const {
	return drawComp_ ? drawComp_->GetDrawSize() : Vector2{ 0.0f, 0.0f };
}

void Background::SetScale(float x, float y) {
	if (drawComp_) {
		drawComp_->SetScale(x, y);
	}
}

void Background::SetScale(const Vector2& scale) {
	SetScale(scale.x, scale.y);
}

void Background::SetColor(unsigned int color) {
	if (drawComp_) {
		drawComp_->SetBaseColor(color);
	}
}

unsigned int Background::GetColor() const {
	return drawComp_ ? drawComp_->GetBaseColor() : 0xFFFFFFFF;
}

void Background::CalculateBounds(float& left, float& right, float& top, float& bottom) const {
	if (!drawComp_) return;

	Vector2 pos = drawComp_->GetPosition();
	Vector2 size = drawComp_->GetFinalDrawSize();

	left = pos.x;
	right = pos.x + size.x;
	top = pos.y;
	bottom = pos.y + size.y;
}

bool Background::IsVisible(const Camera2D& camera, float margin) const {
	if (!drawComp_) return false;

	// 背景の境界を計算
	float bgLeft, bgRight, bgTop, bgBottom;
	CalculateBounds(bgLeft, bgRight, bgTop, bgBottom);

	// カメラの視界範囲を計算
	Vector2 camPos = camera.GetPosition();
	float camZoom = camera.GetZoom();
	float camWidth = kWindowWidth / camZoom;
	float camHeight = kWindowHeight / camZoom;

	float camLeft = camPos.x - camWidth * 0.5f - margin;
	float camRight = camPos.x + camWidth * 0.5f + margin;
	float camTop = camPos.y - camHeight * 0.5f - margin;
	float camBottom = camPos.y + camHeight * 0.5f + margin;

	// AABB（軸平行境界ボックス）による交差判定
	bool horizontalOverlap = (bgLeft < camRight) && (bgRight > camLeft);
	bool verticalOverlap = (bgTop < camBottom) && (bgBottom > camTop);

	return horizontalOverlap && verticalOverlap;
}

void Background::StartPulse(float minScale, float maxScale, float speed) {
	if (drawComp_) {
		drawComp_->StartPulse(minScale, maxScale, speed, true);
	}
}

void Background::StartFadeIn(float duration) {
	if (drawComp_) {
		drawComp_->StartFadeIn(duration);
	}
}

void Background::StartFadeOut(float duration) {
	if (drawComp_) {
		drawComp_->StartFadeOut(duration);
	}
}

void Background::StartColorTransition(const ColorRGBA& targetColor, float duration) {
	if (drawComp_) {
		drawComp_->StartColorTransition(targetColor, duration);
	}
}

void Background::StopAllEffects() {
	if (drawComp_) {
		drawComp_->StopAllEffects();
	}
}

// ========================================
// BackgroundLayer クラスの実装
// ========================================

void BackgroundLayer::Initialize(int textureHandle, float tileWidth, float tileHeight, float parallaxFactor) {
	textureHandle_ = textureHandle;
	tileWidth_ = tileWidth;
	tileHeight_ = tileHeight;
	parallaxFactor_ = parallaxFactor;
	tiles_.clear();
}

void BackgroundLayer::AddTile(const Vector2& position) {
	auto tile = std::make_unique<Background>(textureHandle_, position);
	tile->SetDrawSize(tileWidth_, tileHeight_);
	tile->SetColor(color_);
	tiles_.push_back(std::move(tile));
}

void BackgroundLayer::AddTilesHorizontal(float startX, float y, int count) {
	for (int i = 0; i < count; ++i) {
		AddTile({ startX + i * tileWidth_, y });
	}
}

void BackgroundLayer::AddTilesVertical(float x, float startY, int count) {
	for (int i = 0; i < count; ++i) {
		AddTile({ x, startY + i * tileHeight_ });
	}
}

void BackgroundLayer::AddTilesGrid(float startX, float startY, int countX, int countY) {
	for (int y = 0; y < countY; ++y) {
		for (int x = 0; x < countX; ++x) {
			AddTile({ startX + x * tileWidth_, startY + y * tileHeight_ });
		}
	}
}

void BackgroundLayer::Update(float deltaTime) {
	for (auto& tile : tiles_) {
		tile->Update(deltaTime);
	}
}

void BackgroundLayer::Draw(const Camera2D& camera) {
	// パララックス効果を適用したカメラを作成
	Camera2D parallaxCamera = camera;
	Vector2 camPos = camera.GetPosition();

	// 視差係数に応じてカメラ位置を調整
	Vector2 parallaxPos = {
		camPos.x * parallaxFactor_,
		camPos.y * parallaxFactor_
	};
	parallaxCamera.SetPosition(parallaxPos);

	// 全タイルを描画
	for (auto& tile : tiles_) {
		tile->Draw(parallaxCamera);
	}
}

void BackgroundLayer::SetColor(unsigned int color) {
	color_ = color;
	for (auto& tile : tiles_) {
		tile->SetColor(color);
	}
}