#include "FontAtlas.h"
#include <Novice.h>
#include <fstream>
#include <sstream>
#include <algorithm>

static std::string ReadFileAll(const std::string& path) {
	std::ifstream ifs(path);
	std::ostringstream oss;
	oss << ifs.rdbuf();
	return oss.str();
}

// key=value 抽出補助
static bool ExtractInt(const std::string& src, const std::string& key, int& out) {
	size_t p = src.find(key + "=");
	if (p == std::string::npos) return false;
	p += key.size() + 1;
	size_t e = src.find_first_of(" \r\n", p);
	std::string num = src.substr(p, e - p);
	out = std::stoi(num);
	return true;
}

bool FontAtlas::Load(const std::string& fntPath, const std::string& pngPath) {
	std::string txt = ReadFileAll(fntPath);
	if (txt.empty()) return false;
	if (!ParseFnt(txt)) return false;
	// テクスチャ読み込み
	textureHandle_ = Novice::LoadTexture(pngPath.c_str());
	if (textureHandle_ <= 0) return false;
	ComputeUV();
	return true;
}

bool FontAtlas::ParseFnt(const std::string& text) {
	std::istringstream iss(text);
	std::string line;
	while (std::getline(iss, line)) {
		if (line.rfind("common ", 0) == 0) {
			ExtractInt(line, "lineHeight", lineHeight_);
			ExtractInt(line, "base", baseLine_);
			ExtractInt(line, "scaleW", texW_);
			ExtractInt(line, "scaleH", texH_);
		} else if (line.rfind("char ", 0) == 0) {
			Glyph g;
			ExtractInt(line, "id", g.id);
			ExtractInt(line, "x", g.x);
			ExtractInt(line, "y", g.y);
			ExtractInt(line, "width", g.w);
			ExtractInt(line, "height", g.h);
			ExtractInt(line, "xoffset", g.xoffset);
			ExtractInt(line, "yoffset", g.yoffset);
			ExtractInt(line, "xadvance", g.xadvance);
			if (g.id >= 0) {
				glyphs_[g.id] = g;
			}
		}
	}
	return !glyphs_.empty() && texW_ > 0 && texH_ > 0;
}

void FontAtlas::ComputeUV() {
	if (texW_ <= 0 || texH_ <= 0) return;
	for (auto& kv : glyphs_) {
		Glyph& g = kv.second;
		g.u0 = (float)g.x / texW_;
		g.v0 = (float)g.y / texH_;
		g.u1 = (float)(g.x + g.w) / texW_;
		g.v1 = (float)(g.y + g.h) / texH_;
	}
}

const Glyph* FontAtlas::GetGlyph(int codepoint) const {
	auto it = glyphs_.find(codepoint);
	if (it == glyphs_.end()) return nullptr;
	return &it->second;
}