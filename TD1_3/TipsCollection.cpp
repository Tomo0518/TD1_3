#include "TipsCollection.h"
#include "TextureManager.h"

TipsCollection& TipsCollection::GetInstance() {
    static TipsCollection instance;
    return instance;
}

void TipsCollection::Initialize() {
    tipsDatabase_.clear();
    LoadTipsDefinitions();
}

void TipsCollection::LoadTipsDefinitions() {
    // Tips 1: 操作方法
    tipsDatabase_.push_back(TipsData(
        1,
        "操作方法",
        "移動キーとジャンプ、ブーメランの操作方法",
        TextureId::Tip_01_Controls,        // 解放済み画像
        TextureId::Tip_01_Controls_Shadow  // 影画像
    ));

    // Tips 2: ブーメランの高さ調整
    tipsDatabase_.push_back(TipsData(
        2,
        "ブーメランの高さ調整",
        "ブーメランは投げた後少しの間プレイヤーの高さについてくる",
        TextureId::Tip_02_Height_Control,
        TextureId::Tip_02_Height_Control_Shadow
    ));

    // Tips 3: ブーメランジャンプ
    tipsDatabase_.push_back(TipsData(
        3,
        "ブーメランジャンプ",
        "空中でブーメランに乗ることができます\n投げたブーメランの上に乗って\n高い場所へ移動できる",
        TextureId::Tip_03_Boomerang_Jump,
        TextureId::Tip_03_Boomerang_Jump_Shadow
    ));

    // Tips 4: ブーメランは帰ってくるときにダメージが発生する
    tipsDatabase_.push_back(TipsData(
        4,
        "ブーメランは帰ってくるときにダメージが発生する",
        "ブーメランは帰ってくるときに敵に当たるとダメージを与えます",
        TextureId::Tip_04_Boomerang_Return_Damage,
        TextureId::Tip_04_Boomerang_Return_Damage_Shadow
    ));
}

void TipsCollection::UnlockTips(int tipsId) {
    for (auto& tips : tipsDatabase_) {
        if (tips.id == tipsId) {
            tips.isUnlocked = true;
            return;
        }
    }
}

bool TipsCollection::IsTipsUnlocked(int tipsId) const {
    for (const auto& tips : tipsDatabase_) {
        if (tips.id == tipsId) {
            return tips.isUnlocked;
        }
    }
    return false;
}

const TipsData* TipsCollection::GetTipsData(int tipsId) const {
    for (const auto& tips : tipsDatabase_) {
        if (tips.id == tipsId) {
            return &tips;
        }
    }
    return nullptr;
}

std::vector<TipsData> TipsCollection::GetAllTips() const {
    return tipsDatabase_;
}

int TipsCollection::GetUnlockedCount() const {
    int count = 0;
    for (const auto& tips : tipsDatabase_) {
        if (tips.isUnlocked) {
            count++;
        }
    }
    return count;
}