#include "SettingScene.h"
#include "SceneManager.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

#include "SceneUtilityIncludes.h"

using namespace GameServices;

namespace {
	constexpr float kStickThreshold = 0.55f;

	inline int ClampStep01(int step) {
		return std::clamp(step, 0, 10);
	}

	inline int ClampStep10(int step) {
		return std::clamp(step, 1, 10);
	}

	inline int VolumeToStep(float volume) {
		return std::clamp(static_cast<int>(std::round(volume * 10.0f)), 0, 10);
	}

	inline int StrengthToStep(float strength) {
		return std::clamp(static_cast<int>(std::round(strength * 10.0f)), 1, 10);
	}

	inline float StepToVolume(int step) {
		return step / 10.0f;
	}
}

SettingScene::SettingScene(SceneManager& mgr)
	: manager_(mgr) {

	// フレーム画像
	frame_ = DrawComponent2D(Tex().GetTexture(TextureId::ResultBackground));
	frame_.SetGraphHandle(Novice::LoadTexture("./Resources/images/explanation/frame.png"));
	frame_.SetAnchorPoint({ 0.0f, 0.0f });
	frame_.SetPosition({ 50.0f, 50.0f });

	// 現在値（設定画面を開いた瞬間の値を反映）
	bgmStep_ = VolumeToStep(Sound().GetBgmVolume());
	seStep_ = VolumeToStep(Sound().GetSeVolume());
	vibStrengthStep_ = StrengthToStep(Input().GetVibrationStrength());

	ApplyStepsToServices();

	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}
}

void SettingScene::ApplyStepsToServices() {
	Sound().SetBgmVolume(StepToVolume(bgmStep_));
	Sound().SetSeVolume(StepToVolume(seStep_));

	Input().SetVibrationStrength(StepToVolume(vibStrengthStep_));

	// 現在再生中BGMに反映
	Sound().ApplyAudioSettings();
}

void SettingScene::ChangeFocus(int dir) {
	static constexpr Item order[] = {
		Item::BGM,
		Item::SE,
		Item::VIB_ENABLE,
		Item::VIB_STRENGTH,
		Item::BACK
	};

	const int count = static_cast<int>(sizeof(order) / sizeof(order[0]));
	int cur = 0;
	for (int i = 0; i < count; ++i) {
		if (order[i] == focus_) {
			cur = i;
			break;
		}
	}
	cur = (cur + dir + count) % count;
	focus_ = order[cur];

	Sound().PlaySe(SeId::Select);
}

void SettingScene::AdjustCurrent(int dir) {
	switch (focus_) {
	case Item::BGM: {
		const int before = bgmStep_;
		bgmStep_ = ClampStep01(bgmStep_ + dir);
		if (bgmStep_ != before) {
			ApplyStepsToServices();
			Sound().PlaySe(SeId::Select);
		}
		break;
	}
	case Item::SE: {
		const int before = seStep_;
		seStep_ = ClampStep01(seStep_ + dir);
		if (seStep_ != before) {
			ApplyStepsToServices();
			Sound().PlaySe(SeId::Select);
		}
		break;
	}
	case Item::VIB_ENABLE: {
		if (dir != 0) {
			const bool vibEnable = !Input().IsVibrationEnabled();
			Input().SetVibrationEnabled(vibEnable);
			Sound().PlaySe(SeId::Select);
		}
		break;
	}
	case Item::VIB_STRENGTH: {
		if (!Input().IsVibrationEnabled()) {
			return;
		}
		const int before = vibStrengthStep_;
		vibStrengthStep_ = ClampStep10(vibStrengthStep_ + dir);
		if (vibStrengthStep_ != before) {
			ApplyStepsToServices();
			Sound().PlaySe(SeId::Select);
		}
		break;
	}
	default:
		break;
	}
}

void SettingScene::Leave(bool apply) {
	if (apply) {
		Sound().ApplyAudioSettings();
		Sound().PlaySe(SeId::Decide);
	}
	else {
		// ここで「元に戻す」挙動が必要なら、入室時スナップショットを保持して復元する
		Sound().PlaySe(SeId::Back);
	}

	manager_.RequestCloseSettings();
}

