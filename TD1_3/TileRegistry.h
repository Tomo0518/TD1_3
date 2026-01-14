#pragma once
#include <string>
#include <vector>
#include "TextureManager.h" // TextureId定義のためインクルード

// タイルの種類
enum class TileType {
    None,       // 空気
    Solid,      // 通常ブロック
    AutoTile    // オートタイル
};

struct TileDefinition {
    int id;                 // ID
    std::string name;       // エディタ表示名
    TextureId textureId;    // ★変更: TextureManagerで使うID
    TileType type;          // 種類
    bool isSolid;           // 当たり判定
};

class TileRegistry {
public:
    static const std::vector<TileDefinition>& GetAllTiles() { return tiles_; }

    static const TileDefinition* GetTile(int id) {
        for (const auto& tile : tiles_) {
            if (tile.id == id) return &tile;
        }
        return nullptr;
    }

    // 初期化：ここで TextureId を指定して登録
    static void Initialize() {
        tiles_.clear();

        // ID:0 空気 (テクスチャなしなので適当なIDか、None用ID)
        // TextureId::None があると仮定、なければ適当なものを
        tiles_.push_back({ 0, "Air", TextureId::Count, TileType::None, false });

        // ID:1 地面 (オートタイル)
        // ※ TextureManager側で TextureId::GroundAuto に "ground_auto.png" をロードしておく必要があります
        tiles_.push_back({
            1, "Ground",
            TextureId::GroundAuto, // ここを実際のTextureIdに合わせて変更してください
            TileType::AutoTile,
            true
            });

        // ID:2 鉄ブロック
        tiles_.push_back({
            2, "Iron",
            TextureId::GroundAuto,
            TileType::Solid,
            true
            });
    }

private:
    static std::vector<TileDefinition> tiles_;
};

inline std::vector<TileDefinition> TileRegistry::tiles_;