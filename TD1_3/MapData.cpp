#include "MapData.h"

MapData::MapData() {
    Reset(kMapChipWidth, kMapChipHeight);
}

void MapData::Reset(int width, int height, float tileSize) {
    width_ = width;
    height_ = height;
    tileSize_ = tileSize;

    // 全てのレイヤーを0で初期化
    tilesBackground_.assign(height_, std::vector<int>(width_, 0));
	tilesBackgroundDecoration_.assign(height_, std::vector<int>(width_, 0));
    tilesDecoration_.assign(height_, std::vector<int>(width_, 0));
    tilesBlock_.assign(height_, std::vector<int>(width_, 0));

    // オブジェクトスポーン情報をクリア
    objectSpawns_.clear();
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

        // タイルレイヤー読み込み
        if (j.contains("layers")) {
            auto& layers = j["layers"];

            if (layers.contains("background")) {
                tilesBackground_ = layers["background"].get<std::vector<std::vector<int>>>();
            }
			if (layers.contains("backgroundDecoration")) {
                tilesBackgroundDecoration_ = layers["backgroundDecoration"].get<std::vector<std::vector<int>>>();
            }
            if (layers.contains("decoration")) {
                tilesDecoration_ = layers["decoration"].get<std::vector<std::vector<int>>>();
            }
            if (layers.contains("block")) {
                tilesBlock_ = layers["block"].get<std::vector<std::vector<int>>>();
            }
        }
        // 互換性: 古い形式
        else if (j.contains("tiles")) {
            tilesBlock_ = j["tiles"].get<std::vector<std::vector<int>>>();
        }

        // オブジェクトスポーン情報読み込み
        objectSpawns_.clear();
        if (j.contains("objects")) {
            for (auto& obj : j["objects"]) {
                ObjectSpawnInfo spawn;
                spawn.objectTypeId = obj["type"];
                spawn.position.x = obj["position"]["x"];
                spawn.position.y = obj["position"]["y"];
                spawn.tag = obj.value("tag", "");
                spawn.customData = obj.value("data", json::object());
                objectSpawns_.push_back(spawn);
            }
            Novice::ConsolePrintf("[MapData] Loaded %d object spawns\n", (int)objectSpawns_.size());
        }

        Novice::ConsolePrintf("[MapData] Loaded map: %dx%d\n", width_, height_);
        return true;
    }
    catch (const std::exception& e) {
        Novice::ConsolePrintf("[MapData] Parse error: %s\n", e.what());
        return false;
    }
}

#ifdef _DEBUG
bool MapData::Save(const std::string& filePath) {
    json j;
    JsonUtil::SetValue(j, "width", width_);
    JsonUtil::SetValue(j, "height", height_);
    JsonUtil::SetValue(j, "tileSize", tileSize_);

    // タイルレイヤーを保存
    j["layers"]["background"] = tilesBackground_;
	j["layers"]["backgroundDecoration"] = tilesBackgroundDecoration_;
    j["layers"]["decoration"] = tilesDecoration_;
    j["layers"]["block"] = tilesBlock_;

    // オブジェクトスポーン情報を保存
    json objectsArray = json::array();
    for (auto& spawn : objectSpawns_) {
        json obj;
        obj["type"] = spawn.objectTypeId;
        obj["position"]["x"] = spawn.position.x;
        obj["position"]["y"] = spawn.position.y;
        if (!spawn.tag.empty()) {
            obj["tag"] = spawn.tag;
        }
        if (!spawn.customData.empty()) {
            obj["data"] = spawn.customData;
        }
        objectsArray.push_back(obj);
    }
    j["objects"] = objectsArray;

    // コンパクトフォーマットで保存
    return JsonUtil::SaveMapCompact(filePath, j);
}
#endif

void MapData::AddObjectSpawn(int typeId, const Vector2& position, const std::string& tag, const json& customData) {
    ObjectSpawnInfo spawn;
    spawn.objectTypeId = typeId;
    spawn.position = position;
    spawn.tag = tag;
    spawn.customData = customData;
    objectSpawns_.push_back(spawn);
}

void MapData::RemoveObjectSpawn(size_t index) {
    if (index < objectSpawns_.size()) {
        objectSpawns_.erase(objectSpawns_.begin() + index);
    }
}

void MapData::UpdateObjectSpawnPosition(size_t index, const Vector2& newPosition) {
    if (index < objectSpawns_.size()) {
        objectSpawns_[index].position = newPosition;
    }
}


const std::vector<std::vector<int>>* MapData::GetLayerData(TileLayer layer) const {
    switch (layer) {
    case TileLayer::Background: return &tilesBackground_;
    case TileLayer::BackgroundDecoration: return &tilesBackgroundDecoration_;
    case TileLayer::Decoration: return &tilesDecoration_;
    case TileLayer::Block:      return &tilesBlock_;
    default: return nullptr;
    }
}

std::vector<std::vector<int>>* MapData::GetLayerDataMutable(TileLayer layer) {
    switch (layer) {
    case TileLayer::Background: return &tilesBackground_;
	case TileLayer::BackgroundDecoration: return &tilesBackgroundDecoration_;
    case TileLayer::Decoration: return &tilesDecoration_;
    case TileLayer::Block:      return &tilesBlock_;
    default: return nullptr;
    }
}

int MapData::GetTile(int col, int row, TileLayer layer) const {
    // 範囲チェック
    if (col < 0 || col >= width_ || row < 0 || row >= height_) {
        return 0;
    }

    // レイヤーごとに分岐
    switch (layer) {
    case TileLayer::Block:
        return tilesBlock_[row][col];
    case TileLayer::Decoration:
        return tilesDecoration_[row][col];
    case TileLayer::Background:
        return tilesBackground_[row][col];
    case TileLayer::BackgroundDecoration:
        return tilesBackgroundDecoration_[row][col];
    default:
        return 0;
    }
}

void MapData::SetTile(int col, int row, int tileID, TileLayer layer) {
    if (row < 0 || row >= height_ || col < 0 || col >= width_) return;

    auto* data = GetLayerDataMutable(layer);
    if (data && !data->empty()) {
        (*data)[row][col] = tileID;
    }
}