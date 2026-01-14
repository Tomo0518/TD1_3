#include "MapChip.h"
#include "TileRegistry.h"
#include "TextureManager.h"

MapChip::MapChip() {
}

MapChip::~MapChip() {
}

void MapChip::Initialize(MapData* mapData) {
	mapData_ = mapData;

	// TextureManagerからハンドルを取得してキャッシュを作成
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

// レイヤーを考慮して判定
bool MapChip::IsSameTile(int myID, int tx, int ty, TileLayer layer) const {
	if (!mapData_) return false;
	if (tx < 0 || tx >= mapData_->GetWidth() ||
		ty < 0 || ty >= mapData_->GetHeight()) {
		return true;
	}
	// 指定したレイヤーのタイルと比較
	return (mapData_->GetTile(tx, ty, layer) == myID);
}

// 全レイヤーの描画
void MapChip::Draw(Camera2D& camera) {
	if (!mapData_) return;

	// 1. デコレーション（奥）を描画
	DrawLayer(camera, TileLayer::Decoration);

	// 2. ブロック（手前）を描画
	DrawLayer(camera, TileLayer::Block);
}

// 特定レイヤーだけを描画する
void MapChip::DrawLayer(Camera2D& camera, TileLayer layer) {
	if (!mapData_) return;

	const int width = mapData_->GetWidth();
	const int height = mapData_->GetHeight();
	const float tileSize = mapData_->GetTileSize();

	// カメラ表示範囲（カリング）
	Vector2 cameraTopLeft = camera.GetTopLeft();
	Vector2 cameraBottomRight = camera.GetBottomRight();

	int startX = static_cast<int>(cameraTopLeft.x / tileSize);
	startX = (startX < 0) ? 0 : startX;

	int endX = static_cast<int>(cameraBottomRight.x / tileSize) + 1;
	endX = (endX > width) ? width : endX;

	int startY = static_cast<int>(cameraTopLeft.y / tileSize);
	startY = (startY < 0) ? 0 : startY;

	int endY = static_cast<int>(cameraBottomRight.y / tileSize) + 1;
	endY = (endY > height) ? height : endY;

	const Matrix3x3 vpVp = camera.GetVpVpMatrix();

	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			const int tileID = mapData_->GetTile(x, y, layer);
			if (tileID == 0) continue;

			const TileDefinition* def = TileRegistry::GetTile(tileID);
			if (!def) continue;

			// tileID -> texture handle（キャッシュ）
			const auto it = textureCache_.find(tileID);
			if (it == textureCache_.end()) continue;

			const int handle = it->second;
			if (handle < 0) continue;

			// テクスチャサイズ取得（自動分割のため）
			int texW = 0, texH = 0;
			Novice::GetTextureSize(handle, &texW, &texH);
			if (texW <= 0 || texH <= 0) continue;

			// ===== タイルのワールド4頂点（左上基準） =====
			const float tileLeft = x * tileSize;
			const float tileTop = y * tileSize;
			const float tileRight = tileLeft + tileSize;
			const float tileBottom = tileTop + tileSize;

			const Vector2 wLT = { tileLeft , tileTop };
			const Vector2 wRT = { tileRight, tileTop };
			const Vector2 wLB = { tileLeft , tileBottom };
			const Vector2 wRB = { tileRight, tileBottom };

			// カメラ変換（回転・ズーム含む）
			const Vector2 sLT = Matrix3x3::Transform(wLT, vpVp);
			const Vector2 sRT = Matrix3x3::Transform(wRT, vpVp);
			const Vector2 sLB = Matrix3x3::Transform(wLB, vpVp);
			const Vector2 sRB = Matrix3x3::Transform(wRB, vpVp);

			// ===== src矩形（通常：全体 / AutoTile：4x4から切り抜き） =====
			int srcX = 0;
			int srcY = 0;
			int srcW = texW;
			int srcH = texH;

			if (def->type == TileType::AutoTile) {
				// ビットマスク計算 (上=1, 右=2, 下=4, 左=8)
				bool up = false, right = false, down = false, left = false;

				if (IsSameTile(tileID, x, y + 1, layer)) up = true;
				if (IsSameTile(tileID, x + 1, y, layer)) right = true;
				if (IsSameTile(tileID, x, y - 1, layer)) down = true;
				if (IsSameTile(tileID, x - 1, y, layer)) left = true;

				int maskTable[16] = {
					!up && !left && down && right , !up && left && down && right , !up && left && down && !right , !up && !left && down && !right ,
					up && !left && down && right , up && left && down && right , up && left && down && !right , up && !left && down && !right ,
					up && !left && !down && right , up && left && !down && right ,up && left && !down && !right ,up && !left && !down && !right ,
				   !up && !left && !down && right ,!up && left && !down && right ,!up && left && !down && !right ,!up && !left && !down && !right
				};

				int mask = 0;

				while (!maskTable[mask]) {
					mask++;
				}

				// 4x4グリッド前提。1セルのサイズはテクスチャサイズから算出
				const int cols = 4;
				const int rows = 4;

				const int cellW = texW / cols;
				const int cellH = texH / rows;

				// 不正サイズならフォールバック（全体）
				if (cellW > 0 && cellH > 0) {
					srcX = (mask % cols) * cellW;
					srcY = (mask / cols) * cellH;
					srcW = cellW;
					srcH = cellH;

					// 念のため範囲チェック
					if (srcX < 0 || srcY < 0 || srcX + srcW > texW || srcY + srcH > texH) {
						srcX = 0; srcY = 0; srcW = texW; srcH = texH;
					}
				}
			}

			Novice::DrawQuad(
				static_cast<int>(sLT.x), static_cast<int>(sLB.y),
				static_cast<int>(sRT.x), static_cast<int>(sRB.y),
				static_cast<int>(sLB.x), static_cast<int>(sLT.y),
				static_cast<int>(sRB.x), static_cast<int>(sRT.y),
				srcX, srcY, srcW, srcH,
				handle,
				0xFFFFFFFF
			);
		}
	}
}