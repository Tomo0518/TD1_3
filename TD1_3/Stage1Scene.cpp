#include "Stage1Scene.h"
#include "SceneManager.h"

Stage1Scene::Stage1Scene(SceneManager& manager) : 
	BaseStageScene(manager, 1) {
	// ステージ1固有の初期化
}

void Stage1Scene::InitializeStage() {
	// ========================================
	// ステージ1固有の初期化処理
	// ========================================

}

void Stage1Scene::UpdateStage(float dt, const char* keys, const char* pre) {
	// ステージ1固有の更新処理

	(void)dt;
	(void)keys;
	(void)pre;

	// 現在は特に固有の処理なし
}

void Stage1Scene::DrawStage() {
	// ステージ1固有の描画処理

	// 現在は特に固有の処理なし
}