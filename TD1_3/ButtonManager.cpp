#include "ButtonManager.h"
#include <algorithm>
#include "InputManager.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

ButtonManager::ButtonManager() {
}

void ButtonManager::AddButton(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback) {
	buttons_.emplace_back(position, size, label, callback);
}

void ButtonManager::AddButton(const Button& button) {
	buttons_.push_back(button);
}

void ButtonManager::AddButton(const Vector2& position, const Vector2& size, int normalTexture, int selectedTexture, std::function<void()> callback) {
	buttons_.emplace_back(position, size, normalTexture, selectedTexture, callback);
	// 各ボタンの更新
	for (size_t i = 0; i < buttons_.size(); ++i) {
		buttons_[i].Update(1.0f / 60.0f, i == selectedIndex_);
	}
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

void ButtonManager::Update(float deltaTime) {
	if (buttons_.empty()) return;
	auto& input = InputManager::GetInstance();

	// 初回フレームはスキップ（誤入力防止）
	if (firstFrame_) {
		prevLY_ = input.GetPad()->GetLeftStick().y;
		firstFrame_ = false;

		// ボタンの更新のみ実行
		for (size_t i = 0; i < buttons_.size(); ++i) {
			buttons_[i].Update(deltaTime, i == selectedIndex_);
		}
		return;
	}

	int prevSelected = selectedIndex_;

	// キーボード入力処理
	HandleKeyboardInput();

	// パッド入力処理
	HandlePadInput();

	// 選択が変わった場合、SE再生
	if (prevSelected != selectedIndex_ && onSelectSound_) {
		onSelectSound_();
	}

	// 決定入力
	bool decide = (input.TriggerKey(DIK_SPACE)) ||
		(input.TriggerKey(DIK_RETURN)) ||
		input.GetPad()->Trigger(Pad::Button::A);

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
	prevLY_ = input.GetPad()->GetLeftStick().y;
}

void ButtonManager::HandleKeyboardInput() {
	auto& input = InputManager::GetInstance();
	// 上キー（W）
	if (input.TriggerKey(DIK_W)) {
		if (loopNavigation_) {

			selectedIndex_ = static_cast<int>((selectedIndex_ + static_cast<int>(buttons_.size()) - 1) % static_cast<int>(buttons_.size()));
		}
		else {
			selectedIndex_ = std::max(0, selectedIndex_ - 1);
		}
	}

	// 下キー（S）
	if (input.TriggerKey(DIK_S)) {
		if (loopNavigation_) {
			selectedIndex_ = (selectedIndex_ + 1) % buttons_.size();
		}
		else {
			selectedIndex_ = std::min(static_cast<int>(buttons_.size()) - 1, selectedIndex_ + 1);
		}
	}
}

void ButtonManager::HandlePadInput() {
	Pad* padPtr = InputManager::GetInstance().GetPad();
	if (padPtr) {
		float ly = padPtr->LeftY();
		const float threshold = 0.5f;

		// 上方向（スティックを上に倒す = Y軸+方向）
		bool padUp = (prevLY_ <= threshold && ly > threshold) ||
			padPtr->Trigger(Pad::Button::DPadUp);

		// 下方向（スティックを下に倒す = Y軸-方向）
		bool padDown = (prevLY_ >= -threshold && ly < -threshold) ||
			padPtr->Trigger(Pad::Button::DPadDown);

		if (padUp) {
			if (loopNavigation_) {
				selectedIndex_ = static_cast<int>((selectedIndex_ + static_cast<int>(buttons_.size()) - 1) % static_cast<int>(buttons_.size()));
			}
			else {
				selectedIndex_ = std::max(0, selectedIndex_ - 1);
			}
		}

		if (padDown) {
			if (loopNavigation_) {
				selectedIndex_ = (selectedIndex_ + 1) % buttons_.size();
			}
			else {
				selectedIndex_ = std::min(static_cast<int>(buttons_.size()) - 1, selectedIndex_ + 1);
			}
		}

		prevLY_ = ly;
	}
}

void ButtonManager::Draw() {
	for (auto& button : buttons_) {
		button.Draw();
	}
}

