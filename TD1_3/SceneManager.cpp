#include "SceneManager.h"
#include "TitleScene.h"
#include "StageSelectScene.h"
#include "SettingScene.h"
#include "PauseScene.h"
#include "ResultScene.h"
#include "NightSkyScene.h"
#include "AtomicBarrirScene.h"
#include "CyberOverdrive.h"

#include "GamePlayScene.h"
#include "Stage1Scene.h"


#include <Novice.h>

SceneManager::SceneManager() {
	shared_.LoadCommonTextures();
	ChangeScene(SceneType::Title);
}

void SceneManager::Update(float dt, const char* keys, const char* pre) {
	// Rキーで現在シーンを再生成（初期化）
	if (keys[DIK_R] && !pre[DIK_R]) {
		RequestTransition(currentSceneType_);
	}

	// オーバーレイがある場合はそちらを優先
	if (!overlayScenes_.empty()) {
		overlayScenes_.back()->Update(dt, keys, pre);
		// オーバーレイ表示中は実際の切り替えは行わない（予約のみ）
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

void SceneManager::RequestStage(int stageIndex) {
	pendingStageIndex_ = stageIndex;
	pendingTransition_ = SceneTransition{ StageIndexToSceneType(stageIndex) };
}

void SceneManager::RequestStageRestart() {
	if (currentStageIndex_ >= 1 && currentStageIndex_ <= 12) {
		RequestStage(currentStageIndex_);
	}
}


void SceneManager::RequestPause() {
	if (currentScene_) {
		auto pauseScene = std::make_unique<PauseScene>(*this, *currentScene_, shared_);
		PushOverlay(std::move(pauseScene));
	}
}

void SceneManager::RequestPauseResume() {
	PopOverlay();
}

void SceneManager::RequestOpenSettings() {
	auto settingScene = std::make_unique<SettingScene>(*this, shared_);
	PushOverlay(std::move(settingScene));
}

void SceneManager::RequestCloseSettings() {
	PopOverlay();
}

void SceneManager::RequestResult(int stageIndex, int score) {
	resultScore_ = score;
	pendingStageIndex_ = stageIndex;
	RequestTransition(SceneType::Result);
}

void SceneManager::PushOverlay(std::unique_ptr<GameSceneBase> overlay) {
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
		currentScene_ = std::make_unique<TitleScene>(*this, shared_);
		currentStageIndex_ = -1;
		break;

	case SceneType::StageSelect:
		currentScene_ = std::make_unique<StageSelectScene>(*this, shared_);
		currentStageIndex_ = -1;
		break;

	case SceneType::Stage1:
		currentScene_ = std::make_unique<Stage1Scene>(*this, shared_);
		currentStageIndex_ = 1;
		break;

	case SceneType::Result:
		currentScene_ = std::make_unique<ResultScene>(*this, shared_);
		break;

	case SceneType::Setting:
		currentScene_ = std::make_unique<SettingScene>(*this, shared_);
		break;

	case SceneType::GamePlay:
		currentScene_ = std::make_unique<GamePlayScene>(*this, shared_);
		break;

	case SceneType::NightSky:
		currentScene_ = std::make_unique<NightSkyScene>(*this);
		break;
	case SceneType::AtomicBarrier:
		currentScene_ = std::make_unique<AtomicBarrierScene>(*this);
		break;

	case SceneType::CyberOverdrive:
		currentScene_ = std::make_unique<CyberOverdrive>(*this, shared_);
		break;

	default:
		// 未実装のシーンの場合はタイトルに戻る
		if (type != SceneType::Title) {
			currentScene_ = std::make_unique<TitleScene>(*this, shared_);
			currentSceneType_ = SceneType::Title;
			currentStageIndex_ = -1;
		}
		break;
	}
}

SceneType SceneManager::StageIndexToSceneType(int stageIndex) const {
	switch (stageIndex) {
	case 1: return SceneType::Stage1;
	default: return SceneType::Title;
	}
}