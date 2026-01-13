#include "PhysicsManager.h"
#include <algorithm> // min, max, abs
#include <cmath>

void PhysicsManager::ResolveMapCollision(GameObject2D* obj, const MapData& map) {
    if (!obj) return;

    // 1. オブジェクトのAABB（当たり判定の四角形）情報を取得
    Transform2D& transform = obj->GetTransform();
    Collider& collider = obj->GetCollider();
    Rigidbody2D& rb = obj->GetRigidbody();

    if (!collider.canCollide) return;

    // ワールド座標での左上座標とサイズ
    float objLeft = transform.translate.x + collider.offset.x - collider.size.x * 0.5f;
    float objTop = transform.translate.y + collider.offset.y - collider.size.y * 0.5f;
    float objW = collider.size.x;
    float objH = collider.size.y;
    float objRight = objLeft + objW;
    float objBottom = objTop + objH;

    // 2. 判定すべきマップの範囲を計算（オブジェクトの周囲のみチェック）
    float tileSize = map.GetTileSize();
    int leftTile = (int)(objLeft / tileSize);
    int rightTile = (int)(objRight / tileSize);
    int topTile = (int)(objTop / tileSize);
    int bottomTile = (int)(objBottom / tileSize);

    // マップ範囲外チェックは MapData::GetTile 内で行われるのでここではざっくりでOK
    // ただしループ範囲は制限する

    // 3. 周囲のブロックを走査して衝突チェック
    for (int y = topTile; y <= bottomTile; ++y) {
        for (int x = leftTile; x <= rightTile; ++x) {

            // ブロックID取得
            int tileID = map.GetTile(x, y);
            if (tileID == 0) continue; // 空気ならスルー

            // ブロックの定義を確認
            const TileDefinition* def = TileRegistry::GetTile(tileID);
            if (!def || !def->isSolid) continue; // 当たり判定がないブロックならスルー

            // --- 衝突している！ ---

            // ブロックのAABB
            float tileLeft = x * tileSize;
            float tileTop = y * tileSize;
            float tileRight = tileLeft + tileSize;
            float tileBottom = tileTop + tileSize;

            // めり込み量を計算（Overlap）
            float dx1 = tileRight - objLeft; // 左から当たった場合のめり込み
            float dx2 = objRight - tileLeft; // 右から当たった場合のめり込み
            float dy1 = tileBottom - objTop; // 上から当たった場合のめり込み
            float dy2 = objBottom - tileTop; // 下から当たった場合のめり込み

            // AABB判定（念のため）
            if (dx1 > 0 && dx2 > 0 && dy1 > 0 && dy2 > 0) {

                // 最もめり込みが浅い方向（＝脱出最短ルート）を探す
                float ox = (dx1 < dx2) ? -dx1 : dx2; // X軸の修正量（絶対値が小さい方）
                float oy = (dy1 < dy2) ? -dy1 : dy2; // Y軸の修正量（絶対値が小さい方）

                if (std::abs(ox) < std::abs(oy)) {
                    // X軸方向のめり込みの方が浅い -> 横に押し出す
                    transform.translate.x += (dx1 < dx2) ? dx1 : -dx2;
                    rb.velocity.x = 0.0f; // 壁にぶつかったので速度リセット

                    // 座標更新に伴いAABB情報も更新しないと、次のブロック判定でおかしくなる
                    objLeft = transform.translate.x + collider.offset.x - collider.size.x * 0.5f;
                    objRight = objLeft + objW;
                }
                else {
                    // Y軸方向のめり込みの方が浅い -> 縦に押し出す
                    transform.translate.y += (dy1 < dy2) ? dy1 : -dy2;
                    rb.velocity.y = 0.0f; // 床/天井にぶつかったので速度リセット

                    // 座標更新
                    objTop = transform.translate.y + collider.offset.y - collider.size.y * 0.5f;
                    objBottom = objTop + objH;
                }
            }
        }
    }

    // 最後にワールド行列を再計算（位置が変わったため）
    transform.CalculateWorldMatrix();
}

bool PhysicsManager::CheckAABB(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}