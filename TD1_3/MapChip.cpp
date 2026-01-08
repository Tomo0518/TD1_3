#include "MapChip.h"
#include "TileRegistry.h"

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
            int tileID = mapData_->GetTile(x, y);
            if (tileID == 0) continue;

            // IDから「どの絵を描くか」の情報をRegistryからもらう
            const TileDefinition* def = TileRegistry::GetTile(tileID);

            if (def && def->textureIndex >= 0) {
                // textureIndex を使ってUV計算
                // 仮に横10枚並んでいるタイルセットだとする
                int idx = def->textureIndex;
                int srcX = (idx % 10) * 64; // 64は元画像の1タイルの画素数
                int srcY = (idx / 10) * 64;

                Vector2 worldPos = { x * tileSize, y * tileSize };
                Vector2 screenPos = camera.WorldToScreen(worldPos);

                Novice::DrawSpriteRect(
                    (int)screenPos.x, (int)screenPos.y,
                    srcX, srcY, 64, 64, // 元画像の切り抜きサイズ
                    textureHandle_,
                    tileSize / 64.0f, // 拡大率
                    1, 0.0f, 0xFFFFFFFF
                );
            }
        }
    }
}