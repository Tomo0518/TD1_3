#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "TipsCollection.h"
#include <memory>
#include <string>

/// <summary>
/// 個別のTipsカードUI
/// </summary>
class TipsCardUI {
public:
    TipsCardUI(int tipsId, const Vector2& position, const Vector2& size);
    ~TipsCardUI() = default;

    void Initialize();
    void Update(float deltaTime);

    // ロックアイコンだけ外部から受け取る
    void Draw(DrawComponent2D* lockIcon);

    void UpdateUnlockState();

    void SetSelected(bool selected) { isSelected_ = selected; }
    bool IsSelected() const { return isSelected_; }

    void SetPosition(const Vector2& pos) { position_ = pos; }
    Vector2 GetPosition() const { return position_; }
    Vector2 GetSize() const { return size_; }

    int GetTipsId() const { return tipsId_; }
    float GetCurrentScale() const { return currentScale_; }

private:
    int tipsId_;
    Vector2 position_;
    Vector2 size_;

    bool isUnlocked_;
    bool isSelected_;

    // 各カード固有の画像
    std::unique_ptr<DrawComponent2D> unlockedImage_;  // 解放済み画像
    std::unique_ptr<DrawComponent2D> shadowImage_;    // 影画像（Tips固有）

    // アニメーション用
    float currentScale_;
    float pulseTimer_;
};