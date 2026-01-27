#pragma once
#include "MapData.h"
#include "GameObject2D.h"
#include "TileRegistry.h"
#include <vector>

enum class HitDirection {
    None,
    Top,
    Bottom,
    Left,
    Right
};

/// <summary>
/// マップとオブジェクトの衝突判定・応答を行うクラス
/// (最小実装版：矩形ブロックとの当たり判定のみ)
/// </summary>
class PhysicsManager {
public:
    /// <summary>
    /// 単体オブジェクトのマップ衝突解決
    /// めり込んでいたら位置を補正し、速度を0にする
    /// </summary>
    static HitDirection ResolveMapCollision(GameObject2D* obj, const MapData& map);

    // Y軸専用の衝突判定（Top/Bottom/Noneのみ返す）
    static HitDirection ResolveMapCollisionY(GameObject2D * obj, const MapData & map);

    // X軸専用の衝突判定（Left/Right/Noneのみ返す）  
    static HitDirection ResolveMapCollisionX(GameObject2D* obj, const MapData& map);

	static bool ResolveObjectsCollisions(std::vector<GameObject2D*>& objects);

    static bool ObjectsCollision(GameObject2D* objA, GameObject2D* objB);

private:
    // 矩形同士の衝突チェック（ヘルパー関数）
    static bool CheckAABB(
        float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2
    );

   
};