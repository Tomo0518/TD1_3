#include "PauseScene.h"
#include "SceneManager.h"
#include "SettingScene.h"
#include <Novice.h>
#include "SceneUtilityIncludes.h"

PauseScene::PauseScene(SceneManager& manager, IScene& underlying)
	: manager_(manager), underlying_(underlying) {

	buttonManager_ = std::make_unique<ButtonManager>();
	InitializeButtons();
}

void PauseScene::InitializeButtons() {
	// ボタン画像読み込み
	int resume = Tex().GetTexture(TextureId::UI_Button_Resume);
	int resumeSelected = Tex().GetTexture(TextureId::UI_Button_Resume_Selected);

	int retry = Tex().GetTexture(TextureId::UI_Button_Retry);
	int retrySelected = Tex().GetTexture(TextureId::UI_Button_Retry_Selected);

	int settings = Tex().GetTexture(TextureId::UI_Button_Settings);
	int settingsSelected = Tex().GetTexture(TextureId::UI_Button_Settings_Selected);

	int title = Tex().GetTexture(TextureId::UI_Button_Title);
	int titleSelected = Tex().GetTexture(TextureId::UI_Button_Title_Selected);	

	const float centerX = 640.0f;
	const float startY = 200.0f;
	const float buttonSpacing = 100.0f;
	Vector2 buttonSize = { 300.0f, 80.0f };

	// Resume: ポーズ解除
	buttonManager_->AddButton(
		Vector2{ centerX, startY },
		buttonSize,
		resume,
		resumeSelected,
		[this]() { manager_.PopOverlay(); }
	);

	// Retry: ゲームをリスタート
	buttonManager_->AddButton(
		Vector2{ centerX, startY + buttonSpacing },
		buttonSize,
		retry,
		retrySelected,
		[this]() {
			manager_.RequestRetry();
		}
	);


	// Settings: 設定画面を開く
	buttonManager_->AddButton(
		Vector2{ centerX, startY + buttonSpacing * 2 },
		buttonSize,
		settings,
		settingsSelected,
		[this]() {
			auto settingScene = std::make_unique<SettingScene>(manager_);
			manager_.PushOverlay(std::move(settingScene));
		}
	);

	// Title: タイトルに戻る
	buttonManager_->AddButton(
		Vector2{ centerX, startY + buttonSpacing * 3 },
		buttonSize,
		title,
		titleSelected,
		[this]() {
			manager_.RequestPauseToTitle();
		}
	);

	// SE設定
	buttonManager_->SetOnSelectSound([&]() { Sound().PlaySe(SeId::Select);});
	buttonManager_->SetOnDecideSound([&]() { Sound().PlaySe(SeId::Decide); });
	buttonManager_->SetFirstFrame(true);
}

void PauseScene::Update(float dt, const char* keys, const char* pre) {

	buttonManager_->Update(dt);

	// ESC/B/Startでポーズ解除
	if ((pre[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE]) ||
		Input().GetPad()->Trigger(Pad::Button::B) ||
		Input().GetPad()->Trigger(Pad::Button::Start)) {
		Sound().PlaySe(SeId::Back);
		manager_.PopOverlay();
	}
}

void PauseScene::Draw() {
	// 下のシーンを描画
	underlying_.Draw();

	// 暗いオーバーレイ
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x000000CC, kFillModeSolid);

	// ボタン描画
	buttonManager_->Draw();
}