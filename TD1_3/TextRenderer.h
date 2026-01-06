#pragma once
#include "FontAtlas.h"
#include <string>
#include <cstdint>

class TextRenderer {
public:
	void SetFont(FontAtlas* atlas) { atlas_ = atlas; }
	void DrawTextLabel(int x, int y, const std::string& text, uint32_t color, float scale = 1.0f, int tracking = 0);
	int  MeasureWidth(const std::string& text, float scale = 1.0f, int tracking = 0) const;
private:
	FontAtlas* atlas_ = nullptr;
};
