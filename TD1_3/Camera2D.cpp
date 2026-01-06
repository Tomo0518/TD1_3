#include "Camera2D.h"
#include "Affine2D.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <Novice.h>

// ========== コンストラクタ ==========
Camera2D::Camera2D(const Vector2& position, const Vector2& size, bool invertY)
	: position_(position)
	, size_(size)
	, zoom_(1.0f)
	, rotation_(0.0f)
	, invertY_(invertY) {

	UpdateMatrices();
}

// ========== 更新 ==========
void Camera2D::Update(float deltaTime) {
	// デバッグモード中は通常の更新処理をスキップ
	if (isDebugCamera_) {
		// デバッグモード中でもシェイク・ズーム・移動エフェクトは動作させる
		UpdateMoveEffect(deltaTime);
		UpdateZoomEffect(deltaTime);
		UpdateShakeEffect(deltaTime);

		// 行列を更新
		UpdateMatrices();
		return;  // ここで終了（追従と境界制限をスキップ）
	}

	// 通常モードの更新
	UpdateMoveEffect(deltaTime);
	UpdateZoomEffect(deltaTime);
	UpdateShakeEffect(deltaTime);
	UpdateFollow(deltaTime);
	ApplyBounds();
	UpdateMatrices();
}

// ========== 基本操作 ==========
void Camera2D::SetPosition(const Vector2& pos) {
	position_ = pos;
}

Vector2 Camera2D::GetPosition() const {
	return position_;
}

void Camera2D::SetZoom(float zoom) {
	zoom_ = std::clamp(zoom, 0.1f, 10.0f);
}

float Camera2D::GetZoom() const {
	return zoom_;
}

// ========== イージング移動 ==========
void Camera2D::MoveTo(const Vector2& targetPos, float duration,
	std::function<float(float)> easingFunc) {
	moveEffect_.isActive = true;
	moveEffect_.startPos = position_;
	moveEffect_.targetPos = targetPos;
	moveEffect_.duration = duration;
	moveEffect_.elapsed = 0.0f;
	moveEffect_.easingFunc = easingFunc;
}

void Camera2D::UpdateMoveEffect(float deltaTime) {
	if (!moveEffect_.isActive) return;

	moveEffect_.elapsed += deltaTime;
	float t = moveEffect_.elapsed / moveEffect_.duration;

	if (t >= 1.0f) {
		// 移動完了
		position_ = moveEffect_.targetPos;
		moveEffect_.isActive = false;
	}
	else {
		// イージング補間
		float easedT = moveEffect_.easingFunc(t);
		position_.x = moveEffect_.startPos.x +
			(moveEffect_.targetPos.x - moveEffect_.startPos.x) * easedT;
		position_.y = moveEffect_.startPos.y +
			(moveEffect_.targetPos.y - moveEffect_.startPos.y) * easedT;
	}
}

// ========== ズーム ==========
void Camera2D::ZoomTo(float targetZoom, float duration,
	std::function<float(float)> easingFunc) {
	zoomEffect_.isActive = true;
	zoomEffect_.startZoom = zoom_;
	zoomEffect_.targetZoom = std::clamp(targetZoom, 0.1f, 10.0f);
	zoomEffect_.duration = duration;
	zoomEffect_.elapsed = 0.0f;
	zoomEffect_.easingFunc = easingFunc;
}

void Camera2D::UpdateZoomEffect(float deltaTime) {
	if (!zoomEffect_.isActive) return;

	zoomEffect_.elapsed += deltaTime;
	float t = zoomEffect_.elapsed / zoomEffect_.duration;

	if (t >= 1.0f) {
		// ズーム完了
		zoom_ = zoomEffect_.targetZoom;
		zoomEffect_.isActive = false;
	}
	else {
		// イージング補間
		float easedT = zoomEffect_.easingFunc(t);
		zoom_ = zoomEffect_.startZoom +
			(zoomEffect_.targetZoom - zoomEffect_.startZoom) * easedT;
	}
}

// ========== シェイク ==========
void Camera2D::Shake(float intensity, float duration) {
	shakeEffect_.isActive = true;
	shakeEffect_.intensity = intensity;
	shakeEffect_.duration = duration;
	shakeEffect_.elapsed = 0.0f;
	shakeEffect_.continuous = false;
	shakeEffect_.offset = { 0.0f, 0.0f };
}

void Camera2D::ShakeContinuous(float intensity) {
	shakeEffect_.isActive = true;
	shakeEffect_.intensity = intensity;
	shakeEffect_.continuous = true;
	shakeEffect_.elapsed = 0.0f;
	shakeEffect_.offset = { 0.0f, 0.0f };
}

void Camera2D::StopShake() {
	shakeEffect_.isActive = false;
	shakeEffect_.offset = { 0.0f, 0.0f };
}

