#pragma once
#include "IGameScene.h"
#include "InputManager.h"
#include "Camera2D.h"
#include "SurvivalGameManager.h" // 新マネージャー

class SceneManager;

class PrototypeSurvivalScene : public IScene {
public:
    PrototypeSurvivalScene(SceneManager& manager);
    ~PrototypeSurvivalScene() override;

    void Update(float deltaTime, const char* keys, const char* preKeys) override;
    void Draw() override;

private:
    // シーン遷移用
    SceneManager* sceneManager_;

    // 入力
    std::unique_ptr<InputManager> input_;

    // マネージャー（ここで全てのオブジェクトを管理）
    std::unique_ptr<SurvivalGameObjectManager> gameObjectManager_;

    // カメラ（演出用）
    std::unique_ptr<Camera2D> camera_;

    // レベル管理
    float enemySpawnTimer_ = 0.0f;

    // 演出用シェイクタイマー（カメラクラスにも機能あるが、シーン全体制御として持つ）
    float shakeTimer_ = 0.0f;

    // プライベートメソッド
    void SpawnEnemy();
    void UpdateCamera(float dt); // カメラのズーム制御など
};