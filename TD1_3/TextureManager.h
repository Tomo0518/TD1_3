#pragma once
#include <Novice.h>
#include <array>

// テクスチャの種類（ID）
enum class TextureId {
	White1x1,

	TitleBackground,
	TitleLogo,

	StageSelectBackground,

	ResultBackground,
	ResultClearLabel,

	Count
};

class TextureManager {
public:
	TextureManager();
	~TextureManager() = default;

	static TextureManager& GetInstance() {
		static TextureManager instance;
		return instance;
	}

	// テクスチャハンドル取得
	int GetTexture(TextureId id) const;

private:
	void LoadResources();

private:
	std::array<int, static_cast<int>(TextureId::Count)> textureResources_;
};