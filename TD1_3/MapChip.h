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

    void Draw( Camera2D& camera);

    void SetMapData(MapData* mapData) { mapData_ = mapData; }

private:
    MapData* mapData_ = nullptr;

    // IDごとの画像ハンドルキャッシュ
    // 描画のたびにTextureManagerにアクセスしても良いですが、
    // ここでキャッシュしておくと少し高速です。
    std::map<int, int> textureCache_;

    // テクスチャハンドルの準備
    void LoadTexturesFromManager();

    // オートタイル判定用
    bool IsSameTile(int myID, int tx, int ty) const;
};