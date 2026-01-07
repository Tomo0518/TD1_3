#pragma once
#include "IGameScene.h"
#include "GameShared.h"
#include "Vector2.h"
#include "Camera2D.h"
#include "Background.h"
#include "GameObjectManager.h"

#include <memory>
#include <vector>

class SceneManager;
class DebugWindow;

class GamePlayScene : public IGameScene {
public:
	GamePlayScene(SceneManager& mgr);
	~GamePlayScene();

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

private:
	void Initialize();
	void InitializeCamera();
	void InitializeObjects();
	void InitializeBackground();

	SceneManager& manager_;
	//GameShared* shared_ = nullptr;

	float fade_ = 0.0f;

	// ========== ゲームオブジェクト ==========
	std::unique_ptr<Camera2D> camera_;
	bool isDebugCameraMove_ = false;
	GameObjectManager objectManager_;
	// ※ player_ は objectManager_ 管理へ寄せる（friendのGameManagerっぽくする）
	class Player* player_ = nullptr;

	std::vector<std::unique_ptr<Background>> background_;

	ParticleManager* particleManager_ = nullptr;

	// ========== デバッグ ==========
	std::unique_ptr<DebugWindow> debugWindow_;

	// ========== テクスチャ ==========
	int grHandleBackground_ = -1;
	int grHandleFrame_ = -1;
};