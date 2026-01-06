#pragma once
#include "GameSceneBase.h"
#include "Camera2D.h"
#include "DrawComponent2D.h"
#include "Vector2.h"
#include <vector>
#include <memory>
#include <deque>
#include "SceneManager.h"

class ParticleManager;

/// <summary>
/// 物理バネによる追従・バリア演出シーン
/// </summary>
class AtomicBarrierScene : public GameSceneBase {
public:
	AtomicBarrierScene();
	AtomicBarrierScene(SceneManager& manager);
	~AtomicBarrierScene() override = default;

	void Initialize();
	void Update(float deltaTime, const char* keys, const char* preKeys) override;
	void Draw() override;

private:
	SceneManager* manager_ = nullptr;

	std::unique_ptr<ParticleManager> particleManager_ = nullptr;

	// ==========================================
	// 物理演算用ヘルパー：バネ挙動ベクトル
	// ==========================================
	struct SpringVector2 {
		Vector2 position;     // 現在位置
		Vector2 velocity;     // 現在の速度
		Vector2 target;       // 目標位置（ここに戻ろうとする）

		// 物理定数
		float stiffness;      // バネ定数（硬さ）：高いほど素早く戻る
		float damping;        // 減衰率（抵抗）：高いほど振動が早く収まる
		float mass;           // 質量：重いほど動き出しが遅い

		SpringVector2()
			: position({ 0,0 }), velocity({ 0,0 }), target({ 0,0 })
			, stiffness(150.0f), damping(10.0f), mass(1.0f) {
		}

		// 物理更新（オイラー法）
		void Update(float dt) {
			// フックの法則: F = -k * x
			Vector2 displacement = { position.x - target.x, position.y - target.y };
			Vector2 force = {
				-stiffness * displacement.x,
				-stiffness * displacement.y
			};

			// ダンピング（抵抗）: F_d = -c * v
			Vector2 dampingForce = {
				-damping * velocity.x,
				-damping * velocity.y
			};

			// 加速度: a = F / m
			Vector2 acceleration = {
				(force.x + dampingForce.x) / mass,
				(force.y + dampingForce.y) / mass
			};

			// 速度・位置更新
			velocity.x += acceleration.x * dt;
			velocity.y += acceleration.y * dt;
			position.x += velocity.x * dt;
			position.y += velocity.y * dt;
		}

		// 衝撃を与える（バースト用）
		void ApplyImpulse(const Vector2& impulse) {
			velocity.x += impulse.x / mass;
			velocity.y += impulse.y / mass;
		}
	};

	// ==========================================
	// パーティクル構造体（電子）
	// ==========================================
	struct Electron {
		// 描画・物理
		SpringVector2 physics;             // 物理実体（ここが描画される）
		std::unique_ptr<DrawComponent2D> drawComp;

		// 軌道パラメータ
		float orbitAngleOffset;            // 楕円の傾き（0, 60, 120度）
		float currentPhase;                // 現在の周回位置（ラジアン）
		float orbitSpeed;                  // 周回速度
		Vector2 orbitRadii;                // 楕円の半径（長径・短径）

		// 軌跡（トレイル）
		std::deque<Vector2> trailHistory;
		unsigned int color;                // 固有色
	};

	// ==========================================
	// メンバ変数
	// ==========================================
	Camera2D camera_;

	// プレイヤー
	Vector2 playerPos_ = { 640.0f, 360.0f };
	std::unique_ptr<DrawComponent2D> playerDrawer_;
	float playerSpeed_ = 400.0f;

	// 核（Nucleus）：プレイヤーに遅れてついてくる基準点
	SpringVector2 nucleus_;

	// 電子（パーティクル）たち
	std::vector<Electron> electrons_;

	// リソース
	int particleTex_ = -1;
	int playerTex_ = -1;

	// 演出用
	float screenShakeTimer_ = 0.0f;
	float flashIntensity_ = 0.0f;

	float ghostTimer_ = 0.0f;

	// 内部メソッド
	void SpawnElectrons();

	// ラベル
	std::unique_ptr<DrawComponent2D> sceneLabelDraw_ = nullptr;
	std::unique_ptr<DrawComponent2D> controlLabelDraw_ = nullptr;
};