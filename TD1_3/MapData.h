#pragma once
#include <vector>
#include <string>
#include <Novice.h>
#include "Vector2.h"
#include "JsonUtil.h"
#include "TileRegistry.h"

// オブジェクトスポーン情報
struct ObjectSpawnInfo {
    int objectTypeId;       // オブジェクトタイプID（100=Player, 101=Enemy等）
    Vector2 position;       // ワールド座標（自由配置）
    std::string tag;        // タグ（検索用、例: "player", "enemy"）
    json customData;        // カスタムパラメータ（向き、HP、AI設定等）
};

/// <summary>
/// マップの数値データとリソース情報を管理するクラス
/// 描画や当たり判定のロジックは持たない
/// </summary>
class MapData {
public:
    MapData();
    ~MapData() = default;

	// シングルトンインスタンス取得
	static MapData& GetInstance() {
		static MapData instance;
		return instance;
	}

    /// <summary>
    /// マップデータをリセット（指定サイズで0埋め）
    /// </summary>
    void Reset(int width, int height, float tileSize = 64.0f);

    /// <summary>
    /// JSONファイルからマップデータを読み込む
    /// </summary>
    bool Load(const std::string& filePath);

    /// <summary>
    /// 現在のマップデータをJSONファイルに保存する
    /// </summary>
#ifdef _DEBUG
    bool Save(const std::string& filePath);
#endif

    // --- タイルデータアクセサ ---
    int GetTile(int col, int row, TileLayer layer) const;
    void SetTile(int col, int row, int tileID, TileLayer layer);

    // 既存コード互換用（Blockレイヤーを返す）
    int GetTile(int col, int row) const {
        return GetTile(col, row, TileLayer::Block);
    }

    // --- オブジェクトスポーン情報アクセサ ---
    const std::vector<ObjectSpawnInfo>& GetObjectSpawns() const { return objectSpawns_; }

    /// <summary>
    /// オブジェクトスポーンを追加
    /// </summary>
    void AddObjectSpawn(int typeId, const Vector2& position, const std::string& tag = "", const json& customData = json::object());

    /// <summary>
    /// オブジェクトスポーンを削除（インデックス指定）
    /// </summary>
    void RemoveObjectSpawn(size_t index);


	/// <summary>
	/// オブジェクトスポーンの位置を更新（インデックス指定）
	/// </summary>
    void UpdateObjectSpawnPosition(size_t index, const Vector2& newPosition);

    /// <summary>
    /// オブジェクトスポーンを全削除
    /// </summary>
    void ClearObjectSpawns() { objectSpawns_.clear(); }

    // --- メタデータアクセサ ---
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    float GetTileSize() const { return tileSize_; }

    // レイヤーデータ取得
    std::vector<std::vector<int>>* GetLayerDataMutable(TileLayer layer);
    const std::vector<std::vector<int>>* GetLayerData(TileLayer layer) const;

private:
    // タイルレイヤー
    std::vector<std::vector<int>> tilesBackground_;
    std::vector<std::vector<int>> tilesBackgroundDecoration;
    std::vector<std::vector<int>> tilesDecoration_;
    std::vector<std::vector<int>> tilesBlock_;

    // オブジェクトスポーン情報（座標管理）
    std::vector<ObjectSpawnInfo> objectSpawns_;

    static const int kMapChipWidth = 1000;
    static const int kMapChipHeight = 1000;

    int width_ = 1000;
    int height_ = 1000;
    float tileSize_ = 64.0f;
};