#include "TipsCollection.h"

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
        "./Resources/images/tips/icons/tip_01_controls.png"
    ));

    // Tips 2: ブーメランジャンプ
    tipsDatabase_.push_back(TipsData(
        2,
        "ブーメランジャンプ",
        "空中でブーメランに乗ることができます\n投げたブーメランの上に乗って\n高い場所へ移動できる",
        "./Resources/images/tips/icons/tip_02_boomerang_jump.png"
    ));

    // Tips 3: ブーメランの高さ調整
    tipsDatabase_.push_back(TipsData(
        3,
        "ブーメランの高さ調整",
        "なげ　",
        "ブーメランは投げた後少しの間プレイヤーの高さについてくる"
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