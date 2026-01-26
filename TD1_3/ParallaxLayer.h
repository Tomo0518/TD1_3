#pragma once
#include "Camera2D.h"
#include "TextureManager.h"
#include <string>

class ParallaxLayer {
public:
    ParallaxLayer(TextureId textureId, float scrollSpeed, std::string layerName, float repeatWidth);

    // Y軸視差対応版のコンストラクタ
    ParallaxLayer(TextureId textureId, float scrollSpeedX, float scrollSpeedY,
        std::string layerName, float repeatWidth, float repeatHeight);

    void Draw(Camera2D& camera);
    void SetInitialCameraPosition(const Vector2& initialPos);
    std::string GetLayerNameTag() const { return layerName_; }

private:
    TextureId textureId_;
    float scrollSpeedX_;       // X方向のスクロール速度
    float scrollSpeedY_ = 0.0f; // Y方向のスクロール速度（デフォルト0）
    float repeatWidth_;
    float repeatHeight_ = 0.0f; // Y方向の繰り返し高さ（0なら繰り返しなし）
    std::string layerName_;
    Vector2 initialCameraPos_;
    bool hasVerticalParallax_ = false; // Y軸視差の有効フラグ
};