#include "PhysicsManager.h"
#include <algorithm> // min, max, abs
#include <cmath>


HitDirection PhysicsManager::ResolveMapCollision(GameObject2D* obj, const MapData& map) {
	if (!obj) return HitDirection::None;

	HitDirection hitDir = HitDirection::None;

	// 1. オブジェクトのAABB（当たり判定の四角形）情報を取得
	Transform2D& transform = obj->GetTransform();
	Collider& collider = obj->GetCollider();
	Rigidbody2D& rb = obj->GetRigidbody();

	if (!collider.canCollide) return HitDirection::None;

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

	// 最もめり込みが深い衝突を記録するための変数
	float maxPenetration = 0.0f;

	// 移動方向を取得（速度ベクトル）
	Vector2 moveDirection = { rb.velocity.x, rb.velocity.y };
	float moveSpeed = std::sqrt(moveDirection.x * moveDirection.x + moveDirection.y * moveDirection.y);

	// 移動方向の正規化（ゼロ除算回避）
	if (moveSpeed > 0.01f) {
		moveDirection.x /= moveSpeed;
		moveDirection.y /= moveSpeed;
	}

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
				float ox = (dx1 < dx2) ? dx1 : dx2; // X軸の修正量（絶対値が小さい方）
				float oy = (dy1 < dy2) ? dy1 : dy2; // Y軸の修正量（絶対値が小さい方）

				// めり込みの深さ（最小の修正量）
				float penetrationDepth = std::min(ox, oy);

				// 移動方向との整合性をチェック
				// 移動方向と逆方向の衝突は優先度を下げる
				float directionPriority = 1.0f;

				if (ox < oy) {
					// X方向の衝突
					bool movingRight = rb.velocity.x > 0.1f;
					bool movingLeft = rb.velocity.x < -0.1f;
					bool collidingFromRight = (dx2 < dx1); // 右から当たっている
					bool collidingFromLeft = (dx1 < dx2);  // 左から当たっている

					// 移動方向と衝突方向が一致しない場合は優先度を下げる
					if ((movingRight && collidingFromLeft) || (movingLeft && collidingFromRight)) {
						directionPriority = 0.5f; // 優先度を下げる
					}
				}
				else {
					// Y方向の衝突
					bool movingUp = rb.velocity.y > 0.1f;
					bool movingDown = rb.velocity.y < -0.1f;
					bool collidingFromTop = (dy1 < dy2);    // 上から当たっている
					bool collidingFromBottom = (dy2 < dy1); // 下から当たっている

					// 移動方向と衝突方向が一致しない場合は優先度を下げる
					if ((movingUp && collidingFromBottom) || (movingDown && collidingFromTop)) {
						directionPriority = 0.5f; // 優先度を下げる
					}
				}

				// 優先度を考慮した実効的な深さ
				float effectivePenetration = penetrationDepth * directionPriority;

				if (ox < oy) {
					// X軸方向のめり込みの方が浅い -> 横に押し出す
					bool movingIntoWall = (dx1 < dx2 && rb.velocity.x < -0.5f) ||
						(dx1 >= dx2 && rb.velocity.x > 0.5f);
					bool significantPenetration = ox > 0.5f;

					if (movingIntoWall || significantPenetration) {
						// 基本の押し出し + 微小な余白（0.4px）
						float extraPush = 0.4f;
						float pushAmount = (dx1 < dx2) ? (dx1 + extraPush) : -(dx2 + extraPush);
						transform.translate.x += pushAmount;

						// 速度リセットは移動中のみ
						if (movingIntoWall) {
							rb.velocity.x = 0.0f;
						}

						objLeft = transform.translate.x + collider.offset.x - collider.size.x * 0.5f;
						objRight = objLeft + objW;

						// 優先度を考慮して方向を更新
						if (effectivePenetration > maxPenetration) {
							maxPenetration = effectivePenetration;
							hitDir = (dx1 < dx2) ? HitDirection::Left : HitDirection::Right;
						}
					}
				}
				else {
					// Y軸方向のめり込みの方が浅い -> 縦に押し出す
					transform.translate.y += (dy1 < dy2) ? dy1 : -dy2;
					rb.velocity.y = 0.0f;

					// 座標更新
					objTop = transform.translate.y + collider.offset.y - collider.size.y * 0.5f;
					objBottom = objTop + objH;

					// 優先度を考慮して方向を更新
					if (effectivePenetration > maxPenetration) {
						maxPenetration = effectivePenetration;
						hitDir = (dy1 < dy2) ? HitDirection::Top : HitDirection::Bottom;
					}
				}
			}
		}
	}

	// 最後にワールド行列を再計算
	transform.CalculateWorldMatrix();

	return hitDir;
}

