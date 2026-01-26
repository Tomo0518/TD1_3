#include "ParallaxLayer.h"
#include <Novice.h>

ParallaxLayer::ParallaxLayer(TextureId textureId, float scrollSpeed, std::string layerName, float repeatWidth)
    : textureId_(textureId)
    , scrollSpeedX_(scrollSpeed)
    , scrollSpeedY_(0.0f)
    , repeatWidth_(repeatWidth)
    , repeatHeight_(0.0f)
    , layerName_(layerName)
    , initialCameraPos_{ 0.0f, 0.0f }
    , hasVerticalParallax_(false)
{
}

// Y軸視差対応版のコンストラクタ
ParallaxLayer::ParallaxLayer(TextureId textureId, float scrollSpeedX, float scrollSpeedY,
    std::string layerName, float repeatWidth, float repeatHeight)
    : textureId_(textureId)
    , scrollSpeedX_(scrollSpeedX)
    , scrollSpeedY_(scrollSpeedY)
    , repeatWidth_(repeatWidth)
    , repeatHeight_(repeatHeight)
    , layerName_(layerName)
    , initialCameraPos_{ 0.0f, 0.0f }
    , hasVerticalParallax_(true)
{
}

void ParallaxLayer::Draw(Camera2D& camera) {
    Vector2 cameraPos = camera.GetPosition();

    // カメラの移動量を計算
    float cameraDeltaX = cameraPos.x - initialCameraPos_.x;
    float cameraDeltaY = cameraPos.y - initialCameraPos_.y;

    // スクロール速度を適用
    float offsetX = cameraDeltaX * scrollSpeedX_;
    float offsetY = hasVerticalParallax_ ? -(cameraDeltaY * scrollSpeedY_) : 0.0f;

    // テクスチャ情報取得
    int textureHandle = TextureManager::GetInstance().GetTexture(textureId_);
    int textureWidth, textureHeight;
    Novice::GetTextureSize(textureHandle, &textureWidth, &textureHeight);

    // 画面サイズ
    const float screenWidth = 1280.0f;
    const float screenHeight = 720.0f;

    // ========== X方向の繰り返し描画（共通）==========
    float actualRepeatWidth = (repeatWidth_ > 0) ? repeatWidth_ : static_cast<float>(textureWidth);

    // 符号を正しく
    int startX = static_cast<int>(offsetX / actualRepeatWidth) - 1;
    int endX = static_cast<int>((offsetX + screenWidth) / actualRepeatWidth) + 1;

    // ========== Y方向の処理 ==========
    if (hasVerticalParallax_ && repeatHeight_ > 0.0f) {
        // Y軸視差あり + Y方向繰り返しあり：タイル状に描画
        float actualRepeatHeight = repeatHeight_;

        // Y方向も符号を正しく
        int startY = static_cast<int>(offsetY / actualRepeatHeight) - 1;
        int endY = static_cast<int>((offsetY + screenHeight) / actualRepeatHeight) + 1;

        for (int y = startY; y <= endY; ++y) {
            for (int x = startX; x <= endX; ++x) {
                float drawX = x * actualRepeatWidth - offsetX;
                float drawY = y * actualRepeatHeight - offsetY;

                Novice::DrawSprite(
                    static_cast<int>(drawX),
                    static_cast<int>(drawY),
                    textureHandle,
                    1.0f, 1.0f, 0.0f, 0xFFFFFFFF
                );
            }
        }
    }
    else if (hasVerticalParallax_) {
        // Y軸視差あり + Y方向繰り返しなし：X方向のみ繰り返し、Y方向はスクロール
        for (int x = startX; x <= endX; ++x) {
            float drawX = x * actualRepeatWidth - offsetX;
            float drawY = -offsetY; // Y方向はスクロールするが繰り返しなし

            Novice::DrawSprite(
                static_cast<int>(drawX),
                static_cast<int>(drawY),
                textureHandle,
                1.0f, 1.0f, 0.0f, 0xFFFFFFFF
            );
        }
    }
    else {
        // Y軸視差なし：X方向のみ繰り返し、Y方向は固定
        for (int x = startX; x <= endX; ++x) {
            float drawX = x * actualRepeatWidth - offsetX;
            float drawY = 0.0f; // Y方向は固定（カメラの動きに追従しない）

            Novice::DrawSprite(
                static_cast<int>(drawX),
                static_cast<int>(drawY),
                textureHandle,
                1.0f, 1.0f, 0.0f, 0xFFFFFFFF
            );
        }
    }
}

void ParallaxLayer::SetInitialCameraPosition(const Vector2& initialPos) {
    initialCameraPos_ = initialPos;
}