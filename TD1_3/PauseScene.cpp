#include "PauseScene.h"
#include "SceneManager.h"
#include <Novice.h>
#include <cstring>
#include <algorithm>
#include <string>
#include "Easing.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <ctime>


namespace {
	constexpr uint32_t kPauseColFillNormal = 0x2A2A2AFF;  // 通常ボタン塗り
	constexpr uint32_t kPauseColFillSelected = 0xFFFFBBFF;  // 選択ボタン塗り（黄）
	constexpr uint32_t kPauseColFrameNormal = 0xFFFFFFFF;  // 通常枠
	constexpr uint32_t kPauseColFrameSelected = 0xFFFFBBFF;  // 選択枠
	constexpr uint32_t kPauseColText = 0xCCCCCCFF;  // 黒文字
	constexpr uint32_t kPauseColSelectedText = 0xFFFF55FF;
}

PauseScene::PauseScene(SceneManager& manager, GameSceneBase& underlying, GameShared& shared)
	: manager_(manager), underlying_(underlying), shared_(shared) {

	if (pauseFont_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		pauseText_.SetFont(&pauseFont_);
		fontReady_ = true;
	}

	grHandleBackLabel = Novice::LoadTexture("./Resources/images/pause/backLabel.png");
	grHandleConfirmLabel = Novice::LoadTexture("./Resources/images/pause/confirmLabel.png");
	grHandlePadA = Novice::LoadTexture("./Resources/images/pause/pad_A.png");
	grHandlePadB = Novice::LoadTexture("./Resources/images/pause/pad_B.png");
	grHandleKeySpace = Novice::LoadTexture("./Resources/images/pause/key_space.png");
	grHandleKeyEscape = Novice::LoadTexture("./Resources/images/pause/key_escape.png");

	//入力拒否時間をセット
	inputLockTimer_ = kInputLockDuration;

	const float centerX = 640.0f;
	const float centerY = 360.0f;
	const float w = 320.0f;
	const float h = 64.0f;
	const float gap = 90.0f;

	for (int i = 0; i < kOptionCount; ++i) {
		buttons_[i].grHandle_ = manager_.Shared().texWhite;
		buttons_[i].SetSize({ w, h });
		buttons_[i].SetAnchor({ 0.5f,0.5f });
		float y = centerY + (i - (kOptionCount - 1) / 2.0f) * gap;
		buttons_[i].SetPosition({ centerX, y });
		buttons_[i].SetColor(kPauseColFillNormal);
		buttons_[i].baseScale_ = 1.0f;
		buttons_[i].scaleRange_ = 0.0f;
	}
	UpdateButtonVisual();
}

void PauseScene::UpdateButtonVisual() {
	for (int i = 0; i < kOptionCount; ++i) {
		bool sel = (i == selected_);
		buttons_[i].SetColor(sel ? kPauseColFillSelected : kPauseColFillNormal);
		// サイズアニメは必要なら残す
		buttons_[i].baseScale_ = sel ? 1.05f : 1.0f;
		buttons_[i].scaleRange_ = sel ? 0.10f : 0.0f;
	}
}

