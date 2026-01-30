#pragma once
#include "DrawComponent2D.h"
#include "TipsManager.h"
#include <memory>
#include <cmath>

// Tips本のアイコンUI描画クラス
class TipsUIDrawer {
public:
    TipsUIDrawer();
    ~TipsUIDrawer() = default;
    
    void Initialize();
    void Update(float deltaTime);
    void Draw();
    
    // 新しいTips解放時のアニメーション
    void PlayUnlockReaction();
    
    // クリック判定（後でTips本を開く用）
    bool IsClicked(const Vector2& mousePos) const;
    
    // 通知バッジのクリア（Tips本を開いた時に呼ぶ）
    void ClearNotification() {
        showNotification_ = false;
        unreadCount_ = 0;
    }
    
private:
    void UpdateAnimation(float deltaTime);
    void OnTipsUnlocked(int tipsId);
    
    // アニメーション状態
    enum class AnimState {
        Idle,
        Bounce,      // 跳ねるアニメーション
        Glow         // 光るアニメーション
    };
    
    AnimState animState_;
    float animTimer_;
    float bounceOffset_;
    float glowIntensity_;
    
    // 通知バッジ
    bool showNotification_;
    int unreadCount_;
    
    // 描画設定
    Vector2 basePosition_;
    Vector2 iconSize_;
    
    // DrawComponent（スマートポインタで管理）
    std::unique_ptr<DrawComponent2D> iconDrawComponent_;
    std::unique_ptr<DrawComponent2D> glowDrawComponent_;
    std::unique_ptr<DrawComponent2D> badgeDrawComponent_;
    
    // コールバックID（必要に応じて）
    bool callbackRegistered_;
};