#include "ExplanationScene.h"
#include "SceneManager.h"
#include <Novice.h>
#include <cstdio>

ExplanationScene::ExplanationScene(SceneManager& mgr, GameShared& shared, SceneType returnTo)
	: manager_(mgr), shared_(shared), returnTo_(returnTo) {
	shared_.pad.Update();
	shared_.MarkExplanationViewed();
}

void ExplanationScene::Update(float dt, const char* keys, const char* pre) {
	shared_.pad.Update();

	if (fade_ < 1.0f) fade_ += dt * 4.0f;

	bool close =
		shared_.pad.Trigger(Pad::Button::B) ||
		shared_.pad.Trigger(Pad::Button::A) ||
		shared_.pad.Trigger(Pad::Button::Y) ||
		shared_.pad.Trigger(Pad::Button::Back) ||
		(pre[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE]) ||
		(pre[DIK_RETURN] == 0 && keys[DIK_RETURN]) ||
		(pre[DIK_SPACE] == 0 && keys[DIK_SPACE]) ||
		(pre[DIK_Y] == 0 && keys[DIK_Y]);

	if (close) {
		// 戻る音を再生
		shared_.PlayBackSe();

		// 戻り先へ
		switch (returnTo_) {
		case SceneType::StageSelect: manager_.RequestTransition(SceneType::StageSelect); break;
		case SceneType::Title: default: manager_.RequestTransition(SceneType::Title); break;
		}
	}
}

void ExplanationScene::Draw() {
	// 背景を暗く
	Novice::DrawSprite(0, 0, grHandleBackground, 1.0f, 1.0f, 0.0f, 0xFFFFFFFF);

	Novice::DrawSprite(50, 50, grHandleFrame, 1.0f, 1.0f, 0.0f, 0xFFFFFF88);
}