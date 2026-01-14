#pragma once
#include <vector>
#include <string>
#include <Novice.h>
#include "Vector2.h"
#include "JsonUtil.h" // 既存のJsonUtilを使用
#include "TileRegistry.h" // TileLayer定義のためインクルード

// 将来的に敵の出現位置なども管理するための構造体
struct EnemySpawnInfo {
    int enemyType;
    Vector2 position;
};

// MapDataはほかのクラスを知らない

/// <summary>
/// マップの数値データとリソース情報を管理するクラス
/// 描画や当たり判定のロジックは持たない
/// </summary>
class MapData {
public:
    // コンストラクタ
    MapData();

    // デストラクタ
    ~MapData() = default;

    /// <summary>
    /// マップデータをリセット（指定サイズで0埋め）
    /// </summary>
    void Reset(int width, int height, float tileSize = 64.0f);

    /// <summary>
    /// JSONファイルからマップデータを読み込む
    /// </summary>
    /// <param name="filePath">ファイルパス</param>
    /// <returns>成功したらtrue</returns>
    bool Load(const std::string& filePath);

    /// <summary>
    /// 現在のマップデータをJSONファイルに保存する
    /// </summary>
    /// <param name="filePath">ファイルパス</param>
    /// <returns>成功したらtrue</returns>
    bool Save(const std::string& filePath);

    // --- アクセサ（データの取得・設定） ---

    // レイヤーを指定して取得
    int GetTile(int col, int row, TileLayer layer) const;

    // レイヤーを指定して設定
    void SetTile(int col, int row, int tileID, TileLayer layer);

    // 既存コード互換用（Blockレイヤーを返す）
    // これがあればPhysicsManagerなどは書き換えなくて済む
    int GetTile(int col, int row) const {
        return GetTile(col, row, TileLayer::Block);
    }

    // マップの列数（幅）
    int GetWidth() const { return width_; }

    // マップの行数（高さ）
    int GetHeight() const { return height_; }

    // 1タイルのサイズ
    float GetTileSize() const { return tileSize_; }

	// 指定レイヤーの配列ポインタを取得するヘルパー
    std::vector<std::vector<int>>* GetLayerDataMutable(TileLayer layer);

	// 指定レイヤーの配列ポインタを取得するヘルパー（const版）
	const std::vector<std::vector<int>>* GetLayerData(TileLayer layer) const;


private:
    // ==============================
    // レイヤー情報
    // ==============================
    std::vector<std::vector<int>> tilesBackground_; // 背景レイヤー
    std::vector<std::vector<int>> tilesDecoration_; // 装飾レイヤー
    std::vector<std::vector<int>> tilesBlock_; // ブロックレイヤー
    std::vector<std::vector<int>> tilesObject_;    // オブジェクトレイヤー

   // std::vector<std::vector<int>> tilesBlock_;

	static const int kMapChipWidth = 1000;
	static const int kMapChipHeight = 1000;

    // マップのサイズ
    int width_ = 1000;  // 列数 (x)
    int height_ = 1000; // 行数 (y)
    float tileSize_ = 64.0f;

    // (拡張用) 敵の出現リストなど
    std::vector<EnemySpawnInfo> enemySpawns_;
};