//#pragma once
//#include "IGameScene.h"
//#include "GameShared.h"
//#include "SceneType.h"
//#include "Vector2.h"
//#include "Player.h"
//#include "Camera2D.h"
//#include <memory>
//#include "Background.h"
//
//class SceneManager;
//class DebugWindow;
//
//class GamePlayScene : public IGameScene {
//public:
//	GamePlayScene(SceneManager& mgr);
//	~GamePlayScene();
//
//	void Update(float dt, const char* keys, const char* pre) override;
//	void Draw() override;
//
//private:
//	//void Initialize();
//	void InitializeCamera();
//	void InitializePlayer();
//	void InitializeBackground();
//
//	SceneManager& manager_;
//	float fade_ = 0.0f;
//
//	// ========== ゲームオブジェクト ==========
//	std::unique_ptr<Camera2D> camera_;
//	std::unique_ptr<Player> player_;
//	std::vector<std::unique_ptr<Background>> background_;
//
//	// ========== デバッグ ==========
//	std::unique_ptr<DebugWindow> debugWindow_;
//
//	// ========== テクスチャ ==========
//	int grHandleBackground_ = -1;
//	int grHandleFrame_ = -1;
//};