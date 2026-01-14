#pragma once
#include <Novice.h>
#include <map>
#include <string>
#include "MapData.h"
#include "Camera2D.h"
#include "TileRegistry.h"

// 前方宣言
class TextureManager;

class MapChip {
public:
    MapChip();
    ~MapChip();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="mapData">マップデータ</param>
    /// <param name="texManager">テクスチャマネージャー（ハンドル取得用）</param>
    void Initialize(MapData* mapData);

    // 全レイヤーの描画
    void Draw(Camera2D& camera);

    void SetMapData(MapData* mapData) { mapData_ = mapData; }

private:
    MapData* mapData_ = nullptr;

    // IDごとの画像ハンドルキャッシュ
    std::map<int, int> textureCache_;

    // テクスチャハンドルの準備
    void LoadTexturesFromManager();

    // オートタイル判定用
    bool IsSameTile(int myID, int tx, int ty,TileLayer layer) const;

    // 特定レイヤーだけを描画する
    void DrawLayer(Camera2D& camera, TileLayer layer);
};