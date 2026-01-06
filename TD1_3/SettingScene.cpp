#include "SettingScene.h"
#include "SceneManager.h"
#include <algorithm>
#include <cstdio>

SettingScene::SettingScene(SceneManager& mgr, GameShared& shared)
	: manager_(mgr), shared_(shared) {
	bgmVolume_ = shared_.userBgmVolume;
	seVolume_ = shared_.userSeVolume;
	vibEnable_ = shared_.vibrationEnabled;
	vibStrength_ = shared_.vibrationStrength;

	bgmStep_ = std::clamp(int(std::round(bgmVolume_ * 10.0f)), 0, 10);
	seStep_ = std::clamp(int(std::round(seVolume_ * 10.0f)), 0, 10);
	vibStrengthStep_ = std::clamp(int(std::round(vibStrength_ * 10.0f)), 1, 10);
	ApplyStepsToFloat();

	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}
}

void SettingScene::ApplyStepsToFloat() {
	bgmVolume_ = bgmStep_ / 10.0f;
	seVolume_ = seStep_ / 10.0f;
	vibStrength_ = vibStrengthStep_ / 10.0f;
}

void SettingScene::ChangeFocus(int dir) {
	static constexpr Item order[] = {
		Item::BGM,
		Item::SE,
		Item::VIB_ENABLE,
		Item::VIB_STRENGTH,
		Item::BACK
	};
	int count = (int)(sizeof(order) / sizeof(order[0]));
	int cur = 0;
	for (int i = 0; i < count; ++i)
		if (order[i] == focus_) { cur = i; break; }
	cur = (cur + dir + count) % count;
	focus_ = order[cur];

	// 選択音を再生
	shared_.PlaySelectSe();
}

void SettingScene::AdjustCurrent(int dir) {
	switch (focus_) {
	case Item::BGM: {
		int before = bgmStep_;
		bgmStep_ = std::clamp(bgmStep_ + dir, 0, 10);
		if (bgmStep_ != before) {
			ApplyStepsToFloat();
			shared_.PlaySelectSe();
		}
		break;
	}
	case Item::SE: {
		int before = seStep_;
		seStep_ = std::clamp(seStep_ + dir, 0, 10);
		if (seStep_ != before) {
			ApplyStepsToFloat();
			shared_.PlaySelectSe();
		}
		break;
	}
	case Item::VIB_ENABLE:
		if (dir != 0) {
			vibEnable_ = !vibEnable_;
			shared_.PlaySelectSe();
		}
		break;
	case Item::VIB_STRENGTH:
		if (!vibEnable_) return;
		{
			int before = vibStrengthStep_;
			vibStrengthStep_ = std::clamp(vibStrengthStep_ + dir, 1, 10);
			if (vibStrengthStep_ != before) {
				ApplyStepsToFloat();
				shared_.PlaySelectSe();
			}
			break;
		}
	default: break;
	}
}

void SettingScene::Leave(bool apply) {
	if (apply) {
		// 設定を適用
		shared_.userBgmVolume = bgmVolume_;
		shared_.userSeVolume = seVolume_;
		shared_.vibrationEnabled = vibEnable_;
		shared_.vibrationStrength = vibStrength_;

		// 音声設定を反映
		shared_.ApplyAudioSettings();

		// 決定音を再生（適用後なので新しい音量で再生される）
		shared_.PlayDecideSe();

#ifdef _DEBUG
		// デバッグ出力（設定保存確認用）
		char debugMsg[256];
		snprintf(debugMsg, sizeof(debugMsg),
			"[SettingScene] Settings saved: BGM=%.2f, SE=%.2f, Vibration=%s (%.2f)\n",
			bgmVolume_, seVolume_,
			vibEnable_ ? "ON" : "OFF", vibStrength_);
		Novice::ConsolePrintf(debugMsg);
#endif
	}
	else {
		// キャンセル時はキャンセル音を再生（元の音量）
		shared_.PlayBackSe();

#ifdef _DEBUG
		Novice::ConsolePrintf("[SettingScene] Settings discarded\n");
#endif
	}

	manager_.RequestCloseSettings();
}

