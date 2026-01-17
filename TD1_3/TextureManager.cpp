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
		Novice::LoadTexture("./Resources/images/mapChip/tile_temp7.png");

	// ==================================
	// ゲームオブジェクト
	// ==================================

	// =========Player ==========
	textureResources_[static_cast<int>(TextureId::PlayerAnimeNormal)] =
		Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");

	// デコレーション

	textureResources_[static_cast<int>(TextureId::Deco_Scrap)] =
		Novice::LoadTexture("./Resources/images/mapChip/decoration/scrap_supplystation.png");

	textureResources_[static_cast<int>(TextureId::Deco_Grass)] =
		Novice::LoadTexture("./Resources/images/mapChip/decoration/gras.png");


	// =========Background ==========
	{
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



	// ==========================================================
	// UI用テクスチャ
	// ==========================================================
	// ========== ボタン ==========
	{
		// Playボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_Play)] =
			Novice::LoadTexture("./Resources/images/ui/button/play_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_Play_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/play_selected.png");

		// Quitボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_Quit)] =
			Novice::LoadTexture("./Resources/images/ui/button/quit_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_Quit_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/quit_selected.png");

		// Settingsボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_Settings)] =
			Novice::LoadTexture("./Resources/images/ui/button/setting_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_Settings_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/setting_selected.png");

		// StageSelectボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_StageSelect)] =
			Novice::LoadTexture("./Resources/images/ui/button/notTexture_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_StageSelect_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/notTexture_selected.png");

		// Resumeボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_Resume)] =
			Novice::LoadTexture("./Resources/images/ui/button/resume_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_Resume_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/resume_selected.png");

		// Retryボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_Retry)] =
			Novice::LoadTexture("./Resources/images/ui/button/retry_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_Retry_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/retry_selected.png");

		// Titleボタン
		textureResources_[static_cast<int>(TextureId::UI_Button_Title)] =
			Novice::LoadTexture("./Resources/images/ui/button/title_default.png");
		textureResources_[static_cast<int>(TextureId::UI_Button_Title_Selected)] =
			Novice::LoadTexture("./Resources/images/ui/button/title_selected.png");

		// Pauseボタン
		//textureResources_[static_cast<int>(TextureId::UI_Button_Pause)] =
		//	Novice::LoadTexture("./Resources/images/ui/button/pause_default.png");
		//textureResources_[static_cast<int>(TextureId::UI_Button_Pause_Selected)] =
		//	Novice::LoadTexture("./Resources/images/ui/button/pause_selected.png");

		//textureResources_[static_cast<int>(TextureId::UI_Button_StageSelect)] =
		//	Novice::LoadTexture("./Resources/images/ui/button/stageSelect_default.png");
	}
}