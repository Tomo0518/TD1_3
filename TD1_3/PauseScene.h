#pragma once
#include "Pad.h"
#include "UiDrawComponent.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include "IGameScene.h"
#include <string>
#include <Novice.h>

class SceneManager;
class GameShared;

class PauseScene : public IScene {
public:
	PauseScene(SceneManager& manager, IScene& underlying, GameShared& shared);
	void Update(float deltaTime, const char* keys, const char* pre);
	void Draw();

	//bool SaveCurrentCourse(const std::string& path) const;
	//bool LoadCourse(const std::string& path);

	int inputLockTimer_ = 0;           // 入力禁止タイマー
	const int kInputLockDuration = 30;

	enum class InputMode { Keyboard, Pad };
	InputMode inputMode_ = InputMode::Keyboard;

private:
	void UpdateButtonVisual();
	void DrawCenteredText(int centerX, int y, const std::string& text, float scale, uint32_t color);

	SceneManager& manager_;
	IScene& underlying_;
	GameShared& shared_;

	static constexpr int kOptionCount = 4;
	const char* labels_[kOptionCount] = {
		"RESUME",
		"RETRY",
		"SETTINGS",
		"STAGE SELECT",
	};

	UiDrawComponent buttons_[kOptionCount];
	int  selected_ = 0;
	float prevLY_ = 0.0f;
	bool firstFrame_ = true;

	// フォント
	FontAtlas    pauseFont_;
	TextRenderer pauseText_;
	bool fontReady_ = false;

	//操作方法UI
	int grHandleBackLabel;
	int grHandleConfirmLabel;
	int grHandlePadA; // A
	int grHandlePadB; // B
	int grHandleKeySpace; // SPACE
	int grHandleKeyEscape; // ESCAPE

	float backLabelX = 780.0f, backLabelY = 640.0f, backLabelScale = 1.0f;
	float confirmLabelX = 480.0f, confirmLabelY = 640.0f, confirmLabelScale = 1.0f;
	float padAIconX = 400.0f, padAIconY = 630.0f, padAIconScale = 1.0f;
	float padBIconX = 700.0f, padBIconY = 630.0f, padBIconScale = 1.0f;
	float keySpaceIconX = 400.0f, keySpaceIconY = 630.0f, keySpaceIconScale = 1.0f;
	float keyEscapeIconX = 700.0f, keyEscapeIconY = 630.0f, keyEscapeIconScale = 1.0f;


	void DrawUI();

};