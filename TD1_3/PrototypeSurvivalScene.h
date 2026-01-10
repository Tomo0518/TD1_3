#pragma once
#include "IGameScene.h"
#include "Vector2.h"
#include <vector>
#include <memory>

class SceneManager;

// クラス内で完結させるための前方宣言等はせず、ここに構造体を定義してしまいます
// （プロトタイプとしての可搬性を優先）

/// <summary>
/// 「収縮と発散」プロトタイプシーン
/// </summary>
class PrototypeSurvivalScene : public IGameScene{
public:
    PrototypeSurvivalScene(SceneManager& mgr);
    ~PrototypeSurvivalScene() override;

    void Update(float deltaTime, const char* keys, const char* preKeys) override;
    void Draw() override;

private: // --- 内部クラス・構造体定義 ---

    // プレイヤー（コア）
    struct Proto_Player {
        Vector2 pos;
        float radius = 16.0f;
        int hp = 5;
        int maxHp = 5;
        float invincibilityTimer = 0.0f; // 無敵時間
        unsigned int color = 0xFFFFFFFF;
    };

    // デブリ（攻防一体の壁）
    enum class DebrisState {
        Idle,       // 追従（最小半径）
        Expanding,  // 発散中
        WaitMax,    // 最大展開維持
        Contracting,// 収縮（攻撃）
        Cooldown    // 赤熱硬直
    };

    struct Proto_DebrisManager {
        float currentRadius;
        float minRadius = 60.0f;
        float maxRadius = 250.0f;
        float rotationAngle = 0.0f; // 全体の回転

        DebrisState state = DebrisState::Idle;
        float cooldownTimer = 0.0f;
        bool isCritical = false; // 最大展開からの収縮ボーナス

        // パラメータ
        float expandSpeed = 300.0f;
        float contractSpeed = 1200.0f; // キビキビした戻り
        float rotationSpeed = 2.0f;    // 回転速度

        // 描画用
        int debrisCount = 64;
        float debrisSize = 12.0f;
    };

    // 敵
    enum class EnemyType { Normal, Tank };
    struct Proto_Enemy {
        Vector2 pos;
        Vector2 velocity;
        EnemyType type;
        int hp;
        float radius;
        bool isAlive = false;

        // ノックバック処理用
        Vector2 knockbackVel = { 0,0 };
        float knockbackDuration = 0.0f;
    };

private: // --- メンバ変数 ---
    SceneManager* manager_;

    // ゲームオブジェクト
    Proto_Player player_;
    Proto_DebrisManager debris_;
    std::vector<Proto_Enemy> enemies_;

    // レベルデザイン・制御
    float enemySpawnTimer_ = 0.0f;

    // 演出（Game Feel）
    float hitStopTimer_ = 0.0f;   // 0より大きいとき更新を止める
    float screenShakeTimer_ = 0.0f;
    float screenShakePower_ = 0.0f;
    Vector2 cameraOffset_ = { 0,0 }; // シェイク適用後のオフセット

private: // --- ヘルパー関数 ---

    void UpdatePlayer(float dt);
    void UpdateDebris(float dt);
    void UpdateEnemies(float dt);
    void CheckCollisions();

    // 敵のスポーン
    void SpawnEnemy();

    // 演出
    void StartShake(float duration, float power);
};