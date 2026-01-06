#include "ButtonManager.h"
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


ButtonManager::ButtonManager() {
}

void ButtonManager::AddButton(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback) {
	Button newButton(position, size, label, callback);
	buttons_.push_back(std::move(newButton)); // ムーブで追加
}

void ButtonManager::AddButton(const Button& button) {
	buttons_.push_back(button);
}

void ButtonManager::ClearButtons() {
	buttons_.clear();
	selectedIndex_ = 0;
}

void ButtonManager::SetSelectedIndex(int index) {
	if (!buttons_.empty()) {
		selectedIndex_ = std::clamp(index, 0, static_cast<int>(buttons_.size()) - 1);
	}
}

void ButtonManager::SetButtonTexture(int textureHandle) {
	for (auto& button : buttons_) {
		button.SetTexture(textureHandle);
	}
}

void ButtonManager::Update(float deltaTime, const char* keys, const char* preKeys, Pad& pad) {
	if (buttons_.empty()) return;

	// 初回フレームはスキップ（誤入力防止）
	if (firstFrame_) {
		prevLY_ = pad.LeftY();
		firstFrame_ = false;

		// ボタンの更新のみ実行
		for (size_t i = 0; i < buttons_.size(); ++i) {
			buttons_[i].Update(deltaTime, i == selectedIndex_);
		}
		return;
	}

	int prevSelected = selectedIndex_;

	// キーボード入力処理
	HandleKeyboardInput(keys, preKeys);

	// パッド入力処理
	HandlePadInput(pad);

	// 選択が変わった場合、SE再生
	if (prevSelected != selectedIndex_ && onSelectSound_) {
		onSelectSound_();
	}

	// 決定入力
	bool decide = (preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE]) ||
		(preKeys[DIK_RETURN] == 0 && keys[DIK_RETURN]) ||
		pad.Trigger(Pad::Button::A);

	if (decide) {
		if (onDecideSound_) {
			onDecideSound_();
		}
		buttons_[selectedIndex_].Execute();
	}

	// 各ボタンの更新
	for (size_t i = 0; i < buttons_.size(); ++i) {
		buttons_[i].Update(deltaTime, i == selectedIndex_);
	}

	// 前フレームのパッド入力を保存
	prevLY_ = pad.LeftY();
}

void ButtonManager::HandleKeyboardInput(const char* keys, const char* preKeys) {
	if (buttons_.empty()) return;

	// 上キー（W）
	if (!preKeys[DIK_W] && keys[DIK_W]) {
		if (loopNavigation_) {
			selectedIndex_ = (selectedIndex_ + static_cast<int>(buttons_.size()) - 1) % static_cast<int>(buttons_.size());
		}
		else {
			selectedIndex_ = std::max(0, selectedIndex_ - 1);
		}
	}

	// 下キー（S）
	if (!preKeys[DIK_S] && keys[DIK_S]) {
		if (loopNavigation_) {
			selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(buttons_.size());
		}
		else {
			selectedIndex_ = std::min(static_cast<int>(buttons_.size()) - 1, selectedIndex_ + 1);
		}
	}
}

void ButtonManager::HandlePadInput(Pad& pad) {
	if (buttons_.empty()) return;

	float ly = pad.LeftY();
	const float threshold = 0.5f;

	// 上方向（スティックを上に倒す = Y軸+方向）
	bool padUp = (prevLY_ <= threshold && ly > threshold) ||
		pad.Trigger(Pad::Button::DPadUp);

	// 下方向（スティックを下に倒す = Y軸-方向）
	bool padDown = (prevLY_ >= -threshold && ly < -threshold) ||
		pad.Trigger(Pad::Button::DPadDown);

	if (padUp) {
		if (loopNavigation_) {
			selectedIndex_ = (selectedIndex_ + static_cast<int>(buttons_.size()) - 1) % static_cast<int>(buttons_.size());
		}
		else {
			selectedIndex_ = std::max(0, selectedIndex_ - 1);
		}
	}

	if (padDown) {
		if (loopNavigation_) {
			selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(buttons_.size());
		}
		else {
			selectedIndex_ = std::min(static_cast<int>(buttons_.size()) - 1, selectedIndex_ + 1);
		}
	}
}

void ButtonManager::Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer) {
	for (auto& button : buttons_) {
		button.Draw(textureHandle, font, textRenderer);
	}
}