// Y軸専用の衝突判定（Top/Bottom/Noneのみ返す）
HitDirection PhysicsManager::ResolveMapCollisionY(GameObject2D* obj, const MapData& map) {
	if (!obj) return HitDirection::None;

	HitDirection hitDir = HitDirection::None;

	// 1. オブジェクトのAABB情報を取得
	Transform2D& transform = obj->GetTransform();
	Collider& collider = obj->GetCollider();
	Rigidbody2D& rb = obj->GetRigidbody();

	if (!collider.canCollide) return HitDirection::None;

	// ワールド座標での計算
	float objLeft = transform.translate.x + collider.offset.x - collider.size.x * 0.5f;
	float objTop = transform.translate.y + collider.offset.y - collider.size.y * 0.5f;
	float objW = collider.size.x;
	float objH = collider.size.y;
	float objRight = objLeft + objW;
	float objBottom = objTop + objH;

	// 2. マップ範囲計算
	float tileSize = map.GetTileSize();
	int leftTile = (int)(objLeft / tileSize);
	int rightTile = (int)(objRight / tileSize);
	int topTile = (int)(objTop / tileSize);
	int bottomTile = (int)(objBottom / tileSize);

	float maxPenetration = 0.0f;

	// 3. ブロック走査（Y方向の衝突のみ処理）
	for (int y = topTile; y <= bottomTile; ++y) {
		for (int x = leftTile; x <= rightTile; ++x) {

			int tileID = map.GetTile(x, y);
			if (tileID == 0) continue;

			const TileDefinition* def = TileRegistry::GetTile(tileID);
			if (!def || !def->isSolid) continue;

			// ブロックのAABB
			float tileLeft = x * tileSize;
			float tileTop = y * tileSize;
			float tileRight = tileLeft + tileSize;
			float tileBottom = tileTop + tileSize;

			// めり込み量計算
			float dx1 = tileRight - objLeft;
			float dx2 = objRight - tileLeft;
			float dy1 = tileBottom - objTop;
			float dy2 = objBottom - tileTop;

			// AABB判定
			if (dx1 > 0 && dx2 > 0 && dy1 > 0 && dy2 > 0) {

				float ox = (dx1 < dx2) ? dx1 : dx2;
				float oy = (dy1 < dy2) ? dy1 : dy2;

				// Y方向の衝突のみ処理（ox >= oy の場合のみ）
				if (ox >= oy) {
					float penetrationDepth = oy;

					// 移動方向との整合性チェック
					float directionPriority = 1.0f;
					bool movingUp = rb.velocity.y > 0.1f;
					bool movingDown = rb.velocity.y < -0.1f;
					bool collidingFromTop = (dy1 < dy2);
					bool collidingFromBottom = (dy2 < dy1);

					if ((movingUp && collidingFromBottom) || (movingDown && collidingFromTop)) {
						directionPriority = 0.5f;
					}

					float effectivePenetration = penetrationDepth * directionPriority;

					// Y軸方向に押し出す
					transform.translate.y += (dy1 < dy2) ? dy1 : -dy2;
					rb.velocity.y = 0.0f;

					// 座標更新
					objTop = transform.translate.y + collider.offset.y - collider.size.y * 0.5f;
					objBottom = objTop + objH;

					// 方向を更新
					if (effectivePenetration > maxPenetration) {
						maxPenetration = effectivePenetration;
						hitDir = (dy1 < dy2) ? HitDirection::Top : HitDirection::Bottom;
					}
				}
			}
		}
	}

	transform.CalculateWorldMatrix();
	return hitDir;
}

