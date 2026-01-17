#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include <functional>
#include <string>

/// <summary>
/// 個別のボタンクラス
/// </summary>
class Button {
public:
    Button(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback);
    Button(const Vector2& position, const Vector2& size, int normalTexture, int selectedTexture, std::function<void()> callback);

    void Update(float deltaTime, bool isSelected);
    void Draw();
    void Execute();

    bool IsPointInside(const Vector2& point) const;
    Vector2 GetPosition() const { return position_; }
    Vector2 GetSize() const { return size_; }

private:
    Vector2 position_;
    Vector2 size_;
    std::string label_;
    std::function<void()> callback_;

    bool isSelected_ = false;
    bool isImageButton_ = false;

    // 画像ボタン用
    int normalTexture_ = -1;
    int selectedTexture_ = -1;
    DrawComponent2D drawCompNormal_;
    DrawComponent2D drawCompSelected_;

    // 基本スケール（元画像サイズとボタンサイズの比率）
    float baseScaleX_ = 1.0f;
    float baseScaleY_ = 1.0f;

    // イージング用
    float easeT_ = 0.0f;
    float easeSpeed_ = 10.0f;
    float scaleMin_ = 0.9f;
    float scaleMax_ = 1.1f;

    // テキストボタン用
    uint32_t colorNormal_ = 0xFFFFFF88;
    uint32_t colorSelected_ = 0xFFFFFFFF;
    Vector2 anchor_ = { 0.5f, 0.5f };
};