void Camera2D::UpdateShakeEffect(float deltaTime) {
	if (!shakeEffect_.isActive) return;

	// ランダムなオフセットを生成
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-shakeEffect_.intensity, shakeEffect_.intensity);

	shakeEffect_.offset.x = dist(gen);
	shakeEffect_.offset.y = dist(gen);

	// 時間制限のあるシェイクの場合
	if (!shakeEffect_.continuous) {
		shakeEffect_.elapsed += deltaTime;
		if (shakeEffect_.elapsed >= shakeEffect_.duration) {
			StopShake();
		}
	}
}

// ========== ターゲット追従 ==========
void Camera2D::SetTarget(const Vector2* target) {
	follow_.target = target;
}

void Camera2D::SetFollowSpeed(float speed) {
	follow_.speed = std::clamp(speed, 0.0f, 1.0f);
}

void Camera2D::SetDeadZone(float width, float height) {
	follow_.deadZoneWidth = width;
	follow_.deadZoneHeight = height;
}

void Camera2D::UpdateFollow(float deltaTime) {
	if (!follow_.target) return;

	Vector2 targetPos = *follow_.target;

	// デッドゾーンの処理
	if (follow_.deadZoneWidth > 0.0f || follow_.deadZoneHeight > 0.0f) {
		float dx = targetPos.x - position_.x;
		float dy = targetPos.y - position_.y;

		// X軸のデッドゾーン
		if (std::abs(dx) > follow_.deadZoneWidth * 0.5f) {
			float sign = (dx > 0.0f) ? 1.0f : -1.0f;
			targetPos.x = position_.x + sign * (std::abs(dx) - follow_.deadZoneWidth * 0.5f);
		}
		else {
			targetPos.x = position_.x;
		}

		// Y軸のデッドゾーン
		if (std::abs(dy) > follow_.deadZoneHeight * 0.5f) {
			float sign = (dy > 0.0f) ? 1.0f : -1.0f;
			targetPos.y = position_.y + sign * (std::abs(dy) - follow_.deadZoneHeight * 0.5f);
		}
		else {
			targetPos.y = position_.y;
		}
	}

	// 線形補間で追従（スムーズに追いかける）
	float lerpFactor = 1.0f - std::pow(1.0f - follow_.speed, deltaTime * 60.0f);
	position_.x += (targetPos.x - position_.x) * lerpFactor;
	position_.y += (targetPos.y - position_.y) * lerpFactor;
}

// ========== 境界制限 ==========
void Camera2D::SetBounds(float left, float top, float right, float bottom) {
	bounds_.enabled = true;
	bounds_.left = left;
	bounds_.top = top;
	bounds_.right = right;
	bounds_.bottom = bottom;
}

void Camera2D::ClearBounds() {
	bounds_.enabled = false;
}

void Camera2D::ApplyBounds() {
	if (!bounds_.enabled) return;

	// カメラの半分のサイズを計算（ズームを考慮）
	float halfWidth = (size_.x * 0.5f) / zoom_;
	float halfHeight = (size_.y * 0.5f) / zoom_;

	// 境界内にクランプ
	position_.x = std::clamp(position_.x,
		bounds_.left + halfWidth,
		bounds_.right - halfWidth);
	position_.y = std::clamp(position_.y,
		bounds_.bottom + halfHeight,
		bounds_.top - halfHeight);
}

// ========== 行列計算 ==========
// ========== 行列計算 ==========
void Camera2D::UpdateMatrices() {
	// シェイクオフセットを適用した位置
	Vector2 finalPosition = position_;
	if (shakeEffect_.isActive) {
		finalPosition.x += shakeEffect_.offset.x;
		finalPosition.y += shakeEffect_.offset.y;
	}

	// ビュー行列を作成（カメラのアフィン変換の逆行列）
	Vector2 scale = { zoom_, zoom_ };
	Matrix3x3 cameraAffine = AffineMatrix2D::MakeAffine(scale, rotation_, finalPosition);
	viewMatrix_ = Matrix3x3::Inverse(cameraAffine);

	// 射影行列（正射影）を作成 - Y軸反転オプションあり
	float halfWidth = size_.x * 0.5f;
	float halfHeight = size_.y * 0.5f;
	float yScale = invertY_ ? -1.0f : 1.0f;  // Y軸反転フラグに応じて変更

	projectionMatrix_.m[0][0] = 1.0f / halfWidth;
	projectionMatrix_.m[0][1] = 0.0f;
	projectionMatrix_.m[0][2] = 0.0f;

	projectionMatrix_.m[1][0] = 0.0f;
	projectionMatrix_.m[1][1] = yScale / halfHeight;
	projectionMatrix_.m[1][2] = 0.0f;

	projectionMatrix_.m[2][0] = 0.0f;
	projectionMatrix_.m[2][1] = 0.0f;
	projectionMatrix_.m[2][2] = 1.0f;

	// ビューポート行列を作成
	viewportMatrix_.m[0][0] = size_.x * 0.5f;
	viewportMatrix_.m[0][1] = 0.0f;
	viewportMatrix_.m[0][2] = 0.0f;

	viewportMatrix_.m[1][0] = 0.0f;
	viewportMatrix_.m[1][1] = size_.y * 0.5f;
	viewportMatrix_.m[1][2] = 0.0f;

	viewportMatrix_.m[2][0] = size_.x * 0.5f;
	viewportMatrix_.m[2][1] = size_.y * 0.5f;
	viewportMatrix_.m[2][2] = 1.0f;

	// View * Projection * Viewport 行列を合成
	Matrix3x3 vp = Matrix3x3::Multiply(viewMatrix_, projectionMatrix_);
	vpVpMatrix_ = Matrix3x3::Multiply(vp, viewportMatrix_);
}

