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
	// tutorial image
	// =========================================
	JumpTutorial,
	DashTutorial,
	ThrowTutorial,
	BoomerangTutorial,

	// =========================================
	// enemy
	// =========================================
	KinokoWalk,
	KinokoStun,

	AttackKinokoWalk,
	AttackKinokoStun,
	AttackKinokoAttack,
	AttackKinokoWindup,
	AttackKinokoBattleIdle,
	AttackKinokoRun,

	// =========================================
	// item
	// =========================================
	Boomerang,
	Boomerang_ChargedLv1,
	Boomerang_ChargedLv2,
	Boomerang_ChargedLv3,
	Star_idle,
	Star_shooting,
	Mystery,

	Button_On,
	Button_Off,
	Door_Open,
	Door_Closed,

	// =========================================
	// usagi
	// =========================================
	UsagiIdle,
	UsagiBreathe,
	UsagiRun,
	UsagiAttack,
	UsagiJump,
	UsagiFall,

	BoomerangIdle,
	BoomerangBreathe,
	BoomerangRun,
	BoomerangAttack,
	BoomerangJump,
	BoomerangFall,

	// =========================================
	// マップチップ用テクスチャ
	// =========================================
	GroundAuto, // 地面オートタイル

	// ========================================
	// デコレーション用
	// ========================================
	Deco_Scrap,
	Deco_Sign,
	Deco_Grass,
	Deco_GrassAnim,
	Deco_Rock1,

	// ========================================
	// デコレーション用
	// ========================================
	Deco_Background_RockBlock, // 背景岩ブロック
	Deco_Background_IceBlock, // 背景つらら

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

	Background_Base,
	Background_Far,
	Background_Middle,
	Background_Near,
	Background_Foreground,

	// ========================================
	// パーティクルテクスチャ
	// ========================================
	Particle_Explosion,
	Particle_Debris,
	Particle_Hit,
	Particle_Enemy_HitSmoke,
	Particle_Enemy_Dead,
	Particle_Dust,
	Particle_Rain,
	Particle_Snow,
	Particle_Orb,
	Particle_Glow,
	Particle_Ring,
	Particle_Sparkle,
	Particle_Scratch,
	Particle_Smoke,

	Particle_EnemyHit,
	Particle_EnemyCharge,

	// ========================================
	// UI用テクスチャ
	// ========================================
	// Icon
	Icon_BoomerangReturn,Icon_BoomerangThrow,
	Icon_Dash,

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

	// UI: HP
	PlayerHPFrame, PlayerHPBar,
	BossHPFrame, BossHPBar,

	// UI: Key Guide
	KeyW, KeyA, KeyS, KeyD, KeyK, KeyJ,
	KeySpace, KeyEnter, KeyEsc,

	KeyDush, KeyThrow, KeyJump,
	// Pad Guide
	PadStickUp, PadStickDown, PadStickLeft, PadStickRight,
	PadButtonA, PadButtonB, PadButtonX, PadButtonY,
	PadMove, PadDash, PadThrow, PadJump,

	// UI: Scenes
	PauseText, PauseBg,
	ResultClear, ResultOver,

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