#include "TipsManager.h"

TipsManager& TipsManager::GetInstance() {
    static TipsManager instance;
    return instance;
}

void TipsManager::Initialize() {
    // コレクションを初期化（毎回リセット）
    auto& collection = TipsCollection::GetInstance();
    collection.Initialize();

    // フラグとコールバックもクリア
    hasNewUnlock_ = false;
    unlockCallbacks_.clear();
}

void TipsManager::UnlockTips(int tipsId) {
    auto& collection = GetCollection();

    // 既に解放済みなら何もしない
    if (collection.IsTipsUnlocked(tipsId)) {
        return;
    }

    // Tips解放
    collection.UnlockTips(tipsId);
    hasNewUnlock_ = true;

    // 登録されているコールバックを全て実行
    for (auto& callback : unlockCallbacks_) {
        if (callback) {
            callback(tipsId);
        }
    }
}

void TipsManager::RegisterUnlockCallback(const TipsUnlockCallback& callback) {
    if (callback) {
        unlockCallbacks_.push_back(callback);
    }
}