// X軸専用の衝突判定（Left/Right/Noneのみ返す）
HitDirection PhysicsManager::ResolveMapCollisionX(GameObject2D* obj, const MapData& map) {
	if (!obj) return HitDirection::None;

	HitDirection hitDir = HitDirection::None;

	// 1. オブジェクトのAABB情報を取得
	Transform2D& transform = obj->GetTransform();
	Collider& collider = obj->GetCollider();
	Rigidbody2D& rb = obj->GetRigidbody();

	if (!collider.canCollide) return HitDirection::None;

	// ワールド座標での計算
	float objLeft = transform.translate.x + collider.offset.x - collider.size.x * 0.5f;
	float objTop = transform.translate.y + collider.offset.y - collider.size.y * 0.5f;
	float objW = collider.size.x;
	float objH = collider.size.y;
	float objRight = objLeft + objW;
	float objBottom = objTop + objH;

	// 2. マップ範囲計算
	float tileSize = map.GetTileSize();
	int leftTile = (int)(objLeft / tileSize);
	int rightTile = (int)(objRight / tileSize);
	int topTile = (int)(objTop / tileSize);
	int bottomTile = (int)(objBottom / tileSize);

	float maxPenetration = 0.0f;

	// 3. ブロック走査（X方向の衝突のみ処理）
	for (int y = topTile; y <= bottomTile; ++y) {
		for (int x = leftTile; x <= rightTile; ++x) {

			int tileID = map.GetTile(x, y);
			if (tileID == 0) continue;

			const TileDefinition* def = TileRegistry::GetTile(tileID);
			if (!def || !def->isSolid) continue;

			// ブロックのAABB
			float tileLeft = x * tileSize;
			float tileTop = y * tileSize;
			float tileRight = tileLeft + tileSize;
			float tileBottom = tileTop + tileSize;

			// めり込み量計算
			float dx1 = tileRight - objLeft;
			float dx2 = objRight - tileLeft;
			float dy1 = tileBottom - objTop;
			float dy2 = objBottom - tileTop;

			// AABB判定
			if (dx1 > 0 && dx2 > 0 && dy1 > 0 && dy2 > 0) {

				float ox = (dx1 < dx2) ? dx1 : dx2;
				float oy = (dy1 < dy2) ? dy1 : dy2;

				// X方向の衝突のみ処理（ox < oy の場合のみ）
				if (ox < oy) {
					float penetrationDepth = ox;

					// 移動方向との整合性チェック
					float directionPriority = 1.0f;
					bool movingRight = rb.velocity.x > 0.1f;
					bool movingLeft = rb.velocity.x < -0.1f;
					bool collidingFromRight = (dx2 < dx1);
					bool collidingFromLeft = (dx1 < dx2);

					if ((movingRight && collidingFromLeft) || (movingLeft && collidingFromRight)) {
						directionPriority = 0.5f;
					}

					float effectivePenetration = penetrationDepth * directionPriority;

					// X軸方向に押し出す
					bool movingIntoWall = (dx1 < dx2 && rb.velocity.x < -0.5f) ||
						(dx1 >= dx2 && rb.velocity.x > 0.5f);
					bool significantPenetration = ox > 0.5f;

					if (movingIntoWall || significantPenetration) {
						float extraPush = 0.4f;
						float pushAmount = (dx1 < dx2) ? (dx1 + extraPush) : -(dx2 + extraPush);
						transform.translate.x += pushAmount;

						if (movingIntoWall) {
							rb.velocity.x = 0.0f;
						}

						objLeft = transform.translate.x + collider.offset.x - collider.size.x * 0.5f;
						objRight = objLeft + objW;

						// 方向を更新
						if (effectivePenetration > maxPenetration) {
							maxPenetration = effectivePenetration;
							hitDir = (dx1 < dx2) ? HitDirection::Left : HitDirection::Right;
						}
					}
				}
			}
		}
	}

	transform.CalculateWorldMatrix();
	return hitDir;
}

bool PhysicsManager::CheckAABB(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

bool PhysicsManager::ObjectsCollision(GameObject2D* objA, GameObject2D* objB) {
	if (!objA || !objB) return false;
	Collider& colA = objA->GetCollider();
	Collider& colB = objB->GetCollider();
	if (!colA.canCollide || !colB.canCollide) return false;
	Transform2D& transA = objA->GetTransform();
	Transform2D& transB = objB->GetTransform();
	float aLeft = transA.translate.x + colA.offset.x - colA.size.x * 0.5f;
	float aTop = transA.translate.y + colA.offset.y - colA.size.y * 0.5f;
	float bLeft = transB.translate.x + colB.offset.x - colB.size.x * 0.5f;
	float bTop = transB.translate.y + colB.offset.y - colB.size.y * 0.5f;
	return CheckAABB(aLeft, aTop, colA.size.x, colA.size.y, bLeft, bTop, colB.size.x, colB.size.y);
}

bool PhysicsManager::ResolveObjectsCollisions(std::vector<GameObject2D*>& objects) {
	bool collisionDetected = false;
	for (size_t i = 0; i < objects.size(); ++i) {
		for (size_t j = i + 1; j < objects.size(); ++j) {
			GameObject2D* objA = objects[i];
			GameObject2D* objB = objects[j];
			if (objA == objB) continue;
			if (!objA->GetInfo().isActive || !objB->GetInfo().isActive) continue;
			if (objA->IsDead() || objB->IsDead()) continue;
			if (ObjectsCollision(objA, objB)) {
				objA->OnCollision(objB);
				objB->OnCollision(objA);
				collisionDetected = true;
			}
		}
	}
	return collisionDetected;
}