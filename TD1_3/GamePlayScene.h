#pragma once
#include "IGameScene.h"
#include "Camera2D.h"
#include "GameObjectManager.h"
#include "MapChipEditor.h"
#include "MapData.h"
#include "MapChip.h"
#include "MapManager.h"
#include "BackgroundManager.h"
#include "TipsUIDrawer.h"
#include <memory>
#include <vector>
#include "WorldOrigin.h"

class SceneManager;
class Player;
class Usagi;
class ParticleManager;
class DebugWindow;
class WorldOrigin;

class GamePlayScene : public IScene {
public:
    GamePlayScene(SceneManager& mgr);
    ~GamePlayScene() override;

    void Update(float dt, const char* keys, const char* pre) override;
    void Draw() override;

private:
    SceneManager& manager_;

    // --- ゲームオブジェクト ---
    GameObjectManager objectManager_;
    Usagi* player_ = nullptr;
    WorldOrigin* worldOrigin_ = nullptr; // ワールド原点

    // --- カメラ ---
    Camera2D* camera_;

    // ================================
    //  マップシステム
	// ================================
#ifdef _DEBUG
    MapChipEditor mapEditor_;
#endif
    //MapData mapData_;
    MapChip mapChip_;// 静的マップチップ描画
	MapManager mapManager_;// 動的タイル管理

    // --- 背景 ---
  //  std::vector<std::unique_ptr<Background>> background_;
	std::unique_ptr<BackgroundManager> backgroundManager_;

    // --- パーティクル ---
    ParticleManager* particleManager_ = nullptr;


    // --- Tips System ---
    std::unique_ptr<TipsUIDrawer> tipsUIDrawer_;  // 追加

    // --- デバッグ ---
    std::unique_ptr<DebugWindow> debugWindow_;
    bool isDebugCameraMove_ = false;

    // --- フェード ---
    float fade_ = 0.0f;

    // 初期化系
    void Initialize();
    void InitializeCamera();
    void InitializeObjects();

    void InitializeTipsSystem();
    void InitializeBackground();
    void SpawnObjectFromData(const ObjectSpawnInfo& spawn);

    // ワールド原点取得
    Vector2 GetWorldOriginOffset() const {
        return worldOrigin_ ? worldOrigin_->GetPosition() : Vector2{ 0.0f, 0.0f };
    }

    // collsion check
	void CheckCollisions();
};