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

	// ===========================================
	// tutorial image
	// ===========================================
	textureResources_[static_cast<int>(TextureId::JumpTutorial)] =
		Novice::LoadTexture("./Resources/images/tutorial/jumpTutorial.png");
	textureResources_[static_cast<int>(TextureId::DashTutorial)] =
		Novice::LoadTexture("./Resources/images/tutorial/dashTutorial.png");
	textureResources_[static_cast<int>(TextureId::ThrowTutorial)] =
		Novice::LoadTexture("./Resources/images/tutorial/throwTutorial.png");
	textureResources_[static_cast<int>(TextureId::BoomerangTutorial)] =
		Novice::LoadTexture("./Resources/images/tutorial/boomerangTutorial.png");

	// ==========================================
	// enemy
	// ==========================================
	textureResources_[static_cast<int>(TextureId::KinokoWalk)] =
		Novice::LoadTexture("./Resources/images/enemy/kinoko_walk.png");
	textureResources_[static_cast<int>(TextureId::KinokoStun)] =
		Novice::LoadTexture("./Resources/images/enemy/kinoko_stun.png");

	textureResources_[static_cast<int>(TextureId::AttackKinokoWalk)] =
		Novice::LoadTexture("./Resources/images/enemy/attackKinoko_walk.png");
	textureResources_[static_cast<int>(TextureId::AttackKinokoStun)] =
		Novice::LoadTexture("./Resources/images/enemy/attackKinoko_stun.png");
	textureResources_[static_cast<int>(TextureId::AttackKinokoAttack)] =
		Novice::LoadTexture("./Resources/images/enemy/attackKinoko_attack.png");
	textureResources_[static_cast<int>(TextureId::AttackKinokoWindup)] =
		Novice::LoadTexture("./Resources/images/enemy/attackKinoko_windup.png");
	textureResources_[static_cast<int>(TextureId::AttackKinokoBattleIdle)] =
		Novice::LoadTexture("./Resources/images/enemy/attackKinoko_battleIdle.png");
	textureResources_[static_cast<int>(TextureId::AttackKinokoRun)] =
		Novice::LoadTexture("./Resources/images/enemy/attackKinoko_run.png");


	// ==========================================
	// item
	// ==========================================
	textureResources_[static_cast<int>(TextureId::Boomerang)] =
		Novice::LoadTexture("./Resources/images/item/boomerang.png");
	textureResources_[static_cast<int>(TextureId::Boomerang_ChargedLv1)] =
		Novice::LoadTexture("./Resources/images/item/boomerangBonusLv1.png");
	textureResources_[static_cast<int>(TextureId::Boomerang_ChargedLv2)] =
		Novice::LoadTexture("./Resources/images/item/boomerangBonusLv2.png");
	textureResources_[static_cast<int>(TextureId::Boomerang_ChargedLv3)] =
		Novice::LoadTexture("./Resources/images/item/boomerangBonusLv3.png");

	textureResources_[static_cast<int>(TextureId::Star_idle)] =
		Novice::LoadTexture("./Resources/images/item/star_Idle.png");
	textureResources_[static_cast<int>(TextureId::Star_shooting)] =
		Novice::LoadTexture("./Resources/images/item/star_shooting.png");

	textureResources_[static_cast<int>(TextureId::Mystery)] =
		Novice::LoadTexture("./Resources/images/item/mystery.png");

	textureResources_[static_cast<int>(TextureId::Button_On)] =
		Novice::LoadTexture("./Resources/images/item/buttonOn.png");
	textureResources_[static_cast<int>(TextureId::Button_Off)] =
		Novice::LoadTexture("./Resources/images/item/buttonOff.png");
	textureResources_[static_cast<int>(TextureId::Door_Open)] =
		Novice::LoadTexture("./Resources/images/item/doorOn.png");
	textureResources_[static_cast<int>(TextureId::Door_Closed)] =
		Novice::LoadTexture("./Resources/images/item/doorOff.png");

	// =========================================
	// usagi
	// =========================================
	textureResources_[static_cast<int>(TextureId::UsagiIdle)] =
		Novice::LoadTexture("./Resources/images/usagi/usagi_idle.png");
	textureResources_[static_cast<int>(TextureId::UsagiBreathe)] =
		Novice::LoadTexture("./Resources/images/usagi/usagi_breathe.png");
	textureResources_[static_cast<int>(TextureId::UsagiRun)] =
		Novice::LoadTexture("./Resources/images/usagi/usagi_running.png");
	textureResources_[static_cast<int>(TextureId::UsagiAttack)] =
		Novice::LoadTexture("./Resources/images/usagi/usagi_attack.png");
	textureResources_[static_cast<int>(TextureId::UsagiJump)] =
		Novice::LoadTexture("./Resources/images/usagi/usagi_jump.png");
	textureResources_[static_cast<int>(TextureId::UsagiFall)] =
		Novice::LoadTexture("./Resources/images/usagi/usagi_falling.png");

	textureResources_[static_cast<int>(TextureId::BoomerangIdle)] =
		Novice::LoadTexture("./Resources/images/usagi/boomerang_idle.png");
	textureResources_[static_cast<int>(TextureId::BoomerangBreathe)] =
		Novice::LoadTexture("./Resources/images/usagi/boomerang_breathe.png");
	textureResources_[static_cast<int>(TextureId::BoomerangRun)] =
		Novice::LoadTexture("./Resources/images/usagi/boomerang_running.png");
	textureResources_[static_cast<int>(TextureId::BoomerangAttack)] =
		Novice::LoadTexture("./Resources/images/usagi/boomerang_attack.png");
	textureResources_[static_cast<int>(TextureId::BoomerangJump)] =
		Novice::LoadTexture("./Resources/images/usagi/boomerang_jump.png");
	textureResources_[static_cast<int>(TextureId::BoomerangFall)] =
		Novice::LoadTexture("./Resources/images/usagi/boomerang_falling.png");

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

	// =========================================
	// デコレーション
	// =========================================

	textureResources_[static_cast<int>(TextureId::Deco_Scrap)] =
		Novice::LoadTexture("./Resources/images/mapChip/decoration/scrap_supplystation.png");

	textureResources_[static_cast<int>(TextureId::Deco_Grass)] =
		Novice::LoadTexture("./Resources/images/mapChip/decoration/gras.png");

	textureResources_[static_cast<int>(TextureId::Deco_GrassAnim)] =
		Novice::LoadTexture("./Resources/images/mapChip/decoration/grass_anim.png");

	// ========Background Decoration==========
	textureResources_[static_cast<int>(TextureId::Deco_Background_RockBlock)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/decoration/background_rockblock.png");
	textureResources_[static_cast<int>(TextureId::Deco_Background_IceBlock)] =
		Novice::LoadTexture("./Resources/images/gamePlay/background/decoration/background_iceblock.png");

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

		textureResources_[static_cast<int>(TextureId::Background_Base)] =
			Novice::LoadTexture("./Resources/images/gamePlay/background/base.png");
		textureResources_[static_cast<int>(TextureId::Background_Far)] =
			Novice::LoadTexture("./Resources/images/gamePlay/background/far.png");
		textureResources_[static_cast<int>(TextureId::Background_Middle)] =
			Novice::LoadTexture("./Resources/images/gamePlay/background/middle.png");
		textureResources_[static_cast<int>(TextureId::Background_Near)] =
			Novice::LoadTexture("./Resources/images/gamePlay/background/near.png");
		textureResources_[static_cast<int>(TextureId::Background_Foreground)] =
			Novice::LoadTexture("./Resources/images/gamePlay/background/bgFilter.png");

	}

	// ==========================================================
	// エフェクト用テクスチャ
	// ==========================================================
	{
		textureResources_[static_cast<int>(TextureId::Particle_Explosion)] =
			Novice::LoadTexture("./Resources/images/effect/explosion.png");

		textureResources_[static_cast<int>(TextureId::Particle_Debris)] =
			Novice::LoadTexture("./Resources/images/effect/debris.png");

		textureResources_[static_cast<int>(TextureId::Particle_Hit)] =
			Novice::LoadTexture("./Resources/images/effect/star.png");

		textureResources_[static_cast<int>(TextureId::Particle_Enemy_HitSmoke)] =
			Novice::LoadTexture("./Resources/images/effect/smoke.png");

		textureResources_[static_cast<int>(TextureId::Particle_Enemy_Dead)] =
			Novice::LoadTexture("./Resources/images/effect/snow.png");

		textureResources_[static_cast<int>(TextureId::Particle_Dust)] =
			Novice::LoadTexture("./Resources/images/effect/star.png");

		textureResources_[static_cast<int>(TextureId::Particle_Rain)] =
			Novice::LoadTexture("./Resources/images/effect/rain.png");

		textureResources_[static_cast<int>(TextureId::Particle_Snow)] =
			Novice::LoadTexture("./Resources/images/effect/snow.png");

		textureResources_[static_cast<int>(TextureId::Particle_Orb)] =
			Novice::LoadTexture("./Resources/images/effect/orb.png");

		textureResources_[static_cast<int>(TextureId::Particle_Glow)] =
			Novice::LoadTexture("./Resources/images/effect/particle_output/particle_glow.png");

		textureResources_[static_cast<int>(TextureId::Particle_Ring)] =
			Novice::LoadTexture("./Resources/images/effect/particle_output/particle_ring.png");

		textureResources_[static_cast<int>(TextureId::Particle_Sparkle)] =
			Novice::LoadTexture("./Resources/images/effect/particle_output/particle_sparkle.png");

		textureResources_[static_cast<int>(TextureId::Particle_Scratch)] =
			Novice::LoadTexture("./Resources/images/effect/particle_output/particle_scratch.png");

		textureResources_[static_cast<int>(TextureId::Particle_Smoke)] =
			Novice::LoadTexture("./Resources/images/effect/particle_output/particle_smoke.png");

		// ==========敵用エフェクト==========
		textureResources_[static_cast<int>(TextureId::Particle_EnemyHit)] =
			Novice::LoadTexture("./Resources/images/enemy/particle/hit.png");

		textureResources_[static_cast<int>(TextureId::Particle_EnemyCharge)] =
			Novice::LoadTexture("./Resources/images/enemy/particle/charge.png");
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

		// UI: HP
		//PlayerHPFrame, PlayerHPBar,
		//	BossHPFrame, BossHPBar,

		//	// UI: Key Guide
		//	KeyW, KeyA, KeyS, KeyD,
		//	KeySpace, KeyEnter, KeyEsc,

		//	// UI: Scenes
		//	PauseText, PauseBg,
		//	ResultClear, ResultOver,

		// ====================================
		// UI
		// ====================================
		textureResources_[static_cast<int>(TextureId::PlayerHPFrame)] =
			Novice::LoadTexture("./Resources/images/ui/Gauge/player_hp_frame.png");
		textureResources_[static_cast<int>(TextureId::PlayerHPBar)] =
			Novice::LoadTexture("./Resources/images/ui/Gauge/player_hp_bar.png");

		// =========== Key Guide ==========
		textureResources_[static_cast<int>(TextureId::KeyW)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/w.png");
		textureResources_[static_cast<int>(TextureId::KeyA)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/a.png");
		textureResources_[static_cast<int>(TextureId::KeyS)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/s.png");
		textureResources_[static_cast<int>(TextureId::KeyD)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/d.png");
		textureResources_[static_cast<int>(TextureId::KeyK)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/k.png");
		textureResources_[static_cast<int>(TextureId::KeyJ)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/j.png");


		textureResources_[static_cast<int>(TextureId::KeySpace)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/space.png");
		textureResources_[static_cast<int>(TextureId::KeyEnter)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/enter.png");
		textureResources_[static_cast<int>(TextureId::KeyEsc)] =
			Novice::LoadTexture("./Resources/images/ui/keyGuide/esc.png");
		textureResources_[static_cast<int>(TextureId::PauseText)] =
			Novice::LoadTexture("./Resources/images/ui/pause.png");

		// ========= Icon ==========
		textureResources_[static_cast<int>(TextureId::Icon_BoomerangReturn)] =
			Novice::LoadTexture("./Resources/images/ui/icon/boomerang_return.png");
		textureResources_[static_cast<int>(TextureId::Icon_BoomerangThrow)] =
			Novice::LoadTexture("./Resources/images/ui/icon/boomerang_throw.png");
		textureResources_[static_cast<int>(TextureId::Icon_Dash)] =
			Novice::LoadTexture("./Resources/images/ui/icon/dash.png");


		/*textureResources_[static_cast<int>(TextureId::PauseBg)] =
			Novice::LoadTexture("./Resources/images/ui/pause_bg.png");
		textureResources_[static_cast<int>(TextureId::ResultClear)] =
			Novice::LoadTexture("./Resources/images/ui/scenes/result_clear.png");
		textureResources_[static_cast<int>(TextureId::ResultOver)] =
			Novice::LoadTexture("./Resources/images/ui/scenes/result_over.png");*/


		
	}
}