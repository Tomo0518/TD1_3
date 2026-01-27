#include "SceneManager.h"
#include "TitleScene.h"
#include "StageSelectScene.h"
#include "SettingScene.h"
#include "PauseScene.h"
#include "ResultScene.h"
#include "Stage1Scene.h"
#include "GamePlayScene.h"


#include "SceneUtilityIncludes.h"

#include "MapData.h"

#include <Novice.h>

SceneManager::SceneManager() {
	//shared_.LoadCommonTextures();
	MapData::GetInstance().Load("./Resources/data/stage1.json");
	ChangeScene(SceneType::GamePlay);
}

void SceneManager::Update(float dt, const char* keys, const char* pre) {
	// Rキーで現在シーンを再生成（初期化）

#ifdef _DEBUG
	/*if (keys[DIK_R] && !pre[DIK_R]) {
		RequestTransition(currentSceneType_);
	}*/
#endif

	InputManager::GetInstance().Update();

	// オーバーレイがある場合はそちらを優先
	if (!overlayScenes_.empty()) {
		overlayScenes_.back()->Update(dt, keys, pre);

		// Update完了後に遅延クリア処理を実行
		if (pendingOverlayClear_) {
			overlayScenes_.clear();
			pendingOverlayClear_ = false;
		}

		// オーバーレイ表示中は実際の切り替えは行わない
		return;
	}

	// 遷移処理（R押下時の再生成もここで実行される）
	ProcessSceneTransition();

	// 現在のシーンを更新
	if (currentScene_) {
		currentScene_->Update(dt, keys, pre);
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}

	for (auto& overlay : overlayScenes_) {
		overlay->Draw();
	}
}

void SceneManager::RequestTransition(SceneType targetScene) {
	pendingTransition_ = SceneTransition{ targetScene };
}

void SceneManager::RequestRetry() {
	// オーバーレイクリアを遅延実行に変更
	pendingOverlayClear_ = true;
	RequestTransition(currentSceneType_);
}

void SceneManager::RequestPauseToTitle() {
	// オーバーレイクリアを遅延実行に変更
	pendingOverlayClear_ = true;
	RequestTransition(SceneType::Title);
}



//void SceneManager::RequestStage(int stageIndex) {
//	pendingStageIndex_ = stageIndex;
//	pendingTransition_ = SceneTransition{ StageIndexToSceneType(stageIndex) };
//}
//
//void SceneManager::RequestStageRestart() {
//	if (currentStageIndex_ >= 1 && currentStageIndex_ <= 12) {
//		RequestStage(currentStageIndex_);
//	}
//}


void SceneManager::RequestOpenPause() {
	if (currentScene_) {
		auto pauseScene = std::make_unique<PauseScene>(*this, *currentScene_);
		PushOverlay(std::move(pauseScene));
	}
}

void SceneManager::RequestClosePause() {
	PopOverlay();
}

void SceneManager::RequestOpenSettings() {
	auto settingScene = std::make_unique<SettingScene>(*this);
	PushOverlay(std::move(settingScene));
}

void SceneManager::RequestCloseSettings() {
	PopOverlay();
}

void SceneManager::PushOverlay(std::unique_ptr<IScene> overlay) {
	overlayScenes_.push_back(std::move(overlay));
}

void SceneManager::PopOverlay() {
	if (!overlayScenes_.empty()) {
		overlayScenes_.pop_back();
	}
}

void SceneManager::ProcessSceneTransition() {
	if (!pendingTransition_) {
		return;
	}

	ChangeScene(pendingTransition_->targetScene);
	pendingTransition_.reset();
}

void SceneManager::ChangeScene(SceneType type) {
	currentSceneType_ = type;

	switch (type) {
	case SceneType::Title:
		currentScene_ = std::make_unique<TitleScene>(*this);
		currentStageIndex_ = -1;
		break;

	case SceneType::StageSelect:
		currentScene_ = std::make_unique<StageSelectScene>(*this);
		currentStageIndex_ = -1;
		break;

	case SceneType::GamePlay:
		currentScene_ = std::make_unique<GamePlayScene>(*this);
		break;

	case SceneType::Result:
		currentScene_ = std::make_unique<ResultScene>(*this);//!
		break;

	case SceneType::Setting:
		currentScene_ = std::make_unique<SettingScene>(*this);
		break;

	//case SceneType::GamePlay:
	//	currentScene_ = std::make_unique<GamePlayScene>(*this);//!
	//	break;

	default:
		// 未実装のシーンの場合はタイトルに戻る
		if (type != SceneType::Title) {
			currentScene_ = std::make_unique<TitleScene>(*this);
			currentSceneType_ = SceneType::Title;
			currentStageIndex_ = -1;
		}
		break;
	}
}

SceneType SceneManager::StageIndexToSceneType(int stageIndex) const {
	switch (stageIndex) {
	case 1: return SceneType::GamePlay;
	default: return SceneType::Title;
	}
}