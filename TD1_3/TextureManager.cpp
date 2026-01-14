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
	// マップチップ
	// ==================================
	textureResources_[static_cast<int>(TextureId::GroundAuto)] =
		Novice::LoadTexture("./Resources/images/mapChip/tile_temp3.png");	

	// ==================================
	// ゲームオブジェクト
	// ==================================

	// =========Player ==========
	textureResources_[static_cast<int>(TextureId::PlayerAnimeNormal)] =
		Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");

	// デコレーション

	textureResources_[static_cast<int>(TextureId::Deco_Scrap)] = 
		Novice::LoadTexture("./Resources/images/mapChip/decoration/scrap_supplystation.png");


	// =========Background ==========
	textureResources_[static_cast<int>(TextureId::Background0_0)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background0_0.png");

	textureResources_[static_cast<int>(TextureId::BackgroundBlack)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background_black.png");

	textureResources_[static_cast<int>(TextureId::Background0_2)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background0_2.png");

	textureResources_[static_cast<int>(TextureId::Background1_0)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background1_0.png");

	textureResources_[static_cast<int>(TextureId::Background1_1)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background1_1.png");

	textureResources_[static_cast<int>(TextureId::Background1_2)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background1_2.png");

	textureResources_[static_cast<int>(TextureId::Background2_0)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background2_0.png");

	textureResources_[static_cast<int>(TextureId::Background2_1)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background2_1.png");

	textureResources_[static_cast<int>(TextureId::Background2_2)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/background2_2.png");

	textureResources_[static_cast<int>(TextureId::None)] =
		Novice::LoadTexture("./Resources/images/temp/none.png");
}