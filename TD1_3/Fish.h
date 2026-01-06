#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "Camera2D.h"
#include <memory>

class Fish {
public:
	// 初期化（初期位置を与える）
	void Initialize(const Vector2& position);

	// 更新（プレイヤー位置を参照して行動を変える）
	void Update(const Vector2& playerPos);

	// カメラを使って描画
	void Draw(const Camera2D& camera);

private:
	// 基本パラメータ
	Vector2 position_{ 0.0f, 0.0f };
	Vector2 velocity_{ 0.0f, 0.0f };
	float rotation_{ 0.0f };

	// 描画コンポーネント（存在するAPIのみ使用）
	std::unique_ptr<DrawComponent2D> draw_;

	// 定数
	static constexpr float kNormalSpeed = 1.5f;         // 通常速度
	static constexpr float kDashSpeed = 5.0f;           // 逃走速度
	static constexpr float kDetectionRadius = 150.0f;   // プレイヤー検知距離
	static constexpr float kWanderChangeRate = 0.02f;   // ランダム方向転換率

	// 乱数フォールバック
	static float RandomRange(float minVal, float maxVal);
};