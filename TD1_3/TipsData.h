// 個別のTips情報
#pragma once
#include <string>
#include "TextureManager.h"

struct TipsData {
    int id;
    std::string title;
    std::string description;
    TextureId unlockedTextureId;  // 解放済み画像
    TextureId shadowTextureId;    // 未解放時の影画像（Tips固有）
    bool isUnlocked = false;

    TipsData(int id_, const std::string& title_, const std::string& desc_,
        TextureId unlockedTexId, TextureId shadowTexId)
        : id(id_)
        , title(title_)
        , description(desc_)
        , unlockedTextureId(unlockedTexId)
        , shadowTextureId(shadowTexId)
    {
    }
};