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

    // カメラの移動量を計算
    Vector2 cameraDelta = {
        cameraPos.x - initialCameraPosition_.x,
        cameraPos.y - initialCameraPosition_.y
    };

    // 背景のスクロールオフセット（X軸のみパララックス効果）
    float scrollOffsetX = cameraDelta.x * scrollSpeed_;

    // 繰り返し幅は固定
    float repeatWidth = repeatWidth_;

    // 画面左端を基準にした最初のタイルの位置
    float startX = -std::fmod(scrollOffsetX, repeatWidth);
    if (startX > 0.0f) {
        startX -= repeatWidth; // 画面左端より左から開始
    }

    // 必要なタイル数を計算（固定画面サイズベース）
    int maxTiles = static_cast<int>(std::ceil(1280.0f / repeatWidth)) + 2;

    // Y座標は固定（画面中央基準）
    float screenY = verticalOffset_ - 360.0f; // テクスチャ中心がverticalOffset_になるように

    // タイルを繰り返し描画
    for (int i = 0; i < maxTiles; ++i) {
        float screenX = startX + (i * repeatWidth);

        Novice::DrawSprite(
            static_cast<int>(screenX),
            static_cast<int>(screenY),
            textureHandle_,
            1.0f, 1.0f, 0.0f, 0xFFFFFFFF
        );
    }

#ifdef _DEBUG
    //Novice::ConsolePrintf("[Parallax] Camera:(%.1f, %.1f) Delta:(%.1f, %.1f) ScrollOffset:%.1f Tiles:%d\n",
    //    cameraPos.x, cameraPos.y, cameraDelta.x, cameraDelta.y, scrollOffsetX, maxTiles);
#endif
}