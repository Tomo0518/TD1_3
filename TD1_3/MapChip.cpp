#include "MapChip.h"

MapChip::MapChip() {
}

MapChip::~MapChip() {
    // mapData_は外部（Sceneなど）が持っているはずなのでdeleteしない
}

void MapChip::Initialize(MapData* mapData, const std::string& textureFilePath) {
    mapData_ = mapData;

    // 画像のロード
    textureHandle_ = Novice::LoadTexture(textureFilePath.c_str());

    // 安全のため、もしロード失敗したらエラーログなどを出すと良い
    if (textureHandle_ == -1) {
        Novice::ConsolePrintf("[MapChip] Failed to load texture: %s\n", textureFilePath.c_str());
    }
}

void MapChip::Draw(Camera2D& camera) {
    // データがない、または画像がない場合は描画できない
    if (!mapData_ || textureHandle_ == -1) return;

    // マップ情報の取得
    int width = mapData_->GetWidth();
    int height = mapData_->GetHeight();
    float tileSize = mapData_->GetTileSize();

    // カメラの表示範囲を取得（カリング用）
    Vector2 cameraTopLeft = camera.GetTopLeft();
    Vector2 cameraBottomRight = camera.GetBottomRight();

    // 描画すべきグリッド範囲を計算（画面外はループすら回さない）
    // 左端：カメラ左端 ÷ タイルサイズ（0未満にならないようmax）
    int startX = (int)(cameraTopLeft.x / tileSize);
    startX = (startX < 0) ? 0 : startX;

    // 右端：カメラ右端 ÷ タイルサイズ（マップ幅を超えないようmin）
    int endX = (int)(cameraBottomRight.x / tileSize) + 1;
    endX = (endX > width) ? width : endX;

    // 上端
    int startY = (int)(cameraTopLeft.y / tileSize);
    startY = (startY < 0) ? 0 : startY;

    // 下端
    int endY = (int)(cameraBottomRight.y / tileSize) + 1;
    endY = (endY > height) ? height : endY;

    // 範囲内だけループして描画
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            // マップデータからチップ番号を取得
            int tileID = mapData_->GetTile(x, y);

            // 0は「空気」として描画しないルール
            if (tileID != 0) {
                // ワールド座標
                Vector2 worldPos;
                worldPos.x = x * tileSize;
                worldPos.y = y * tileSize;

                // スクリーン座標に変換（カメラクラスの機能を使用）
                // ※TransformクラスがあるならMatrixを使っても良いが、ここでは単純化
                Vector2 screenPos = camera.WorldToScreen(worldPos);

                // 描画（今回はIDに応じて元画像の切り抜き位置を変える処理は省略し、
                // 単純にその画像を描画する。タイルセットを使う場合はsrcRectの計算が必要）

                // 例：タイルセットが横10列の場合のUV計算
                /*
                int srcX = (tileID % 10) * (int)textureSrcSize_;
                int srcY = (tileID / 10) * (int)textureSrcSize_;
                Novice::DrawSpriteRect(
                    (int)screenPos.x, (int)screenPos.y,
                    srcX, srcY,
                    (int)textureSrcSize_, (int)textureSrcSize_,
                    textureHandle_,
                    tileSize / textureSrcSize_, // スケール
                    1, 0.0f, 0xFFFFFFFF
                );
                */

                // 簡易版：とりあえず全部同じ画像で描画（動作確認用）
                Novice::DrawSprite(
                    (int)screenPos.x, (int)screenPos.y,
                    textureHandle_,
                    1.0f, 1.0f,
                    0.0f, 0xFFFFFFFF
                );
            }
        }
    }
}