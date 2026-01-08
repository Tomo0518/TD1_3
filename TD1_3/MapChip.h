#pragma once
#include <Novice.h>
#include <string>
#include "MapData.h"
#include "Camera2D.h"

/// <summary>
/// マップの描画担当クラス
/// ロジック（当たり判定）は持たず、MapDataを見て絵を出すだけ
/// </summary>
class MapChip {
public:
    // コンストラクタ・デストラクタ
    MapChip();
    ~MapChip();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="mapData">描画対象のマップデータへのポインタ</param>
    /// <param name="textureFilePath">タイルセット画像のパス</param>
    void Initialize(MapData* mapData, const std::string& textureFilePath);

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">カメラ（カリングと座標変換に使用）</param>
    void Draw(Camera2D& camera);

    /// <summary>
    /// 参照するマップデータを変更する（シーン遷移時など）
    /// </summary>
    void SetMapData(MapData* mapData) { mapData_ = mapData; }

private:
    // 描画対象のデータ（所有権は持たず、参照するだけ）
    MapData* mapData_ = nullptr;

    // マップチップの画像ハンドル（タイルセット）
    int textureHandle_ = -1;

    // タイルセット画像内の1チップのサイズ（px）
    // ※画面上の描画サイズ(mapData_->tileSize)とは別に、元画像のサイズが必要な場合に使用
    // 今回は簡単のため、画面描画サイズと同じと仮定するか、別途定数で管理
    float textureSrcSize_ = 64.0f;
};