#include "TextRenderer.h"
#include <Novice.h>
#include <cstdio>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef DrawText
#undef DrawText   // Windows API マクロ無効化
#endif

// ASCIIで1文字描画
void TextRenderer::DrawTextLabel(int x, int y, const std::string& text, uint32_t color, float scale, int tracking) {
	if (!atlas_) { OutputDebugStringA("TextRenderer: atlas null\n"); return; }
	if (atlas_->GetTextureHandle() <= 0) { OutputDebugStringA("TextRenderer: texture handle invalid\n"); return; }

	const int texW = atlas_->GetTexW();
	const int texH = atlas_->GetTexH();
	if (texW <= 0 || texH <= 0) { OutputDebugStringA("TextRenderer: atlas tex size 0\n"); return; }

	int lineH = int(atlas_->GetLineHeight() * scale);
	if (lineH <= 0) {
		// fallback: グリフ最大高さを推定 (初回だけ重いならキャッシュ化)
		lineH = int(16 * scale); // 簡易。必要なら最大 g->h を走査
	}

	int penX = x;
	int penY = y;
	int drawnCount = 0;

	for (unsigned char ch : text) {
		if (ch == '\n') {
			penX = x;
			penY += lineH;
			continue;
		}
		const Glyph* g = atlas_->GetGlyph(ch);
		if (!g) continue;

		int destX = penX + int(g->xoffset * scale);
		int destY = penY + int(g->yoffset * scale);

		float scaleX = (g->w > 0) ? (g->w * scale) / float(texW) : 0.0f;
		float scaleY = (g->h > 0) ? (g->h * scale) / float(texH) : 0.0f;

		Novice::DrawSpriteRect(
			destX, destY,
			g->x, g->y, g->w, g->h,
			atlas_->GetTextureHandle(),
			scaleX, scaleY,
			0.0f,
			color
		);

		penX += int(g->xadvance * scale) + tracking;
		++drawnCount;
	}

#ifdef _DEBUG
	if (drawnCount == 0 && !text.empty()) {
		OutputDebugStringA(("TextRenderer: drawnCount=0 text=\"" + std::string(text) + "\"\n").c_str());
	}
#endif
}

int TextRenderer::MeasureWidth(const std::string& text, float scale, int tracking) const {
	if (!atlas_) return 0;
	int w = 0;
	int maxW = 0;
	for (unsigned char ch : text) {
		if (ch == '\n') {
			maxW = std::max(maxW, w);
			w = 0;
			continue;
		}
		const Glyph* g = atlas_->GetGlyph(ch);
		if (!g) continue;
		w += int(g->xadvance * scale) + tracking;
	}
	maxW = std::max(maxW, w);
	return maxW;
}