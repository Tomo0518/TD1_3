#include "TextureManager.h"

TextureManager::TextureManager() {
	textureResources_.fill(-1);
	LoadResources();
}

int TextureManager::GetTexture(TextureId id) const {
	return textureResources_[static_cast<int>(id)];
}

void TextureManager::LoadResources() {
	// 共通
	textureResources_[static_cast<int>(TextureId::White1x1)] =
		Novice::LoadTexture("./NoviceResources/white1x1.png");

	// Title
	textureResources_[static_cast<int>(TextureId::TitleBackground)] =
		Novice::LoadTexture("./Resources/images/title/background_ver1.png");
	textureResources_[static_cast<int>(TextureId::TitleLogo)] =
		Novice::LoadTexture("./Resources/images/title/logo_ver1.png");

	// StageSelect
	textureResources_[static_cast<int>(TextureId::StageSelectBackground)] =
		Novice::LoadTexture("./Resources/images/stageSelect/background_ver1.png");

	// Result
	textureResources_[static_cast<int>(TextureId::ResultBackground)] =
		Novice::LoadTexture("./Resources/images/result/result_sky.png");
	textureResources_[static_cast<int>(TextureId::ResultClearLabel)] =
		Novice::LoadTexture("./Resources/images/result/clear.png");


	// ==================================
	// ゲームオブジェクト
	// ==================================

	// =========Player ==========
	textureResources_[static_cast<int>(TextureId::PlayerAnimeNormal)] =
		Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
}