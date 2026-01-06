#include "Fish.h"
#include "Random.h"
#include <Novice.h>
#include <cmath>
#include <cstdlib>

// 数学定数
static constexpr float PI = 3.14159265359f;

// Random::Range が未実装でも動くようにフォールバック
float Fish::RandomRange(float minVal, float maxVal) {
#ifdef USE_RANDOM_CLASS
	return Random::Range(minVal, maxVal);
#else
	float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	return minVal + (maxVal - minVal) * t;
#endif
}

void Fish::Initialize(const Vector2& position) {
	position_ = position;

	// ランダムな初期方向
	float randomAngle =
#ifdef USE_RANDOM_CLASS
		Random::Range(0.0f, 2.0f * PI);
#else
		RandomRange(0.0f, 2.0f * PI);
#endif
	Vector2 dir{ std::cos(randomAngle), std::sin(randomAngle) };
	velocity_ = Vector2::Multiply(kNormalSpeed, dir);

	rotation_ = 0.0f;

	// 画像ロードと描画設定
	int handle = Novice::LoadTexture("./Resources/images/gamePlay/koi.png");
	draw_ = std::make_unique<DrawComponent2D>(handle);
	draw_->SetDrawSize({ 32.0f, 16.0f });          // 横長の魚っぽいサイズ
	draw_->SetBaseColor(0xFF8080FF);               // RGBA（赤っぽい色）
	draw_->SetAnchorPoint({ 0.5f, 0.5f });         // 中心を原点に
	draw_->SetPosition(position_);
	draw_->SetRotation(rotation_);
}

void Fish::Update(const Vector2& playerPos) {
	// プレイヤーへのベクトルと距離
	Vector2 toPlayer = Vector2::Subtract(playerPos, position_);
	float distance = Vector2::Length(toPlayer);

	// 行動決定
	if (distance < kDetectionRadius) {
		// 逃走モード（逆方向へダッシュ）
		Vector2 fleeDir = Vector2::Normalize(toPlayer);
		fleeDir.x = -fleeDir.x;
		fleeDir.y = -fleeDir.y;
		velocity_ = Vector2::Multiply(kDashSpeed, fleeDir);
	}
	else {
		// 通常モード：たまに少し向きを変える
		float changeChance =
#ifdef USE_RANDOM_CLASS
			Random::Range(0.0f, 1.0f);
#else
			RandomRange(0.0f, 1.0f);
#endif
		if (changeChance < kWanderChangeRate) {
			float angleChange =
#ifdef USE_RANDOM_CLASS
				Random::Range(-0.5f, 0.5f);
#else
				RandomRange(-0.5f, 0.5f);
#endif
			float currentAngle = std::atan2(velocity_.y, velocity_.x);
			currentAngle += angleChange;
			Vector2 newDir{ std::cos(currentAngle), std::sin(currentAngle) };
			velocity_ = Vector2::Multiply(kNormalSpeed, newDir);
		}
		// 速度を通常速度に調整（逃走後の復帰など）
		float speed = Vector2::Length(velocity_);
		if (speed > kNormalSpeed) {
			Vector2 dir = Vector2::Normalize(velocity_);
			velocity_ = Vector2::Multiply(kNormalSpeed, dir);
		}
	}

	// 移動
	position_ = Vector2::Add(position_, velocity_);

	// 画面端で反射（1280x720前提）
	if (position_.x < 0.0f || position_.x > 1280.0f) {
		velocity_.x *= -1.0f;
		position_.x = (std::max)(0.0f, (std::min)(1280.0f, position_.x));
	}
	if (position_.y < 0.0f || position_.y > 720.0f) {
		velocity_.y *= -1.0f;
		position_.y = (std::max)(0.0f, (std::min)(720.0f, position_.y));
	}

	// 進行方向の角度
	rotation_ = std::atan2(velocity_.y, velocity_.x);

	// 描画コンポーネントへ反映（存在するAPIのみ）
	if (draw_) {
		draw_->SetPosition(position_);
		draw_->SetRotation(rotation_);
	}
}

void Fish::Draw(const Camera2D& camera) {
	if (draw_) {
		draw_->Draw(camera);
	}
}