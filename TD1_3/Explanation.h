#pragma once
#include "GameSceneBase.h"
#include "GameShared.h"
#include "SceneType.h"

class SceneManager;

class ExplanationScene : public GameSceneBase {
public:
	ExplanationScene(SceneManager& mgr, GameShared& shared, SceneType returnTo);
	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

	int grHandleBackground = Novice::LoadTexture("./Resources/images/explanation/background.png");
	int grHandleFrame = Novice::LoadTexture("./Resources/images/explanation/frame.png");

private:
	SceneManager& manager_;
	GameShared& shared_;
	SceneType returnTo_;
	float fade_ = 0.0f;
};