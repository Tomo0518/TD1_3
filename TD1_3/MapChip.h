#pragma once
#include <Novice.h>
#include <map>
#include <string>
#include "MapData.h"
#include "Camera2D.h"
#include "TileRegistry.h"

/// <summary>
/// 静的なマップチップ描画クラス
/// </summary>
class MapChip {
public:
    MapChip();
    ~MapChip();

    void Initialize();
    void Draw(Camera2D& camera, const MapData& mapData);
    void DrawBackgroundDecorationBlock(Camera2D& camera, const MapData& mapData);

private:
    MapData* mapData_ = nullptr;
    std::map<int, int> textureCache_;

    void LoadTexturesFromManager();
    bool IsSameTile(int myID, int tx, int ty, TileLayer layer) const;
    void DrawLayer(Camera2D& camera, TileLayer layer);

    /// <summary>
    /// カリング範囲を計算（マージン付き）
    /// </summary>
    struct CullingRange {
        int startX, endX;
        int startY, endY;
    };
    CullingRange CalculateCullingRange(Camera2D& camera, int width, int height, float tileSize, int marginTiles) const;

    /// <summary>
    /// タイルの描画サイズを計算
    /// </summary>
    struct DrawSize {
        float width;
        float height;
    };
    DrawSize CalculateDrawSize(TileLayer layer, int texW, int texH, float tileSize) const;

    /// <summary>
    /// タイルのワールド座標4頂点を計算
    /// </summary>
    struct TileVertices {
        Vector2 worldLT, worldRT, worldLB, worldRB;
        Vector2 screenLT, screenRT, screenLB, screenRB;
    };
    TileVertices CalculateTileVertices(int x, int y, float tileSize, const Vector2& drawOffset,
        const DrawSize& drawSize, const Matrix3x3& vpVp) const;

    /// <summary>
    /// オートタイルのマスク値を計算
    /// </summary>
    int CalculateAutoTileMask(int tileID, int x, int y, TileLayer layer) const;

    /// <summary>
    /// オートタイルのsrc矩形を計算
    /// </summary>
    struct SrcRect {
        int x, y, w, h;
    };
    SrcRect CalculateAutoTileSrcRect(int mask, int texW, int texH) const;
};