void PauseScene::Update(float deltaTime, const char* keys, const char* pre) {
	deltaTime; // 未使用
	// 入力ロック中は処理しない
	if (inputLockTimer_ > 0) {
		inputLockTimer_--;
		// ボタンのアニメーション更新だけ実行
		for (auto& b : buttons_) {
			b.Update();
		}
		return;
	}

	if (firstFrame_) {
		prevLY_ = shared_.pad.LeftY();
		firstFrame_ = false;
		for (auto& b : buttons_) b.Update();
		return;
	}

	// ========================================
	// キーボード入力
	// ========================================
	if (!pre[DIK_W] && keys[DIK_W]) {
		selected_ = (selected_ + kOptionCount - 1) % kOptionCount;
		shared_.PlaySelectSe();
		UpdateButtonVisual();
	}

	if (!pre[DIK_S] && keys[DIK_S]) {
		selected_ = (selected_ + 1) % kOptionCount;
		shared_.PlaySelectSe();
		UpdateButtonVisual();
	}

	// ========================================
	// パッド入力
	// ========================================
	float ly = shared_.pad.LeftY();
	const float threshold = 0.5f;

	bool padUp = (prevLY_ <= threshold && ly > threshold) ||
		shared_.pad.Trigger(Pad::Button::DPadUp);

	bool padDown = (prevLY_ >= -threshold && ly < -threshold) ||
		shared_.pad.Trigger(Pad::Button::DPadDown);

	if (padUp) {
		selected_ = (selected_ + kOptionCount - 1) % kOptionCount;
		shared_.PlaySelectSe();
		UpdateButtonVisual();
	}

	if (padDown) {
		selected_ = (selected_ + 1) % kOptionCount;
		shared_.PlaySelectSe();
		UpdateButtonVisual();
	}

	prevLY_ = ly;

	// ========================================
	// 決定・キャンセル入力
	// ========================================
	bool decide = (pre[DIK_SPACE] == 0 && keys[DIK_SPACE]) ||
		shared_.pad.Trigger(Pad::Button::A);

	if (decide) {
		shared_.PlayDecideSe();
		switch (selected_) {
		case 0: // 戻る
			manager_.RequestPauseResume();
			return;
		case 1: { // リスタート
			manager_.Shared().StopAllBgm();
			int idx = underlying_.GetStageIndex();
			if (idx >= 0) {
				manager_.RequestStageRestart();
			}
			else {
				manager_.RequestTransition(SceneType::StageSelect);
			}
			return;
		}
		case 2: // 設定
			manager_.RequestOpenSettings();
			return;
		case 3: // セレクトに戻る
			shared_.PlayExclusive_(BgmKind::Title);
			manager_.RequestTransition(SceneType::StageSelect);
			return;
		}
	}

	// ポーズ解除（B/Start/ESC）
	if ((pre[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE]) ||
		shared_.pad.Trigger(Pad::Button::B) ||
		shared_.pad.Trigger(Pad::Button::Start)) {
		shared_.PlayBackSe();
		manager_.RequestPauseResume();
		return;
	}

	// ボタンのアニメーション更新
	for (auto& b : buttons_) {
		b.Update();
	}
}

void PauseScene::DrawCenteredText(int centerX, int y, const std::string& text, float scale, uint32_t color) {
	int w = pauseText_.MeasureWidth(text, scale);
	pauseText_.DrawTextLabel(centerX - w / 2, y, text, color, scale);
}

void PauseScene::Draw() {
	underlying_.Draw();

	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x000000AA, kFillModeSolid);

	// メニュー
	const float baseLabelScale = 1.5f;
	for (int i = 0; i < kOptionCount; ++i) {
		buttons_[i].Draw();

		Vector2 p = buttons_[i].GetPosition();
		Vector2 size = buttons_[i].GetSize();
		Vector2 scl = buttons_[i].GetScale();
		float w = size.x * scl.x;
		float h = size.y * scl.y;

		int btnLeft = (int)(p.x - w * 0.5f);
		int btnTop = (int)(p.y - h * 0.5f);

		uint32_t frameCol = (i == selected_) ? kPauseColFrameSelected : kPauseColFrameNormal;
		Novice::DrawBox(btnLeft, btnTop, (int)w, (int)h, 0.0f, frameCol, kFillModeWireFrame);

		// 文字色: 選択時は指定の黄色、通常は淡色
		uint32_t textCol = (i == selected_) ? kPauseColSelectedText : kPauseColText;

		float scale = (i == selected_) ? (baseLabelScale * 1.02f) : baseLabelScale;
		float baselineY = p.y - (scale * pauseFont_.GetLineHeight()) * 0.45f;

		float drawScale = scale;
		if (i == 3) {
			drawScale *= 0.82f;
			baselineY += 5.0f;
		}
		DrawCenteredText((int)p.x, (int)baselineY, labels_[i], drawScale, textCol);
	}

	DrawUI();

	//float hintScale = 0.85f;
	//struct Seg { const char* txt; uint32_t col; };
	//Seg segs[] = {
	//	{"LEFT / Stick : Select   ", 0xc5d7daFF},
	//	{"A", 0x77c45fff},
	//	{" : Confirm    ", 0xc5d7daFF},
	//	{"B", 0xff423cFF},
	//	{" : Back    ", 0xc5d7daFF}
	//};

	//int totalW = 0;
	//for (auto& s : segs) totalW += pauseText_.MeasureWidth(s.txt, hintScale);
	//int startX = (int)(640.0f - totalW * 0.5f);
	//int baseY = 650;
	//int x = startX;

	////Novice::DrawBox(static_cast<int>(640.0f - totalW / 2.0f - 20.0f), static_cast<int>(baseY - 10, (int)totalW + 40), (int)totalW + 80, 80, 0.0f, BLACK, kFillModeSolid);

	//for (auto& s : segs) {
	//	pauseText_.DrawTextLabel(x, baseY, s.txt, s.col, hintScale);
	//	x += pauseText_.MeasureWidth(s.txt, hintScale);
	//}

	// 操作ヒント
	//DrawCenteredText(
	//	640, 660,
	//	"LEFT / Stick : Select   A: Confirm    B/START: Resume",
	//	0.85f, 0xFFFFFFFF);
}

