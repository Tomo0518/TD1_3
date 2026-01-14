#include "MapData.h"

MapData::MapData() {
    // デフォルトで空のマップを作っておく
    Reset(1000, 1000);
}

void MapData::Reset(int width, int height, float tileSize) {
    width_ = width;
    height_ = height;
    tileSize_ = tileSize;

    // 全てのレイヤーを0で初期化
    tilesBackground_.assign(height_, std::vector<int>(width_, 0));
    tilesDecoration_.assign(height_, std::vector<int>(width_, 0));
    tilesBlock_.assign(height_, std::vector<int>(width_, 0));
    tilesObject_.assign(height_, std::vector<int>(width_, 0));

    enemySpawns_.clear();
}

bool MapData::Load(const std::string& filePath) {
    json j;
    if (!JsonUtil::LoadFromFile(filePath, j)) {
        Novice::ConsolePrintf("[MapData] Failed to load: %s\n", filePath.c_str());
        return false;
    }

    try {
        width_ = JsonUtil::GetValue<int>(j, "width", kMapChipWidth);
        height_ = JsonUtil::GetValue<int>(j, "height", kMapChipHeight);
        tileSize_ = JsonUtil::GetValue<float>(j, "tileSize", 64.0f);

        // 配列リセット（全レイヤーを初期化）
        Reset(width_, height_, tileSize_);

        // "layers" キーの下に各レイヤーデータがある想定
        if (j.contains("layers")) {
            auto& layers = j["layers"];

            // 各レイヤーを読み込み（存在すれば上書き、なければ0埋めのまま）
            if (layers.contains("background")) {
                tilesBackground_ = layers["background"].get<std::vector<std::vector<int>>>();
            }
            if (layers.contains("decoration")) {
                tilesDecoration_ = layers["decoration"].get<std::vector<std::vector<int>>>();
            }
            if (layers.contains("block")) {
                tilesBlock_ = layers["block"].get<std::vector<std::vector<int>>>();
            }
            if (layers.contains("object")) {
                tilesObject_ = layers["object"].get<std::vector<std::vector<int>>>();
            }
        }
        // 互換性: 古い形式（ルートに "tiles" がある場合）はBlockとして読む
        else if (j.contains("tiles")) {
            tilesBlock_ = j["tiles"].get<std::vector<std::vector<int>>>();
        }

        Novice::ConsolePrintf("[MapData] Loaded map (4 Layers): %dx%d\n", width_, height_);
        return true;
    }
    catch (const std::exception& e) {
        Novice::ConsolePrintf("[MapData] Parse error: %s\n", e.what());
        return false;
    }
}

bool MapData::Save(const std::string& filePath) {
    json j;
    JsonUtil::SetValue(j, "width", width_);
    JsonUtil::SetValue(j, "height", height_);
    JsonUtil::SetValue(j, "tileSize", tileSize_);

    // 各レイヤーを保存
    j["layers"]["background"] = tilesBackground_;
    j["layers"]["decoration"] = tilesDecoration_;
    j["layers"]["block"] = tilesBlock_;
    j["layers"]["object"] = tilesObject_;

    return JsonUtil::SaveToFile(filePath, j);
}

// 指定レイヤーの配列ポインタを取得するヘルパー
const std::vector<std::vector<int>>* MapData::GetLayerData(TileLayer layer) const {
    switch (layer) {
    case TileLayer::Background: return &tilesBackground_;
    case TileLayer::Decoration: return &tilesDecoration_;
    case TileLayer::Block:      return &tilesBlock_;
    case TileLayer::Object:     return &tilesObject_;
    default: return nullptr;
    }
}

std::vector<std::vector<int>>* MapData::GetLayerDataMutable(TileLayer layer) {
    switch (layer) {
    case TileLayer::Background: return &tilesBackground_;
    case TileLayer::Decoration: return &tilesDecoration_;
    case TileLayer::Block:      return &tilesBlock_;
    case TileLayer::Object:     return &tilesObject_;
    default: return nullptr;
    }
}

int MapData::GetTile(int col, int row, TileLayer layer) const {
    // 範囲外チェック
    if (row < 0 || row >= height_ || col < 0 || col >= width_) {
        return 0;
    }
    const auto* data = GetLayerData(layer);
    if (data && !data->empty()) {
        return (*data)[row][col];
    }
    return 0;
}

void MapData::SetTile(int col, int row, int tileID, TileLayer layer) {
    if (row < 0 || row >= height_ || col < 0 || col >= width_) return;

    auto* data = GetLayerDataMutable(layer);
    if (data && !data->empty()) {
        (*data)[row][col] = tileID;
    }
}