#include "MapChip.h"
#include "TileRegistry.h"
#include "TextureManager.h"

MapChip::MapChip() {
}

MapChip::~MapChip() {
}

void MapChip::Initialize() {
	mapData_ = &MapData::GetInstance();
	LoadTexturesFromManager();
}

void MapChip::LoadTexturesFromManager() {
	textureCache_.clear();
	const auto& tiles = TileRegistry::GetAllTiles();

	for (const auto& tile : tiles) {
		if (tile.type != TileType::None) {
			int handle = TextureManager::GetInstance().GetTexture(tile.textureId);
			if (handle >= 0) {
				textureCache_[tile.id] = handle;
			}
		}
	}
}

bool MapChip::IsSameTile(int myID, int tx, int ty, TileLayer layer) const {
	if (!mapData_) return false;
	if (tx < 0 || tx >= mapData_->GetWidth() ||
		ty < 0 || ty >= mapData_->GetHeight()) {
		return true;
	}
	return (mapData_->GetTile(tx, ty, layer) == myID);
}

void MapChip::Draw(Camera2D& camera) {
	if (!mapData_) return;

	DrawLayer(camera, TileLayer::Decoration);
	DrawLayer(camera, TileLayer::Block);
}

// --- カリング範囲計算 ---
MapChip::CullingRange MapChip::CalculateCullingRange(Camera2D& camera, int width, int height,
	float tileSize, int marginTiles) const {
	Vector2 cameraTopLeft = camera.GetTopLeft();
	Vector2 cameraBottomRight = camera.GetBottomRight();

	// マージンを適用
	cameraTopLeft.x -= marginTiles * tileSize;
	cameraTopLeft.y -= marginTiles * tileSize;
	cameraBottomRight.x += marginTiles * tileSize;
	cameraBottomRight.y += marginTiles * tileSize;

	CullingRange range;
	range.startX = static_cast<int>(cameraTopLeft.x / tileSize);
	range.startX = (range.startX < 0) ? 0 : range.startX;

	range.endX = static_cast<int>(cameraBottomRight.x / tileSize) + 1;
	range.endX = (range.endX > width) ? width : range.endX;

	range.startY = static_cast<int>(cameraTopLeft.y / tileSize);
	range.startY = (range.startY < 0) ? 0 : range.startY;

	range.endY = static_cast<int>(cameraBottomRight.y / tileSize) + 1;
	range.endY = (range.endY > height) ? height : range.endY;

	return range;
}

// --- 描画サイズ計算 ---
MapChip::DrawSize MapChip::CalculateDrawSize(TileLayer layer, int texW, int texH, float tileSize) const {
	DrawSize size;
	if (layer == TileLayer::Decoration) {
		// Decorationは実サイズ
		size.width = static_cast<float>(texW);
		size.height = static_cast<float>(texH);
	}
	else {
		// それ以外はタイルサイズに合わせる
		size.width = tileSize;
		size.height = tileSize;
	}
	return size;
}

// --- タイル頂点計算 ---
MapChip::TileVertices MapChip::CalculateTileVertices(int x, int y, float tileSize,
	const Vector2& drawOffset, const DrawSize& drawSize,
	const Matrix3x3& vpVp) const {
	TileVertices vertices;

	// ワールド座標
	const float tileLeft = x * tileSize + drawOffset.x;
	const float tileTop = y * tileSize + drawOffset.y;
	const float tileRight = tileLeft + drawSize.width;
	const float tileBottom = tileTop + drawSize.height;

	vertices.worldLT = { tileLeft,  tileTop };
	vertices.worldRT = { tileRight, tileTop };
	vertices.worldLB = { tileLeft,  tileBottom };
	vertices.worldRB = { tileRight, tileBottom };

	// スクリーン座標への変換
	vertices.screenLT = Matrix3x3::Transform(vertices.worldLT, vpVp);
	vertices.screenRT = Matrix3x3::Transform(vertices.worldRT, vpVp);
	vertices.screenLB = Matrix3x3::Transform(vertices.worldLB, vpVp);
	vertices.screenRB = Matrix3x3::Transform(vertices.worldRB, vpVp);

	return vertices;
}