void SettingScene::Update(float dt, const char* keys, const char* pre) {
	(void)dt;
	shared_.pad.Update();
	float lx = shared_.pad.LeftX();
	float ly = shared_.pad.LeftY();
	const float th = 0.55f;
	bool up = false, down = false, left = false, right = false;
	if (firstFrame_) {
		prevLX_ = lx; prevLY_ = ly; firstFrame_ = false;
	}
	else {
		if (prevLY_ <= th && ly > th) up = true;
		if (prevLY_ >= -th && ly < -th) down = true;
		if (prevLX_ >= -th && lx < -th) left = true;
		if (prevLX_ <= th && lx > th) right = true;
		prevLX_ = lx; prevLY_ = ly;
	}
	bool kUp = (pre[DIK_W] == 0 && keys[DIK_W]);
	bool kDown = (pre[DIK_S] == 0 && keys[DIK_S]);
	bool kLeft = (pre[DIK_A] == 0 && keys[DIK_A]);
	bool kRight = (pre[DIK_D] == 0 && keys[DIK_D]);

	if (kUp || up) ChangeFocus(-1);
	if (kDown || down) ChangeFocus(+1);
	if (kLeft || left) AdjustCurrent(-1);
	if (kRight || right) AdjustCurrent(+1);

	bool decide = (!pre[DIK_SPACE] && keys[DIK_SPACE]) || (!pre[DIK_RETURN] && keys[DIK_RETURN]) || shared_.pad.Trigger(Pad::Button::A);
	bool cancel = (pre[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE]) || shared_.pad.Trigger(Pad::Button::B);
	if (decide && focus_ == Item::BACK) { Leave(true); return; }
	if (cancel) { Leave(true); return; }
}

void SettingScene::Draw() {
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x00000088, kFillModeSolid);
	Novice::DrawSprite(50, 50, grHandleFrame, 1.0f, 1.0f, 0.0f, 0xFFFFFFAA);

	auto drawBar = [&](int y, const char* lbl, int step, bool foc) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "%s  \xE2\x97\x80%2d\xE2\x96\xB6  (%.2f)",
			lbl, step, step / 10.0f);
		uint32_t col = foc ? 0xFFFFAAFF : 0xDDDDDDFF;
		if (fontReady_) text_.DrawTextLabel(440, y, buf, col, foc ? 1.1f : 0.9f);
		};

	auto drawToggle = [&](int y, const char* lbl, bool on, bool foc) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "%s  \xE2\x97\x80 %s \xE2\x96\xB6",
			lbl, on ? "ON " : "OFF");
		uint32_t col = foc ? 0xFFFFAAFF : 0xDDDDDDFF;
		if (!on && (lbl[0] == 'V')) {
			col = foc ? 0x8888AAFF : 0x555555FF;
		}
		if (fontReady_) text_.DrawTextLabel(440, y, buf, col, foc ? 1.1f : 0.9f);
		};

	if (fontReady_) text_.DrawTextLabel(500, 90, "SETTINGS", 0xFFFFFFFF, 1.6f);

	drawBar(200, "BGM VOLUME", bgmStep_, focus_ == Item::BGM);
	drawBar(280, "SE  VOLUME", seStep_, focus_ == Item::SE);
	drawToggle(360, "VIBRATION", vibEnable_, focus_ == Item::VIB_ENABLE);

	{
		drawBar(440, "VIB STRENGTH", vibStrengthStep_, focus_ == Item::VIB_STRENGTH);
		if (!vibEnable_) {
			char buf[128];
			std::snprintf(buf, sizeof(buf), "VIB STRENGTH  \xE2\x97\x80%2d\xE2\x96\xB6  (%.2f)",
				vibStrengthStep_, vibStrengthStep_ / 10.0f);
			if (fontReady_) text_.DrawTextLabel(440, 440, buf,
				focus_ == Item::VIB_STRENGTH ? 0x666666FF : 0x444444FF,
				focus_ == Item::VIB_STRENGTH ? 1.1f : 0.9f);
		}
	}

	const char* backLbl = "[ BACK ]";
	uint32_t backCol = focus_ == Item::BACK ? 0xFFFFAAFF : 0xDDDDDDFF;
	if (fontReady_) text_.DrawTextLabel(560, 540, backLbl, backCol, focus_ == Item::BACK ? 1.2f : 1.0f);
}