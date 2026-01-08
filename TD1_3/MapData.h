#pragma once
#include <vector>
#include <string>
#include <Novice.h>
#include "Vector2.h"
#include "JsonUtil.h" // 既存のJsonUtilを使用

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

    // 指定座標のタイル番号を取得（範囲外は0を返す安全設計）
    int GetTile(int col, int row) const;

    // 指定座標のタイル番号を書き換える（エディタ用）
    void SetTile(int col, int row, int tileType);

    // マップの列数（幅）
    int GetWidth() const { return width_; }

    // マップの行数（高さ）
    int GetHeight() const { return height_; }

    // 1タイルのサイズ
    float GetTileSize() const { return tileSize_; }

private:
    // マップチップの番号を格納する2次元配列 [row][col]
    // vector<vector<int>> にすることでサイズを可変にする
    std::vector<std::vector<int>> tiles_;

    // マップのサイズ
    int width_ = 0;  // 列数 (x)
    int height_ = 0; // 行数 (y)
    float tileSize_ = 64.0f;

    // (拡張用) 敵の出現リストなど
    std::vector<EnemySpawnInfo> enemySpawns_;
};