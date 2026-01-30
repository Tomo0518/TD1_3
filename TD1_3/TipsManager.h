#pragma once
#include "TipsCollection.h"
#include <functional>
#include <vector>

// Tips解放時のコールバック型
using TipsUnlockCallback = std::function<void(int tipsId)>;

// Tipsマネージャー（シングルトン）
class TipsManager {
public:
    static TipsManager& GetInstance();

    // 初期化（起動時に毎回リセット）
    void Initialize();

    // Tips解放
    void UnlockTips(int tipsId);

    // 新規解放があったかチェック
    bool HasNewUnlock() const { return hasNewUnlock_; }
    void ClearNewUnlockFlag() { hasNewUnlock_ = false; }

    // コールバック登録（UIが解放通知を受け取るため）
    void RegisterUnlockCallback(const TipsUnlockCallback& callback);

    // コレクションへのアクセス
    TipsCollection& GetCollection() { return TipsCollection::GetInstance(); }

private:
    TipsManager() = default;
    ~TipsManager() = default;
    TipsManager(const TipsManager&) = delete;
    TipsManager& operator=(const TipsManager&) = delete;

    bool hasNewUnlock_ = false;
    std::vector<TipsUnlockCallback> unlockCallbacks_;
};