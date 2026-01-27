#pragma once
#include <vector>
#include <memory>
#include "TileInstance.h"
#include "MapData.h"
#include "Camera2D.h"

class MapManager {
public:
    MapManager() = default;
    ~MapManager() = default;

    // マップデータから演出用タイル(TileInstance)を生成
    void Initialize();

    // カメラ範囲に基づいた更新（カリング）
    void Update(float deltaTime, Camera2D& camera);

    // 描画
   // void Draw(const Camera2D& camera);

    void Draw(const Camera2D& camera,DrawLayer layer);

    // 特定座標のタイルに干渉
    void InteractionTile(const Vector2& worldPos);

	// タイル変更通知(Editorで使用)
    void OnTileChanged(int x, int y, TileLayer layer);

private:
    std::vector<std::unique_ptr<TileInstance>> dynamicTiles_;
};