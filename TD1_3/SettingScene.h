#pragma once
#include "IGameScene.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include "DrawComponent2D.h"

class SceneManager;

class SettingScene : public IGameScene {
public:
	enum class Item {
		BGM,
		SE,
		VIB_ENABLE,
		VIB_STRENGTH,
		BACK
	};

	explicit SettingScene(SceneManager& mgr);

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

private:
	SceneManager& manager_;

	// UI
	Item focus_ = Item::BGM;

	int bgmStep_ = 10;          // 0..10
	int seStep_ = 10;           // 0..10
	int vibStrengthStep_ = 10;  // 1..10

	// 入力（スティックのトリガ判定用）
	float prevLX_ = 0.0f;
	float prevLY_ = 0.0f;
	bool firstFrame_ = true;

	// UI描画
	DrawComponent2D frame_;
	FontAtlas font_;
	TextRenderer text_;
	bool fontReady_ = false;

private:
	void ApplyStepsToServices();
	void ChangeFocus(int dir);      // -1 / +1
	void AdjustCurrent(int dir);    // -1 / +1
	void Leave(bool apply);

	// 入力集約
	void UpdateInput();

	bool IsFocused(Item item) const { return focus_ == item; }
};