Matrix3x3 Camera2D::GetVpVpMatrix() const {
	return vpVpMatrix_;
}

// ========== デバッグ用カメラ操作 ==========
void Camera2D::DebugMove(bool isDebug, const char* keys, const char* pre) {
	if (!isDebug) {
		isDebugCamera_ = false;
		return;
	}

	isDebugCamera_ = true;

	// デバッグモード中はターゲット追従を無効化
	const Vector2* originalTarget = follow_.target;
	follow_.target = nullptr;

	const float moveSpeed = 300.0f;      // 移動速度（ピクセル/秒）
	const float zoomSpeed = 0.5f;        // ズーム速度
	const float rotationSpeed = 1.0f;    // 回転速度（ラジアン/秒）

	// ========== 移動 ==========
	// 矢印キーで移動
	if (IsInvertY()) {
		if (keys[DIK_UP]) {
			position_.y += moveSpeed * (1.0f / 60.0f);  // 仮に60FPS想定
		}
		if (keys[DIK_DOWN]) {
			position_.y -= moveSpeed * (1.0f / 60.0f);
		}
		if (keys[DIK_LEFT]) {
			position_.x -= moveSpeed * (1.0f / 60.0f);
		}
		if (keys[DIK_RIGHT]) {
			position_.x += moveSpeed * (1.0f / 60.0f);
		}
	}
	else {
		if (keys[DIK_UP]) {
			position_.y -= moveSpeed * (1.0f / 60.0f);  // 仮に60FPS想定
		}
		if (keys[DIK_DOWN]) {
			position_.y += moveSpeed * (1.0f / 60.0f);
		}
		if (keys[DIK_LEFT]) {
			position_.x -= moveSpeed * (1.0f / 60.0f);
		}
		if (keys[DIK_RIGHT]) {
			position_.x += moveSpeed * (1.0f / 60.0f);
		}
	}


	// ========== ズーム ==========
	// PageUp/PageDown でズーム
	if (keys[DIK_E]) {  // PageUp
		zoom_ += zoomSpeed * (1.0f / 60.0f);
		zoom_ = std::clamp(zoom_, 0.1f, 10.0f);
	}
	if (keys[DIK_Q]) {  // PageDown
		zoom_ -= zoomSpeed * (1.0f / 60.0f);
		zoom_ = std::clamp(zoom_, 0.1f, 10.0f);
	}



	// ========== 回転 ==========
	// Q/E で回転
	if (keys[DIK_R]) {
		rotation_ -= rotationSpeed * (1.0f / 60.0f);
	}
	if (keys[DIK_T]) {
		rotation_ += rotationSpeed * (1.0f / 60.0f);
	}

	// ========== リセット ==========
	// Rキーでカメラリセット
	if (keys[DIK_F] && !pre[DIK_F]) {
		position_ = { 640.0f, 360.0f };
		zoom_ = 1.0f;
		rotation_ = 0.0f;
	}

	// ========== シェイクテスト ==========
	// Spaceキーでシェイク
	if (keys[DIK_SPACE] && !pre[DIK_SPACE]) {
		Shake(10.0f, 0.5f);
	}

	// ========== ズームテスト ==========
	// 1キーでズームイン
	if (keys[DIK_1] && !pre[DIK_1]) {
		ZoomTo(2.0f, 1.0f, Easing::EaseOutQuad);
	}
	// 2キーでズームアウト
	if (keys[DIK_2] && !pre[DIK_2]) {
		ZoomTo(1.0f, 1.0f, Easing::EaseInOutQuad);
	}

	// ========== 移動テスト ==========
	// 3キーで中央に移動
	if (keys[DIK_3] && !pre[DIK_3]) {
		MoveTo({ 640.0f, 360.0f }, 2.0f, Easing::EaseOutCubic);
	}

	// デバッグモード終了時にターゲット追従を復元
	if (!isDebug && originalTarget) {
		follow_.target = originalTarget;
	}
}