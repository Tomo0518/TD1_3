#pragma once
#include "GameSceneBase.h"
#include "Pad.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include "GameShared.h"
#include <Novice.h>

class SceneManager;

class SettingScene : public GameSceneBase {
public:

	enum class Item {
		BGM,
		SE,
		VIB_ENABLE,
		VIB_STRENGTH,
		BACK
	};

	SettingScene(SceneManager& mgr, GameShared& shared);
	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

	float GetBgmVolume() const { return bgmVolume_; }
	float GetSeVolume()  const { return seVolume_; }

	int grHandleFrame = Novice::LoadTexture("./Resources/images/explanation/frame.png");

private:
	SceneManager& manager_;
	GameShared& shared_;

	Item focus_ = Item::BGM;

	int bgmStep_{ 10 };
	int seStep_{ 10 };
	int vibStrengthStep_{ 10 };
	bool vibEnable_{ true };
	float bgmVolume_{ 1.0f };
	float seVolume_{ 1.0f };
	float vibStrength_{};

	// アナログ閾値
	float prevLX_ = 0.0f;
	float prevLY_ = 0.0f;
	bool firstFrame_ = true;

	// フォント
	FontAtlas font_;
	TextRenderer text_;
	bool fontReady_ = false;

	void ApplyStepsToFloat();
	void ChangeFocus(int dir);     // -1 / +1
	void AdjustCurrent(int dir);   // -1 / +1
	void Leave(bool apply);


};