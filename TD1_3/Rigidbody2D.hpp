#pragma once
#include "Vector2.h"
#include <algorithm> // min, max用
#include <cmath>     // pow用

/// <summary>
/// 物理挙動（速度・加速度・摩擦）計算クラス
/// 座標は持たず、移動量(Velocity)の計算のみ
/// </summary>
class Rigidbody2D {
public:
    // --- 物理パラメータ ---
    Vector2 velocity = { 0.0f, 0.0f };
    Vector2 acceleration = { 0.0f, 0.0f };

    // 減速率 (0.0f ~ 1.0f)
    // 1.0f = 減速なし、0.9f = 毎秒90%に減速
    Vector2 deceleration = { 0.0f, 0.0f };

    float maxSpeedX = 6.0f;
    float maxSpeedY = 25.0f;

    // --- 回転パラメータ ---
    float angularVelocity = 0.0f;
    float angularAcceleration = 0.0f;
    float angularDeceleration = 0.0f;
    float maxAngularSpeed = 0.4f;

    // ==========================
    // メソッド
	// ==========================
    // 初期化
    void Initialize() {
        velocity = { 0.0f, 0.0f };
        acceleration = { 0.0f, 0.0f };
        angularVelocity = 0.0f;
    }

    // 力を加える（瞬間的な加速）
    void AddForce(const Vector2& force) {
        acceleration += force;
    }

    // トルクを加える（回転力）
    void AddTorque(float torque) {
        angularAcceleration += torque;
    }

    // 更新処理：加速度から速度を計算する
    void Update(float deltaTime) {
        // 1. 速度の更新 (v = v0 + at)
        velocity += acceleration * deltaTime;

        // 2. 最大速度制限
        if (abs(velocity.x) > maxSpeedX) {
			velocity.x = (velocity.x > 0 ? 1 : -1) * maxSpeedX;
        }
        if (abs(velocity.y) > maxSpeedY) {
			velocity.y = (velocity.y > 0 ? 1 : -1) * maxSpeedY;
		}


        // 3. 角速度の更新
        angularVelocity += angularAcceleration * deltaTime;

        // 角速度制限
        angularVelocity = std::clamp(angularVelocity, -maxAngularSpeed, maxAngularSpeed);

        // 4. 減速（摩擦）処理
        // 加速度入力がない軸に対してのみ減速を適用する（慣性）
        if (acceleration.x == 0.0f && deceleration.x > 0.0f) {
            velocity.x *= std::pow(deceleration.x, deltaTime);
        }
        if (acceleration.y == 0.0f && deceleration.y > 0.0f) {
            velocity.y *= std::pow(deceleration.y, deltaTime);
        }
        if (angularAcceleration == 0.0f && angularDeceleration > 0.0f) {
            angularVelocity *= std::pow(angularDeceleration, deltaTime);
        }

        // 5. 加速度のリセット
        // AddForceは「そのフレームだけの力」として扱うためリセットする
        acceleration = { 0.0f, 0.0f };
        angularAcceleration = 0.0f;
    }

    // 現在の速度から、このフレームの移動量を計算して返す
    Vector2 GetMoveDelta(float deltaTime) const {
        return velocity * deltaTime;
    }

    // このフレームの回転量を計算して返す
    float GetRotationDelta(float deltaTime) const {
        return angularVelocity * deltaTime;
    }
};