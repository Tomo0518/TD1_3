#include "ParallaxLayer.h"
#include <Novice.h>
#include <cmath>


ParallaxLayer::ParallaxLayer(TextureId textureId, float scrollSpeed, float repeatWidth)
    : textureHandle_(TextureManager::GetInstance().GetTexture(textureId))
    , scrollSpeed_(scrollSpeed)
    , repeatWidth_(repeatWidth)
    , verticalOffset_(360.0f)  // 画面中央(720 / 2)
    , initialCameraPosition_({ 0.0f, 0.0f }) {
}

void ParallaxLayer::Draw(Camera2D& camera) {
    Vector2 cameraPos = camera.GetPosition();
    float zoom = camera.GetZoom();

    // カメラの移動量を計算（初期位置からの差分）
    Vector2 cameraDelta = {
        cameraPos.x - initialCameraPosition_.x,
        cameraPos.y - initialCameraPosition_.y
    };

    // 背景のスクロールオフセット（X軸のみパララックス効果）
    float scrollOffsetX = cameraDelta.x * scrollSpeed_;

    // ズーム対応の繰り返し幅
    float zoomedRepeatWidth = repeatWidth_ * zoom;

    // 画面左端を基準にした最初のタイルの位置
    float startX = -std::fmod(scrollOffsetX, zoomedRepeatWidth);
    if (startX > 0.0f) {
        startX -= zoomedRepeatWidth; // 画面左端より左から開始
    }

    // 必要なタイル数を計算（ズームアウト時は多く必要）
    int maxTiles = static_cast<int>(std::ceil(1280.0f / zoomedRepeatWidth)) + 2;

    // Y座標は固定（画面中央基準）
    float screenY = verticalOffset_ - (360.0f * zoom); // テクスチャ中心がverticalOffset_になるように

    // タイルを繰り返し描画
    for (int i = 0; i < maxTiles; ++i) {
        float screenX = startX + (i * zoomedRepeatWidth);

        Novice::DrawSprite(
            static_cast<int>(screenX),
            static_cast<int>(screenY),
            textureHandle_,
            zoom, zoom, 0.0f, 0xFFFFFFFF
        );
    }

#ifdef _DEBUG
    Novice::ConsolePrintf("[Parallax] Camera:(%.1f, %.1f) Delta:(%.1f, %.1f) ScrollOffset:%.1f Zoom:%.2f Tiles:%d\n",
        cameraPos.x, cameraPos.y, cameraDelta.x, cameraDelta.y, scrollOffsetX, zoom, maxTiles);
#endif
}