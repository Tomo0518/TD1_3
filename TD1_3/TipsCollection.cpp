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
        "←→キー: 移動\nShift: ダッシュ\nZ: ブーメラン投げ\nブーメランは自動で戻ってきます",
        "./Resources/images/tips/icons/tip_01_controls.png"
    ));

    // Tips 2: ブーメランジャンプ
    tipsDatabase_.push_back(TipsData(
        2,
        "ブーメランジャンプ",
        "空中でブーメランに乗ることができます\n投げたブーメランの上に乗って\n高い場所へ移動しましょう",
        "./Resources/images/tips/icons/tip_02_boomerang_jump.png"
    ));

    // Tips 3: ブーメランの高さ調整
    tipsDatabase_.push_back(TipsData(
        3,
        "ブーメランの高さ調整",
        "Zキーを押しながら↑↓キーで\nブーメランの投げる高さを調整できます\n高い敵や障害物を避けましょう",
        "./Resources/images/tips/icons/tip_03_boomerang_height.png"
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