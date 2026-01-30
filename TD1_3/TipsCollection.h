#pragma once
#include "TipsData.h"
#include <vector>
#include <memory>

// Tipsコレクション管理クラス（シングルトン）
class TipsCollection {
public:
    static TipsCollection& GetInstance();

    // 初期化（起動時に毎回リセット）
    void Initialize();

    // Tips操作
    void UnlockTips(int tipsId);
    bool IsTipsUnlocked(int tipsId) const;
    const TipsData* GetTipsData(int tipsId) const;
    std::vector<TipsData> GetAllTips() const;
    int GetUnlockedCount() const;
    int GetTotalCount() const { return static_cast<int>(tipsDatabase_.size()); }

private:
    TipsCollection() = default;
    ~TipsCollection() = default;
    TipsCollection(const TipsCollection&) = delete;
    TipsCollection& operator=(const TipsCollection&) = delete;

    void LoadTipsDefinitions();  // 3つのTipsを定義

    std::vector<TipsData> tipsDatabase_;
};