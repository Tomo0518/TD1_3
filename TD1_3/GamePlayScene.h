#pragma once
#include "IGameScene.h"
#include "Camera2D.h"
#include "GameObjectManager.h"
#include "MapChipEditor.h"
#include "MapData.h"
#include "MapChip.h"
#include "Background.h"
#include <memory>
#include <vector>
#include "WorldOrigin.h"

class SceneManager;
class Player;
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
    Player* player_ = nullptr;
    WorldOrigin* worldOrigin_ = nullptr; // ワールド原点

    // --- カメラ ---
    std::unique_ptr<Camera2D> camera_;

    // --- マップシステム ---
    MapChipEditor mapEditor_;
    MapData mapData_;
    MapChip mapChip_;

    // --- 背景 ---
    std::vector<std::unique_ptr<Background>> background_;

    // --- パーティクル ---
    ParticleManager* particleManager_ = nullptr;

    // --- デバッグ ---
    std::unique_ptr<DebugWindow> debugWindow_;
    bool isDebugCameraMove_ = false;

    // --- フェード ---
    float fade_ = 0.0f;

    // 初期化系
    void Initialize();
    void InitializeCamera();
    void InitializeObjects();
    void InitializeBackground();
    void SpawnObjectFromData(const ObjectSpawnInfo& spawn);

    // ワールド原点取得
    Vector2 GetWorldOriginOffset() const {
        return worldOrigin_ ? worldOrigin_->GetPosition() : Vector2{ 0.0f, 0.0f };
    }
};