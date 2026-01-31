#pragma once
#include "Vector2.h"
#include <vector>
#include <memory>
#include "DrawComponent2D.h"
#include "Boomerang.hpp" // ブーメランの状態を見るために必要

/// <summary>
/// 物理バネ挙動を持つ座標計算用ヘルパー
/// </summary>
struct SpringVector2 {
    Vector2 position;     // 現在位置
    Vector2 velocity;     // 現在の速度
    Vector2 target;       // 目標位置

    // 手触りを決めるパラメータ
    float stiffness = 120.0f; // バネの硬さ（高いとキビキビ動く）
    float damping = 10.0f;    // 減衰（高いと振動せずスッと止まる）
    float mass = 1.0f;        // 重さ

    void Update(float dt) {
        float usedt = dt;
        Vector2 force;
        force.x = -stiffness * (position.x - target.x);
        force.y = -stiffness * (position.y - target.y);

        Vector2 dampingForce;
        dampingForce.x = -damping * velocity.x;
        dampingForce.y = -damping * velocity.y;

        Vector2 acceleration;
        acceleration.x = (force.x + dampingForce.x) / mass;
        acceleration.y = (force.y + dampingForce.y) / mass;

        velocity.x += acceleration.x * usedt;
        velocity.y += acceleration.y * usedt;

        position.x += velocity.x * usedt;
        position.y += velocity.y * usedt;
    }
};

/// <summary>
/// プレイヤーやブーメランの周囲を回る星（ビット）を管理するクラス
/// </summary>
class OrbitSystem {
public:
    OrbitSystem();
    ~OrbitSystem() = default;

    // 初期化
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="deltaTime">デルタタイム</param>
    /// <param name="playerPos">プレイヤーの中心座標</param>
    /// <param name="starCount">現在の星の所持数</param>
    /// <param name="boomerangs">ブーメランのリスト（追従先判定用）</param>
    void Update(float deltaTime, const Vector2& playerPos, int starCount, const std::vector<Boomerang*>& boomerangs);

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw(const Camera2D& camera);

private:
    // --- メンバ変数 ---

    // バネ制御される中心点（ここを中心に星が回る）
    SpringVector2 nucleus_;

    // 星の回転制御
    float rotationAngle_ = 0.0f;
    float rotationSpeed_ = 3.0f;
    float orbitRadius_ = 60.0f; // 回転半径

    // 描画用
    int currentDrawCount_ = 0; // 現在表示すべき星の数
    std::unique_ptr<DrawComponent2D> starDrawer_; // 星の描画コンポーネント

    // ターゲット状態管理（補間用）
    bool isTrackingBoomerang_ = false;
};