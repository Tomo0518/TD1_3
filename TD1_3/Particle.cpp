#include "Particle.h"
#include <cmath>
#include <algorithm>

#ifdef min
#undef min
#endif

void Particle::Initialize(
	const Vector2& pos,
	const Vector2& vel,
	const Vector2& acc,
	int life,
	int texHandle,
	float scaleStart,
	float scaleEnd,
	unsigned int colorStart,
	unsigned int colorEnd,
	float rotation,
	float rotationSpeed,
	BlendMode blendMode,
	float drawSize,
	bool useAnimation,
	int divX,
	int divY,
	int totalFrames,
	float animSpeed
) {
	// 基本パラメータ
	isAlive_ = true;
	position_ = pos;
	velocity_ = vel;
	acceleration_ = acc;
	lifeTimer_ = life;
	maxLife_ = life;
	textureHandle_ = texHandle;
	blendMode_ = blendMode;
	behavior_ = ParticleBehavior::Physics;

	// 回転
	rotation_ = rotation;
	rotationSpeed_ = rotationSpeed;

	// スケール
	scaleStart_ = scaleStart;
	scaleEnd_ = scaleEnd;
	currentScale_ = scaleStart;

	// 色
	colorStart_ = colorStart;
	colorEnd_ = colorEnd;
	currentColor_ = colorStart;

	// 描画サイズ
	drawSize_ = drawSize;

	// アニメーション
	useAnimation_ = useAnimation;
	divX_ = divX;
	divY_ = divY;
	totalFrames_ = totalFrames;
	animSpeed_ = animSpeed;
	animTimer_ = 0.0f;
	currentFrame_ = 0;
}

void Particle::Update(float deltaTime) {
	if (!isAlive_) return;

	// 1. 寿命を減算
	lifeTimer_--;
	if (lifeTimer_ <= 0) {
		isAlive_ = false;
		return;
	}

	// 2. 進行度を計算（0.0 ～ 1.0）
	float t = 1.0f - (static_cast<float>(lifeTimer_) / static_cast<float>(maxLife_));
	t = std::clamp(t, 0.0f, 1.0f);

	// 3. 線形補間でスケールと色を更新
	currentScale_ = Lerp(scaleStart_, scaleEnd_, t);
	currentColor_ = LerpColor(colorStart_, colorEnd_, t);

	// 4. アニメーションフレーム更新
	if (useAnimation_ && totalFrames_ > 1 && animSpeed_ > 0.0f) {
		animTimer_ += deltaTime;
		if (animTimer_ >= animSpeed_) {
			animTimer_ -= animSpeed_;
			currentFrame_++;
			if (currentFrame_ >= totalFrames_) {
				currentFrame_ = 0;  // ループしない場合は最終フレームで停止
			}
		}
	}

	// 5. 挙動タイプ別の物理演算
	if (behavior_ == ParticleBehavior::Physics) {
		// 通常の物理挙動
		velocity_.x += acceleration_.x * deltaTime;
		velocity_.y += acceleration_.y * deltaTime;
		position_.x += velocity_.x * deltaTime;
		position_.y += velocity_.y * deltaTime;
		rotation_ += rotationSpeed_ * deltaTime;
	}
	else if (behavior_ == ParticleBehavior::Homing) {
		// Homing 挙動
		if (homingTarget_ != nullptr) {
			// ターゲットへの方向ベクトルを計算
			Vector2 direction = {
				homingTarget_->x - position_.x,
				homingTarget_->y - position_.y
			};

			// 正規化
			float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
			if (length > 0.001f) {
				direction.x /= length;
				direction.y /= length;

				// ターゲット方向への加速度を追加
				velocity_.x += direction.x * homingStrength_ * deltaTime;
				velocity_.y += direction.y * homingStrength_ * deltaTime;
			}
		}

		// 加速度適用
		velocity_.x += acceleration_.x * deltaTime;
		velocity_.y += acceleration_.y * deltaTime;

		// 位置更新
		position_.x += velocity_.x * deltaTime;
		position_.y += velocity_.y * deltaTime;

		// 回転を速度方向に向ける
		if (velocity_.x != 0.0f || velocity_.y != 0.0f) {
			rotation_ = atan2f(velocity_.y, velocity_.x);
		}
	}
	else if (behavior_ == ParticleBehavior::Ghost) {
		// 残像：移動しない、フェードのみ
	}
	else if (behavior_ == ParticleBehavior::Stationary) {
		// その場に留まる
		rotation_ += rotationSpeed_ * deltaTime;
	}
}

void Particle::CheckGroundCollision(float groundY) {
	if (!isAlive_) return;

	// Yが+で上方向のシステムでは、地面より下 = Y < groundY
	if (position_.y <= groundY) {  // ★修正：<= に変更
		// 雨の場合：跳ね返りエフェクトを出して消える
		if (type_ == ParticleType::Rain) {
			// 速度を反転（跳ね返る）
			velocity_.y *= -0.3f;  // 反発係数0.3
			position_.y = groundY;  // 地面の位置に補正

			// 寿命を大幅に減らす（すぐ消える）
			lifeTimer_ = std::min(lifeTimer_, 10);
		}
		// 雪の場合：地面に着いたら消える
		else if (type_ == ParticleType::Snow) {
			isAlive_ = false;
		}
	}
}

float Particle::Lerp(float start, float end, float t) const {
	return start + (end - start) * t;
}

unsigned int Particle::LerpColor(unsigned int start, unsigned int end, float t) const {
	// RGBA各チャンネルを抽出
	unsigned int sR = (start >> 24) & 0xFF;
	unsigned int sG = (start >> 16) & 0xFF;
	unsigned int sB = (start >> 8) & 0xFF;
	unsigned int sA = start & 0xFF;

	unsigned int eR = (end >> 24) & 0xFF;
	unsigned int eG = (end >> 16) & 0xFF;
	unsigned int eB = (end >> 8) & 0xFF;
	unsigned int eA = end & 0xFF;

	// 補間
	unsigned int r = static_cast<unsigned int>(Lerp(static_cast<float>(sR), static_cast<float>(eR), t));
	unsigned int g = static_cast<unsigned int>(Lerp(static_cast<float>(sG), static_cast<float>(eG), t));
	unsigned int b = static_cast<unsigned int>(Lerp(static_cast<float>(sB), static_cast<float>(eB), t));
	unsigned int a = static_cast<unsigned int>(Lerp(static_cast<float>(sA), static_cast<float>(eA), t));

	// 結合して返す
	return (r << 24) | (g << 16) | (b << 8) | a;
}