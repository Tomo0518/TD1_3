#pragma once
#include "Vector2.h"
#include "Camera2D.h"
#include "TextureManager.h"

/// <summary>
/// パララックススクロール用の背景レイヤー（スクリーン座標ベース）
/// </summary>
class ParallaxLayer {
public:
    ParallaxLayer(TextureId textureId, float scrollSpeed, float repeatWidth = 1280.0f);

    void Draw(Camera2D& camera);

    void SetScrollSpeed(float speed) { scrollSpeed_ = speed; }
    void SetRepeatWidth(float width) { repeatWidth_ = width; }
    void SetVerticalPosition(float y) { verticalOffset_ = y; }
    void SetInitialCameraPosition(const Vector2& initialPos) { initialCameraPosition_ = initialPos; }

private:
    int textureHandle_;          // 背景画像
    float scrollSpeed_;          // スクロール速度（0.0 = 固定, 1.0 = カメラと同速）
    float repeatWidth_;          // 繰り返しの幅（テクスチャの幅）
    float verticalOffset_;       // 垂直方向のオフセット（スクリーン座標）
    Vector2 initialCameraPosition_; // 初期カメラ位置（移動量計算用）
};