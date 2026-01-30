#include "TipsUIDrawer.h"
#include "ParticleManager.h"
#include <algorithm>

TipsUIDrawer::TipsUIDrawer()
    : animState_(AnimState::Idle),
    animTimer_(0.0f),
    bounceOffset_(0.0f),
    glowIntensity_(0.0f),
    showNotification_(false),
    unreadCount_(0),
    callbackRegistered_(false) {

    // 画面左上に配置
    basePosition_ = { 80.0f, 80.0f };
    iconSize_ = { 64.0f, 64.0f };
}

void TipsUIDrawer::Initialize() {
    // 本のアイコン（静止画）
    iconDrawComponent_ = std::make_unique<DrawComponent2D>(
        TextureId::TipsBook  // TextureManagerに登録されているID
    );
    iconDrawComponent_->SetDrawSize(iconSize_);
    iconDrawComponent_->SetAnchorPoint({ 0.5f, 0.5f });

    // 光エフェクト用
    glowDrawComponent_ = std::make_unique<DrawComponent2D>(
        TextureId::TipsBookGlow  // TextureManagerに登録されているID
    );
    glowDrawComponent_->SetDrawSize({ iconSize_.x * 1.5f, iconSize_.y * 1.5f });
    glowDrawComponent_->SetAnchorPoint({ 0.5f, 0.5f });
    glowDrawComponent_->SetBaseColor(ColorRGBA(1.0f, 1.0f, 1.0f, 0.0f).ToUInt());

    // 通知バッジ
    badgeDrawComponent_ = std::make_unique<DrawComponent2D>(
        TextureId::TipsNotificationBadge  // TextureManagerに登録されているID
    );
    badgeDrawComponent_->SetDrawSize({ 24.0f, 24.0f });
    badgeDrawComponent_->SetAnchorPoint({ 0.5f, 0.5f });

    // Tips解放コールバック登録
    if (!callbackRegistered_) {
        TipsManager::GetInstance().RegisterUnlockCallback(
            [this](int tipsId) {
                OnTipsUnlocked(tipsId);
            }
        );
        callbackRegistered_ = true;
    }
}

void TipsUIDrawer::Update(float deltaTime) {
    UpdateAnimation(deltaTime);

    // DrawComponentの更新
    if (iconDrawComponent_) {
        iconDrawComponent_->Update(deltaTime);
    }
    if (glowDrawComponent_) {
        glowDrawComponent_->Update(deltaTime);
    }
    if (badgeDrawComponent_) {
        badgeDrawComponent_->Update(deltaTime);
    }

    // 新規解放があるか確認
    if (TipsManager::GetInstance().HasNewUnlock()) {
        showNotification_ = true;
    }
}

void TipsUIDrawer::Draw() {
    Vector2 drawPos = basePosition_;
    drawPos.y += bounceOffset_;

    // 光エフェクト（光っている時のみ）
    if (glowIntensity_ > 0.01f && glowDrawComponent_) {
        glowDrawComponent_->SetPosition(drawPos);

        // アルファ値を色として設定
        ColorRGBA glowColor(1.0f, 1.0f, 1.0f, glowIntensity_);
        glowDrawComponent_->SetBaseColor(glowColor.ToUInt());

        glowDrawComponent_->DrawScreen();  // UI用のスクリーン描画
    }

    // アイコン本体
    if (iconDrawComponent_) {
        iconDrawComponent_->SetPosition(drawPos);
        iconDrawComponent_->DrawScreen();  // UI用のスクリーン描画
    }

    // 通知バッジ
    if (showNotification_ && badgeDrawComponent_) {
        Vector2 badgePos = drawPos;
        badgePos.x += iconSize_.x * 0.4f;
        badgePos.y -= iconSize_.y * 0.4f;

        badgeDrawComponent_->SetPosition(badgePos);
        badgeDrawComponent_->DrawScreen();  // UI用のスクリーン描画
    }
}

void TipsUIDrawer::PlayUnlockReaction() {
    animState_ = AnimState::Bounce;
    animTimer_ = 0.0f;

    // パーティクルをアイコン位置に発生
    ParticleManager::GetInstance().Emit(
        ParticleType::Hit,  // 既存のパーティクルを使用
        basePosition_
    );

    // TODO: サウンド再生を追加予定
    // SoundManager::GetInstance().PlaySE("tips_notification");
}

void TipsUIDrawer::OnTipsUnlocked(int tipsId) {
	tipsId;  // 未使用（将来の拡張用）
    // Tips解放時の処理
    PlayUnlockReaction();
    unreadCount_++;
}

void TipsUIDrawer::UpdateAnimation(float deltaTime) {
    animTimer_ += deltaTime;

    switch (animState_) {
    case AnimState::Bounce:
        // バウンスアニメーション（0.5秒）
        if (animTimer_ < 0.5f) {
            float t = animTimer_ / 0.5f;
            bounceOffset_ = -20.0f * std::sin(t * 3.14159f);
        }
        else {
            bounceOffset_ = 0.0f;
            animState_ = AnimState::Glow;
            animTimer_ = 0.0f;
        }
        break;

    case AnimState::Glow:
        // 光るアニメーション（1.0秒）
        if (animTimer_ < 1.0f) {
            float t = animTimer_ / 1.0f;
            glowIntensity_ = 0.8f * std::sin(t * 3.14159f);
        }
        else {
            glowIntensity_ = 0.0f;
            animState_ = AnimState::Idle;
        }
        break;

    case AnimState::Idle:
        // 通常時：通知がある場合はゆっくり点滅
        if (showNotification_) {
            glowIntensity_ = 0.3f * (0.5f + 0.5f * std::sin(animTimer_ * 2.0f));
        }
        else {
            glowIntensity_ = 0.0f;
        }
        break;
    }
}

bool TipsUIDrawer::IsClicked(const Vector2& mousePos) const {
    // アイコンの矩形範囲内かチェック
    float halfSize = iconSize_.x * 0.5f;
    return (mousePos.x >= basePosition_.x - halfSize &&
        mousePos.x <= basePosition_.x + halfSize &&
        mousePos.y >= basePosition_.y - halfSize &&
        mousePos.y <= basePosition_.y + halfSize);
}