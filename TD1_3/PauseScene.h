#pragma once
#include "IGameScene.h"
#include "ButtonManager.h"
#include <memory>

class SceneManager;

class PauseScene : public IScene {
public:
    PauseScene(SceneManager& manager, IScene& underlying);
    ~PauseScene() override = default;

    void Update(float dt, const char* keys, const char* preKeys) override;
    void Draw() override;

private:
    SceneManager& manager_;
    IScene& underlying_;
    std::unique_ptr<ButtonManager> buttonManager_;

    void InitializeButtons();
};