// --- オートタイルマスク計算 ---
int MapChip::CalculateAutoTileMask(int tileID, int x, int y, TileLayer layer) const {
	// 4方向の判定
	bool up = IsSameTile(tileID, x, y + 1, layer);
	bool right = IsSameTile(tileID, x + 1, y, layer);
	bool down = IsSameTile(tileID, x, y - 1, layer);
	bool left = IsSameTile(tileID, x - 1, y, layer);

	// マスクテーブル
	int maskTable[16] = {
		!up && !left && down && right,  !up && left && down && right,  !up && left && down && !right,  !up && !left && down && !right,
		up && !left && down && right,   up && left && down && right,   up && left && down && !right,   up && !left && down && !right,
		up && !left && !down && right,  up && left && !down && right,  up && left && !down && !right,  up && !left && !down && !right,
		!up && !left && !down && right, !up && left && !down && right, !up && left && !down && !right, !up && !left && !down && !right
	};

	int mask = 0;
	while (mask < 16 && !maskTable[mask]) {
		mask++;
	}

	// 特殊ケース（4方向全て埋まっている場合、斜めを確認）
	if (mask == 5) {
		bool leftTop = IsSameTile(tileID, x - 1, y + 1, layer);
		bool rightTop = IsSameTile(tileID, x + 1, y + 1, layer);
		bool leftBottom = IsSameTile(tileID, x - 1, y - 1, layer);
		bool rightBottom = IsSameTile(tileID, x + 1, y - 1, layer);

		if (!leftTop && rightTop && leftBottom && rightBottom) {
			mask = 16;
		}
		else if (leftTop && !rightTop && leftBottom && rightBottom) {
			mask = 17;
		}
		else if (leftTop && rightTop && !leftBottom && rightBottom) {
			mask = 18;
		}
		else if (leftTop && rightTop && leftBottom && !rightBottom) {
			mask = 19;
		}
	}

	return mask;
}

// --- オートタイルsrc矩形計算 ---
MapChip::SrcRect MapChip::CalculateAutoTileSrcRect(int mask, int texW, int texH) const {
	SrcRect rect = { 0, 0, texW, texH };

	// タイルの画像の分割数（4列×5行）
	const int cols = 4;
	const int rows = 5;

	const int cellW = texW / cols;
	const int cellH = texH / rows;

	if (cellW > 0 && cellH > 0) {
		rect.x = (mask % cols) * cellW;
		rect.y = (mask / cols) * cellH;
		rect.w = cellW;
		rect.h = cellH;

		// 範囲チェック
		if (rect.x < 0 || rect.y < 0 || rect.x + rect.w > texW || rect.y + rect.h > texH) {
			rect = { 0, 0, texW, texH };
		}
	}

	return rect;
}

// --- メイン描画処理 ---
void MapChip::DrawLayer(Camera2D& camera, TileLayer layer) {
	if (!mapData_) return;

	const int width = mapData_->GetWidth();
	const int height = mapData_->GetHeight();
	const float tileSize = mapData_->GetTileSize();
	const int cullingMarginTiles = 3;

	// 1. カリング範囲計算
	CullingRange range = CalculateCullingRange(camera, width, height, tileSize, cullingMarginTiles);
	const Matrix3x3 vpVp = camera.GetVpVpMatrix();

	// 2. タイルループ
	for (int y = range.startY; y < range.endY; ++y) {
		for (int x = range.startX; x < range.endX; ++x) {
			const int tileID = mapData_->GetTile(x, y, layer);
			if (tileID == 0) continue;

			const TileDefinition* def = TileRegistry::GetTile(tileID);
			if (!def) continue;

			// 3. テクスチャハンドル取得
			const auto it = textureCache_.find(tileID);
			if (it == textureCache_.end()) continue;

			const int handle = it->second;
			if (handle < 0) continue;

			// 4. テクスチャサイズ取得
			int texW = 0, texH = 0;
			Novice::GetTextureSize(handle, &texW, &texH);
			if (texW <= 0 || texH <= 0) continue;

			// 5. 描画サイズ決定
			DrawSize drawSize = CalculateDrawSize(layer, texW, texH, tileSize);

			// 6. タイル頂点計算
			TileVertices vertices = CalculateTileVertices(x, y, tileSize, def->drawOffset, drawSize, vpVp);

			// 7. src矩形計算
			SrcRect srcRect = { 0, 0, texW, texH };
			if (def->type == TileType::AutoTile) {
				int mask = CalculateAutoTileMask(tileID, x, y, layer);
				srcRect = CalculateAutoTileSrcRect(mask, texW, texH);
			}

			// 8. 描画
			Novice::DrawQuad(
				static_cast<int>(vertices.screenLT.x), static_cast<int>(vertices.screenLB.y),
				static_cast<int>(vertices.screenRT.x), static_cast<int>(vertices.screenRB.y),
				static_cast<int>(vertices.screenLB.x), static_cast<int>(vertices.screenLT.y),
				static_cast<int>(vertices.screenRB.x), static_cast<int>(vertices.screenRT.y),
				srcRect.x, srcRect.y, srcRect.w, srcRect.h,
				handle,
				0xFFFFFFFF
			);
		}
	}
}