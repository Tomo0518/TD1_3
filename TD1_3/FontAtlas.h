#pragma once
#include <string>
#include <unordered_map>

struct Glyph {
	int id = 0;
	int x = 0, y = 0;       // アトラス上位置
	int w = 0, h = 0;       // サイズ
	int xoffset = 0, yoffset = 0;
	int xadvance = 0;
	float u0 = 0.0f, v0 = 0.0f;
	float u1 = 0.0f, v1 = 0.0f;
};

class FontAtlas {
public:
	bool Load(const std::string& fntPath, const std::string& pngPath);

	const Glyph* GetGlyph(int codepoint) const;
	int GetLineHeight() const { return lineHeight_; }
	int GetTextureHandle() const { return textureHandle_; }
	int GetTexW() const { return texW_; }
	int GetTexH() const { return texH_; }
private:
	bool ParseFnt(const std::string& text);
	void ComputeUV();

	std::unordered_map<int, Glyph> glyphs_;
	int lineHeight_ = 0;
	int baseLine_ = 0;
	int texW_ = 0;
	int texH_ = 0;
	int textureHandle_ = -1;
};