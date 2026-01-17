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

	// =========================================
	// マップチップ用テクスチャ
	// =========================================
	GroundAuto, // 地面オートタイル

	// ========================================
	// デコレーション用
	// ========================================
	Deco_Scrap,
	Deco_Grass,


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

	// ========================================
	// UI用テクスチャ
	// ========================================

	// ========== ボタン ==========
	UI_Button_Play,
	UI_Button_Play_Selected,

	UI_Button_StageSelect,
	UI_Button_StageSelect_Selected,

	UI_Button_Settings,
	UI_Button_Settings_Selected,

	UI_Button_Quit,
	UI_Button_Quit_Selected,

	UI_Button_Pause,
	UI_Button_Pause_Selected,

	UI_Button_Resume,
	UI_Button_Resume_Selected,

	UI_Button_Retry,
	UI_Button_Retry_Selected,

	UI_Button_Title,
	UI_Button_Title_Selected,

	None,		// テクスチャなし用
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

	void LoadResources();

private:
	std::array<int, static_cast<int>(TextureId::Count)> textureResources_;
};