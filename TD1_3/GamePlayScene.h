#pragma once
#include "IGameScene.h"
#include "GameShared.h"
#include "Camera2D.h"
#include "Background.h"
#include "GameObjectManager.h"

#include "MapData.h"
#include "MapChip.h"
#include "MapChipEditor.h" // ファイル名がMapEditor.hならそちらに合わせて
#include "PhysicsManager.h"

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
    float fade_ = 0.0f;

    // ========== ゲームオブジェクト ==========
    std::unique_ptr<Camera2D> camera_;
    bool isDebugCameraMove_ = false;
    GameObjectManager objectManager_;
    class Player* player_ = nullptr;

    std::vector<std::unique_ptr<Background>> background_;
    ParticleManager* particleManager_ = nullptr;

    // ========== デバッグ ==========
    std::unique_ptr<DebugWindow> debugWindow_;

    // ========== マップシステム追加 ==========
    MapData mapData_;       // データ
    MapChip mapChip_;       // 描画
    MapChipEditor mapEditor_;   // エディタ


    int grHandleBackground_ = -1;
    int grHandleFrame_ = -1;
};