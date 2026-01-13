#include "MapChip.h"
#include "TileRegistry.h"

MapChip::MapChip() {
}

MapChip::~MapChip() {
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
	const int width = mapData_->GetWidth();
	const int height = mapData_->GetHeight();
	const float tileSize = mapData_->GetTileSize();

	// タイルシートのサイズ（自動分割用）
	int texW = 0;
	int texH = 0;
	Novice::GetTextureSize(textureHandle_, &texW, &texH);

	const int srcTileSize = static_cast<int>(textureSrcSize_);
	const int tilesPerRow = (srcTileSize > 0) ? (texW / srcTileSize) : 0;
	if (tilesPerRow <= 0) {
		return;
	}

	// カメラの表示範囲を取得（カリング用）
	Vector2 cameraTopLeft = camera.GetTopLeft();
	Vector2 cameraBottomRight = camera.GetBottomRight();

	// 描画すべきグリッド範囲を計算（画面外はループすら回さない）
	int startX = static_cast<int>(cameraTopLeft.x / tileSize);
	startX = (startX < 0) ? 0 : startX;

	int endX = static_cast<int>(cameraBottomRight.x / tileSize) + 1;
	endX = (endX > width) ? width : endX;

	int startY = static_cast<int>(cameraTopLeft.y / tileSize);
	startY = (startY < 0) ? 0 : startY;

	int endY = static_cast<int>(cameraBottomRight.y / tileSize) + 1;
	endY = (endY > height) ? height : endY;

	// カメラ行列（回転・ズーム込み）
	const Matrix3x3 vpVp = camera.GetVpVpMatrix();

	// 範囲内だけループして描画
	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			const int tileID = mapData_->GetTile(x, y);
			if (tileID == 0) continue;

			const TileDefinition* def = TileRegistry::GetTile(tileID);
			if (!def) continue;
			if (def->textureIndex < 0) continue;

			// textureIndex -> src rect（画像サイズから列数を算出）
			const int idx = def->textureIndex;

			const int srcX = (idx % tilesPerRow) * srcTileSize;
			const int srcY = (idx / tilesPerRow) * srcTileSize;

			// 範囲外（タイル数超過）なら描かない
			if (srcX < 0 || srcY < 0) continue;
			if (srcX + srcTileSize > texW) continue;
			if (srcY + srcTileSize > texH) continue;

			// タイルのワールド4頂点（左上基準）
			const float left = x * tileSize;
			const float top = y * tileSize;
			const float right = left + tileSize;
			const float bottom = top + tileSize;

			const Vector2 wLT = { left, top };
			const Vector2 wRT = { right, top };
			const Vector2 wLB = { left, bottom };
			const Vector2 wRB = { right, bottom };

			// カメラ変換してスクリーンへ（回転・ズーム込み）
			const Vector2 sLT = Matrix3x3::Transform(wLT, vpVp);
			const Vector2 sRT = Matrix3x3::Transform(wRT, vpVp);
			const Vector2 sLB = Matrix3x3::Transform(wLB, vpVp);
			const Vector2 sRB = Matrix3x3::Transform(wRB, vpVp);

			Novice::DrawQuad(
				static_cast<int>(sLT.x), static_cast<int>(sLT.y),
				static_cast<int>(sRT.x), static_cast<int>(sRT.y),
				static_cast<int>(sLB.x), static_cast<int>(sLB.y),
				static_cast<int>(sRB.x), static_cast<int>(sRB.y),
				srcX, srcY, srcTileSize, srcTileSize,
				textureHandle_,
				0xFFFFFFFF
			);
		}
	}
}