void PauseScene::DrawUI() {
	// --- 操作アイコン描画 ---

	if (shared_.GetInputMode() == GameShared::InputMode::Pad) {
		Novice::DrawSprite(
			static_cast<int>(padAIconX), static_cast<int>(padAIconY),
			grHandlePadA, padAIconScale, padAIconScale, 0.0f, 0xFFFFFFFF
		);

		Novice::DrawSprite(
			static_cast<int>(padBIconX), static_cast<int>(padBIconY),
			grHandlePadB, padBIconScale, padBIconScale, 0.0f, 0xFFFFFFFF
		);
	}
	else {
		Novice::DrawSprite(
			static_cast<int>(keySpaceIconX), static_cast<int>(keySpaceIconY),
			grHandleKeySpace, keySpaceIconScale, keySpaceIconScale, 0.0f, 0xFFFFFFFF
		);

		Novice::DrawSprite(
			static_cast<int>(keyEscapeIconX), static_cast<int>(keyEscapeIconY),
			grHandleKeyEscape, keyEscapeIconScale, keyEscapeIconScale, 0.0f, 0xFFFFFFFF
		);
	}

	Novice::DrawSprite(
		static_cast<int>(backLabelX), static_cast<int>(backLabelY),
		grHandleBackLabel, backLabelScale, backLabelScale, 0.0f, 0xFFFFFFFF
	);
	Novice::DrawSprite(
		static_cast<int>(confirmLabelX), static_cast<int>(confirmLabelY),
		grHandleConfirmLabel, confirmLabelScale, confirmLabelScale, 0.0f, 0xFFFFFFFF
	);

#ifdef _DEBUG
	ImGui::Begin("PauseScene 操作アイコン配置");
	ImGui::Text("Pad A");
	ImGui::SliderFloat("PadA X", &padAIconX, 0.0f, 1280.0f);
	ImGui::SliderFloat("PadA Y", &padAIconY, 0.0f, 720.0f);
	ImGui::SliderFloat("PadA Scale", &padAIconScale, 0.1f, 3.0f);

	ImGui::Separator();
	ImGui::Text("Pad B");
	ImGui::SliderFloat("PadB X", &padBIconX, 0.0f, 1280.0f);
	ImGui::SliderFloat("PadB Y", &padBIconY, 0.0f, 720.0f);
	ImGui::SliderFloat("PadB Scale", &padBIconScale, 0.1f, 3.0f);

	ImGui::Separator();
	ImGui::Text("Key Space");
	ImGui::SliderFloat("Space X", &keySpaceIconX, 0.0f, 1280.0f);
	ImGui::SliderFloat("Space Y", &keySpaceIconY, 0.0f, 720.0f);
	ImGui::SliderFloat("Space Scale", &keySpaceIconScale, 0.1f, 3.0f);

	ImGui::Separator();
	ImGui::Text("Key Escape");
	ImGui::SliderFloat("Escape X", &keyEscapeIconX, 0.0f, 1280.0f);
	ImGui::SliderFloat("Escape Y", &keyEscapeIconY, 0.0f, 720.0f);
	ImGui::SliderFloat("Escape Scale", &keyEscapeIconScale, 0.1f, 3.0f);

	ImGui::Separator();
	ImGui::Text("Back Label");
	ImGui::SliderFloat("BackLabel X", &backLabelX, 0.0f, 1280.0f);
	ImGui::SliderFloat("BackLabel Y", &backLabelY, 0.0f, 720.0f);
	ImGui::SliderFloat("BackLabel Scale", &backLabelScale, 0.1f, 3.0f);

	ImGui::Separator();
	ImGui::Text("Confirm Label");
	ImGui::SliderFloat("ConfirmLabel X", &confirmLabelX, 0.0f, 1280.0f);
	ImGui::SliderFloat("ConfirmLabel Y", &confirmLabelY, 0.0f, 720.0f);
	ImGui::SliderFloat("ConfirmLabel Scale", &confirmLabelScale, 0.1f, 3.0f);

	ImGui::End();
#endif
}