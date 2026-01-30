#pragma once
#include <string>

// 個別のTips情報
struct TipsData {
    int id;                          // Tips ID (1-3)
    std::string title;               // タイトル
    std::string description;         // 説明文
    std::string iconTexturePath;     // アイコン画像パス
    bool isUnlocked;                 // 解放済みか

    TipsData()
        : id(0), title(""), description(""), iconTexturePath(""), isUnlocked(false) {
    }

    TipsData(int _id, const std::string& _title, const std::string& _desc,
        const std::string& _iconPath)
        : id(_id), title(_title), description(_desc),
        iconTexturePath(_iconPath), isUnlocked(false) {
    }
};