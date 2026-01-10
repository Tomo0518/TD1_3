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
	PlayerAnimeNormal,


	// ==========================================
	// ゲームシーン背景
	// ==========================================
	Background0_0,
	BackgroundBlack,
	Background0_2,
	Background1_0,
	Background1_1,
	Background1_2,
	Background2_0,
	Background2_1,
	Background2_2,
	Background3_0,
	Background3_1,
	Background3_2,



	Count,	// 最後
};

class TextureManager {
public:
	TextureManager();
	~TextureManager() = default;

	static TextureManager& GetInstance() {
		static TextureManager instance;
		return instance;
	}

	// 削除・コピー禁止
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	// テクスチャハンドル取得
	int GetTexture(TextureId id) const;

private:
	void LoadResources();
	std::array<int, static_cast<int>(TextureId::Count)> textureResources_;
};