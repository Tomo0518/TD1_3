#pragma once
#include "GameObject2D.h"
#include "Vector2.h"

/// <summary>
/// ワールド座標系の原点を示すオブジェクト
/// このオブジェクトの位置が論理座標の(0,0)となる
/// </summary>
class WorldOrigin : public GameObject2D {
public:
    WorldOrigin(const std::string& name = "WorldOrigin")
        : GameObject2D(0/*ワールドオリジン*/, name) {
    }

    void Update(float dt) override {
        // 原点は移動しない（または特殊な条件で移動可能）
        dt;
    }

    void Draw(Camera2D& camera) {
        camera;
#ifdef _DEBUG
        // デバッグ表示：原点マーカー
        Vector2 screenPos = camera.WorldToScreen(transform_.translate);
        Novice::DrawBox(
            static_cast<int>(screenPos.x - 10),
            static_cast<int>(screenPos.y - 10),
            20, 20, 0.0f, 0xFF0000FF, kFillModeWireFrame
        );
        Novice::DrawLine(
            static_cast<int>(screenPos.x - 20), static_cast<int>(screenPos.y),
            static_cast<int>(screenPos.x + 20), static_cast<int>(screenPos.y),
            0xFF0000FF
        );
        Novice::DrawLine(
            static_cast<int>(screenPos.x), static_cast<int>(screenPos.y - 20),
            static_cast<int>(screenPos.x), static_cast<int>(screenPos.y + 20),d
            0xFF0000FF
        );
#endif
    }

    // ワールド座標をこの原点からの相対座標に変換
    Vector2 WorldToRelative(const Vector2& worldPos) const {
        return worldPos - transform_.translate;
    }

    // 相対座標をワールド座標に変換
    Vector2 RelativeToWorld(const Vector2& relativePos) const {
        return relativePos + transform_.translate;
    }
};