void SettingScene::UpdateInput() {
	Pad* pad = Input().GetPad();
	const float lx = pad ? pad->LeftX() : 0.0f;
	const float ly = pad ? pad->LeftY() : 0.0f;

	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;

	if (firstFrame_) {
		prevLX_ = lx;
		prevLY_ = ly;
		firstFrame_ = false;
	}
	else {
		if (prevLY_ <= kStickThreshold && ly > kStickThreshold) up = true;
		if (prevLY_ >= -kStickThreshold && ly < -kStickThreshold) down = true;
		if (prevLX_ >= -kStickThreshold && lx < -kStickThreshold) left = true;
		if (prevLX_ <= kStickThreshold && lx > kStickThreshold) right = true;

		prevLX_ = lx;
		prevLY_ = ly;
	}

	const bool kUp = Input().TriggerKey(DIK_W);
	const bool kDown = Input().TriggerKey(DIK_S);
	const bool kLeft = Input().TriggerKey(DIK_A);
	const bool kRight = Input().TriggerKey(DIK_D);

	if (kUp || up) ChangeFocus(-1);
	if (kDown || down) ChangeFocus(+1);
	if (kLeft || left) AdjustCurrent(-1);
	if (kRight || right) AdjustCurrent(+1);

	const bool decide =
		Input().TriggerKey(DIK_SPACE) ||
		Input().TriggerKey(DIK_RETURN) ||
		(pad && pad->Trigger(Pad::Button::A));

	const bool cancel =
		Input().TriggerKey(DIK_ESCAPE) ||
		(pad && pad->Trigger(Pad::Button::B));

	if (decide && focus_ == Item::BACK) {
		Leave(true);
	}
	else if (cancel) {
		Leave(false);
	}
}

void SettingScene::Update(float dt, const char* keys, const char* pre) {
	(void)dt;
	(void)keys;
	(void)pre;

	frame_.Update(dt);
	UpdateInput();
}

void SettingScene::Draw() {
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x00000088, kFillModeSolid);

	// フレーム描画（DrawComponent2D化）
	frame_.DrawScreen();

	auto drawBar = [&](int y, const char* lbl, int step, bool foc, bool enable) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "%s  \xE2\x97\x80%2d\xE2\x96\xB6  (%.2f)",
			lbl, step, StepToVolume(step));

		uint32_t col = foc ? 0xFFFFAAFF : 0xDDDDDDFF;
		if (!enable) {
			col = foc ? 0x666666FF : 0x444444FF;
		}

		if (fontReady_) {
			text_.DrawTextLabel(440, y, buf, col, foc ? 1.1f : 0.9f);
		}
		};

	auto drawToggle = [&](int y, const char* lbl, bool on, bool foc) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "%s  \xE2\x97\x80 %s \xE2\x96\xB6",
			lbl, on ? "ON " : "OFF");

		uint32_t col = foc ? 0xFFFFAAFF : 0xDDDDDDFF;
		if (!on) {
			col = foc ? 0x8888AAFF : 0x555555FF;
		}

		if (fontReady_) {
			text_.DrawTextLabel(440, y, buf, col, foc ? 1.1f : 0.9f);
		}
		};

	if (fontReady_) {
		text_.DrawTextLabel(500, 90, "SETTINGS", 0xFFFFFFFF, 1.6f);
	}

	const bool vibEnabled = Input().IsVibrationEnabled();

	drawBar(200, "BGM VOLUME", bgmStep_, IsFocused(Item::BGM), true);
	drawBar(280, "SE  VOLUME", seStep_, IsFocused(Item::SE), true);
	drawToggle(360, "VIBRATION", vibEnabled, IsFocused(Item::VIB_ENABLE));

	drawBar(440, "VIB STRENGTH", vibStrengthStep_, IsFocused(Item::VIB_STRENGTH), vibEnabled);

	const char* backLbl = "[ BACK ]";
	const uint32_t backCol = IsFocused(Item::BACK) ? 0xFFFFAAFF : 0xDDDDDDFF;
	if (fontReady_) {
		text_.DrawTextLabel(560, 540, backLbl, backCol, IsFocused(Item::BACK) ? 1.2f : 1.0f);
	}
}