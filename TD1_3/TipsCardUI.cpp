#include "TipsCardUI.h"
#include "TipsManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>

TipsCardUI::TipsCardUI(int tipsId, const Vector2& position, const Vector2& size)
    : tipsId_(tipsId)
    , position_(position)
    , size_(size)
    , isUnlocked_(false)
    , isSelected_(false)
    , currentScale_(1.0f)
    , pulseTimer_(0.0f)
{
}

void TipsCardUI::Initialize() {
    UpdateUnlockState();

    // Tips データを取得
    const TipsData* tipsData = TipsManager::GetInstance().GetCollection().GetTipsData(tipsId_);

    if (tipsData) {
        // 解放済み画像を設定
        unlockedImage_ = std::make_unique<DrawComponent2D>(tipsData->unlockedTextureId);
        unlockedImage_->SetDrawSize(size_);
        unlockedImage_->SetAnchorPoint({ 0.5f, 0.5f });

        // 影画像を設定（Tips固有）
        shadowImage_ = std::make_unique<DrawComponent2D>(tipsData->shadowTextureId);
        shadowImage_->SetDrawSize(size_);
        shadowImage_->SetAnchorPoint({ 0.5f, 0.5f });
    }
}

void TipsCardUI::Update(float deltaTime) {
    UpdateUnlockState();

    // パルスアニメーション（未解放時のみ）
    if (!isUnlocked_) {
        pulseTimer_ += deltaTime;
    }

    if (unlockedImage_) {
        unlockedImage_->Update(deltaTime);
    }
    if (shadowImage_) {
        shadowImage_->Update(deltaTime);
    }
}

void TipsCardUI::Draw(DrawComponent2D* lockIcon) {
    if (isUnlocked_) {
        // 解放済み：解放済み画像を表示
        if (unlockedImage_) {
            unlockedImage_->SetPosition(position_);
            unlockedImage_->SetScale(currentScale_, currentScale_);
            unlockedImage_->DrawScreen();
        }
    }
    else {
        // 未解放：影画像とロックアイコンを表示

        // パルスアニメーション計算
        float pulse = std::sin(pulseTimer_ * 3.14159f) * 0.5f + 0.5f;
        float alpha = 0.3f + pulse * 0.1f;

        // 影描画（Tips固有）
        if (shadowImage_) {
            shadowImage_->SetPosition(position_);
            shadowImage_->SetScale(currentScale_, currentScale_);

            unsigned int color = 0xFFFFFF00 | static_cast<unsigned int>(alpha * 255.0f);
            shadowImage_->SetBaseColor(color);

            shadowImage_->DrawScreen();
        }

        // ロックアイコン描画（共通）
        if (lockIcon) {
            lockIcon->SetPosition(position_);
            lockIcon->SetScale(currentScale_, currentScale_);
            lockIcon->SetDrawSize({ size_.x * 0.4f, size_.y * 0.4f });
            lockIcon->DrawScreen();
        }
    }
}

void TipsCardUI::UpdateUnlockState() {
    isUnlocked_ = TipsManager::GetInstance().GetCollection().IsTipsUnlocked(tipsId_);
}