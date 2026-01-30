#pragma once
#include "GameObject2D.h"
#include "DrawComponent2D.h"
#include "TipsManager.h"
#include "ParticleManager.h"
#include <memory>

class TipsTrigger : public GameObject2D {
public:
    TipsTrigger(int id,const std::string name, const Vector2& position, int tipsId)
        : GameObject2D(id,name),
        tipsId_(tipsId),
        isTriggered_(false),
        player_(nullptr) {
		this->SetPosition(position);
        // 当たり判定設定
        collider_.size = { 64.0f, 64.0f };

#ifdef _DEBUG
        // デバッグ用の描画（開発中のみ表示）
        //auto drawComp = std::make_unique<DrawComponent2D>(this, 1);
        //// 仮のテクスチャパス（後でデバッグ用画像に差し替え可能）
        //drawComp->SetTexture("./Resources/images/debug/tips_trigger_debug.png");
        //drawComp->SetSize(size_);
        //drawComp->SetAlpha(0.5f);  // 半透明
        //AddDrawComponent(std::move(drawComp));
#endif
    }

    void Update(float deltaTime) override {
		deltaTime; // 未使用
        if (isTriggered_) return;

        // プレイヤーとの衝突判定
        if (player_ && IsCollidingWith(player_)) {
            OnTrigger();
        }
    }

    // プレイヤーへの参照を設定（GamePlaySceneから呼ばれる）
    void SetPlayer(GameObject2D* player) {
        player_ = player;
    }

    int GetTipsId() const { return tipsId_; }
    bool IsTriggered() const { return isTriggered_; }

private:
    void OnTrigger() {
        if (isTriggered_) return;

        isTriggered_ = true;

        // Tipsを解放
        TipsManager::GetInstance().UnlockTips(tipsId_);

        // エフェクト再生
        PlayUnlockEffect();

        // このオブジェクトを非表示化（削除はしない）

    }

    void PlayUnlockEffect() {
        ParticleManager::GetInstance().Emit(ParticleType::Hit, transform_.translate);
    }

    bool IsCollidingWith(GameObject2D* other) {
        if (!other) return false;

        Vector2 otherPos = other->GetPosition();
        Vector2 otherSize = other->GetCollider().size;

        // 矩形の衝突判定（AABB）
        return (transform_.translate.x < otherPos.x + otherSize.x &&
            transform_.translate.x + collider_.size.x > otherPos.x &&
            transform_.translate.y < otherPos.y + otherSize.y &&
            transform_.translate.y + collider_.size.y > otherPos.y);
    }

    int tipsId_;
    bool isTriggered_;
    GameObject2D* player_;  // プレイヤーへの参照（所有しない）
};