#include "MapData.h"

MapData::MapData() {
    // デフォルトで空のマップを作っておく
    Reset(20, 15);
}

void MapData::Reset(int width, int height, float tileSize) {
    width_ = width;
    height_ = height;
    tileSize_ = tileSize;

    // 2次元配列をリサイズし、0で埋める
    tiles_.assign(height_, std::vector<int>(width_, 0));
    enemySpawns_.clear();
}

bool MapData::Load(const std::string& filePath) {
    json j;
    if (!JsonUtil::LoadFromFile(filePath, j)) {
        Novice::ConsolePrintf("[MapData] Failed to load: %s\n", filePath.c_str());
        return false;
    }

    try {
        // 基本情報の読み込み
        width_ = JsonUtil::GetValue<int>(j, "width", 20);
        height_ = JsonUtil::GetValue<int>(j, "height", 15);
        tileSize_ = JsonUtil::GetValue<float>(j, "tileSize", 64.0f);

        // 配列のリサイズ
        tiles_.assign(height_, std::vector<int>(width_, 0));

        // タイルデータの読み込み
        // JSON形式: "tiles": [ [0,0,1...], [0,1,0...] ] のような2次元配列を想定
        if (j.contains("tiles") && j["tiles"].is_array()) {
            auto& jsonRows = j["tiles"];
            for (int y = 0; y < height_ && y < (int)jsonRows.size(); ++y) {
                auto& jsonCols = jsonRows[y];
                if (jsonCols.is_array()) {
                    for (int x = 0; x < width_ && x < (int)jsonCols.size(); ++x) {
                        tiles_[y][x] = jsonCols[x].get<int>();
                    }
                }
            }
        }

        Novice::ConsolePrintf("[MapData] Loaded map: %dx%d\n", width_, height_);
        return true;

    }
    catch (const std::exception& e) {
        Novice::ConsolePrintf("[MapData] Parse error: %s\n", e.what());
        return false;
    }
}

bool MapData::Save(const std::string& filePath) {
    json j;

    // 基本データの設定
    JsonUtil::SetValue(j, "width", width_);
    JsonUtil::SetValue(j, "height", height_);
    JsonUtil::SetValue(j, "tileSize", tileSize_);

    // タイルデータをJSON配列に変換
    j["tiles"] = tiles_; // nlohmann::jsonはvector<vector<int>>をそのまま変換可能

    // ファイルへ保存
    return JsonUtil::SaveToFile(filePath, j);
}

int MapData::GetTile(int col, int row) const {
    // 範囲外チェック（超重要）
    // 配列の範囲外にアクセスするとゲームが落ちるため、必ずガードする
    if (row < 0 || row >= height_ || col < 0 || col >= width_) {
        return 0; // 範囲外は「空気(0)」あるいは「壁」として扱う
    }
    return tiles_[row][col];
}

void MapData::SetTile(int col, int row, int tileType) {
    // 範囲外なら何もしない（エディタでの誤操作防止）
    if (row < 0 || row >= height_ || col < 0 || col >= width_) {
        return;
    }
    tiles_[row][col] = tileType;
}