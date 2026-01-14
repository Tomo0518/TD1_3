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

bool MapChip::IsSameTile(int myID, int tx, int ty) const {
	if (!mapData_) return false;
	if (tx < 0 || tx >= mapData_->GetWidth() ||
		ty < 0 || ty >= mapData_->GetHeight()) {
		return true;
	}
	return (mapData_->GetTile(tx, ty) == myID);
}

void MapChip::Draw( Camera2D& camera) {
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
			const int tileID = mapData_->GetTile(x, y);
			if (tileID == 0) continue;

			// tileID -> texture handle（キャッシュ）
			const auto it = textureCache_.find(tileID);
			if (it == textureCache_.end()) continue;

			const int handle = it->second;
			if (handle < 0) continue;

			// 画像サイズ取得（等倍→tileSizeへ合わせるために使う）
			int texW = 0, texH = 0;
			Novice::GetTextureSize(handle, &texW, &texH);
			if (texW <= 0 || texH <= 0) continue;

			// タイルのワールド4頂点（左上基準）
			const float left = x * tileSize;
			const float top = y * tileSize;
			const float right = left + tileSize;
			const float bottom = top + tileSize;

			const Vector2 wLT = { left, top };
			const Vector2 wRT = { right, top };
			const Vector2 wLB = { left, bottom };
			const Vector2 wRB = { right, bottom };

			// カメラ変換（回転・ズーム含む）
			const Vector2 sLT = Matrix3x3::Transform(wLT, vpVp);
			const Vector2 sRT = Matrix3x3::Transform(wRT, vpVp);
			const Vector2 sLB = Matrix3x3::Transform(wLB, vpVp);
			const Vector2 sRB = Matrix3x3::Transform(wRB, vpVp);

			// 個別テクスチャなのでソース矩形は画像全体
			Novice::DrawQuad(
				static_cast<int>(sLT.x), static_cast<int>(sLT.y),
				static_cast<int>(sRT.x), static_cast<int>(sRT.y),
				static_cast<int>(sLB.x), static_cast<int>(sLB.y),
				static_cast<int>(sRB.x), static_cast<int>(sRB.y),
				0, 0, texW, texH,
				handle,
				0xFFFFFFFF
			);
		}
	}
}