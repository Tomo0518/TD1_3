#include "MapManager.h"
#include "TileRegistry.h"
#include "WindowSize.h"

void MapManager::Initialize() {
    dynamicTiles_.clear();
    auto& mapData = MapData::GetInstance();
    float tileSize = mapData.GetTileSize();

    // 全レイヤーを走査してComponent指定のタイルを探す
    std::vector<TileLayer> layers = { TileLayer::Decoration, TileLayer::Block };

    for (auto layer : layers) {
        for (int y = 0; y < mapData.GetHeight(); ++y) {
            for (int x = 0; x < mapData.GetWidth(); ++x) {
                int id = mapData.GetTile(x, y, layer);
                if (id == 0) continue;

                const TileDefinition* def = TileRegistry::GetTile(id);
                if (def && def->renderMode == RenderMode::Component) {
                    // タイルの中心座標を計算
                    Vector2 worldPos = {
                        x * tileSize + tileSize * 0.5f,
                        y * tileSize + tileSize * 0.5f
                    };
                    dynamicTiles_.push_back(std::make_unique<TileInstance>(*def, worldPos));
                }
            }
        }
    }
}

void MapManager::Update(float deltaTime, Camera2D& camera) {
    // カメラ表示範囲より少し広い矩形（アクティブエリア）を計算
    float margin = 128.0f;
    Vector2 camPos = camera.GetPosition();
    float hw = (kWindowWidth * (1.0f / camera.GetZoom())) * 0.5f + margin;
    float hh = (kWindowHeight * (1.0f / camera.GetZoom())) * 0.5f + margin;

    for (auto& tile : dynamicTiles_) {
        Vector2 tPos = tile->GetWorldPos();
        bool inRange = (tPos.x > camPos.x - hw && tPos.x < camPos.x + hw &&
            tPos.y > camPos.y - hh && tPos.y < camPos.y + hh);

        tile->Update(deltaTime, inRange);
    }
}

//void MapManager::Draw(const Camera2D& camera) {
//    // 描画自体はTileInstance側でDrawComponent2Dを介して行う
//    for (auto& tile : dynamicTiles_) {
//        tile->Draw(camera);
//    }
//}

void MapManager::Draw(const Camera2D& camera, DrawLayer layer) {
    // 指定されたDrawLayerのタイルのみ描画
    for (auto& tile : dynamicTiles_) {
        if (tile->GetDrawLayer() == layer) {
            tile->Draw(camera);
        }
    }
}

void MapManager::InteractionTile(const Vector2& worldPos) {
    //「手触り」：攻撃が当たった場所などのタイルを揺らす
    float range = 32.0f; // 判定半径
    for (auto& tile : dynamicTiles_) {
        Vector2 diff = { tile->GetWorldPos().x - worldPos.x, tile->GetWorldPos().y - worldPos.y };
        if (diff.x * diff.x + diff.y * diff.y < range * range) {
            tile->OnHit();
        }
    }
}

void MapManager::OnTileChanged(int col, int row, TileLayer layer) {
    auto& mapData = MapData::GetInstance();
    float tileSize = mapData.GetTileSize();

    // 1. 座標の特定（タイルの中心点）
    Vector2 targetPos = {
        col * tileSize + tileSize * 0.5f,
        row * tileSize + tileSize * 0.5f
    };

    // 2. 削除処理：リストから指定座標に一致するインスタンスを探して削除
    // std::remove_if を使うのが、スマートポインタのリスト操作として最も合理的で高速です
    dynamicTiles_.erase(
        std::remove_if(dynamicTiles_.begin(), dynamicTiles_.end(),
            [&](const std::unique_ptr<TileInstance>& tile) {
                Vector2 p = tile->GetWorldPos();
                // 浮動小数点の誤差が心配な場合は、0.1f程度の誤差を許容して判定します
                return (std::abs(p.x - targetPos.x) < 0.1f && std::abs(p.y - targetPos.y) < 0.1f);
            }),
        dynamicTiles_.end()
    );

    // 3. 再生成（削除だけの場合は、ここでの ID チェックで終了します）
    int id = mapData.GetTile(col, row, layer);
    if (id == 0) return; // IDが0（空気）なら、削除だけで終了

    const TileDefinition* def = TileRegistry::GetTile(id);
    if (def && def->renderMode == RenderMode::Component) {
        dynamicTiles_.push_back(std::make_unique<TileInstance>(*def, targetPos));
        dynamicTiles_.back()->OnHit(); // 置いた瞬間のリアクション
    }
}