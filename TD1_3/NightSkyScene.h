#pragma once
#include "GameSceneBase.h"
#include "Camera2D.h"
#include "DrawComponent2D.h"
#include <vector>
#include <memory>
#include <list>
#include "SceneManager.h"

/// <summary>
/// 夜空を望遠鏡で覗く演出シーン（流れ星・キラキラ）
/// </summary>
class NightSkyScene : public GameSceneBase {
public:
	NightSkyScene();
	NightSkyScene(SceneManager& manager);
	~NightSkyScene() override = default;

	void Initialize();
	void Update(float deltaTime, const char* keys, const char* preKeys) override;
	void Draw() override;

private:

	SceneManager* manager_ = nullptr;

	// 1. 背景の星（前回と同じ）
	struct Star {
		std::unique_ptr<DrawComponent2D> drawComp;
		Vector2 originalPosition;
		float originalScale;
	};

	// 2. 流れ星本体
	struct ShootingStar {
		std::unique_ptr<DrawComponent2D> drawComp;
		Vector2 position;
		Vector2 velocity;
		bool isActive = false;
		float spawnTimer = 0.0f; // 次にパーティクルを出すまでの時間
		float radius = 18.0f;    // 当たり判定半径（スクリーン座標ベース）
	};

	// 3. 軌跡のキラキラ（大量に出るので軽量化）
	struct TrailParticle {
		Vector2 position;
		Vector2 velocity;
		float scale;
		float life;      // 残り寿命 (1.0 -> 0.0)
		float decayRate; // 減衰速度
		unsigned int color;
	};

	// --- メンバ変数 ---
	Camera2D camera_;

	// リソース
	int starTextureHandle_ = -1;
	int controlUiTextureHandle_ = -1;
	int lensFrameTextureHandle_ = -1;
	int uiWidth_;
	int uiHeight_;
	int particleTextureHandle_ = -1; // キラキラ用

	// オブジェクト管理
	std::vector<Star> stars_;
	std::list<ShootingStar> shootingStars_;
	std::list<TrailParticle> trails_;

	// パーティクル描画用
	std::unique_ptr<DrawComponent2D> particleDrawer_;

	// 流れ星の発生管理
	float shootingStarTimer_ = 0.0f;

	// 望遠鏡（レンズ）の設定
	Vector2 lensPosition_ = { 640.0f, 360.0f };
	float lensRadius_ = 150.0f;
	float lensMagnification_ = 2.0f;
	unsigned int bgColor_ = 0x050510FF;

	// --- 演出王 追加状態 ---
	// ヒットストップ（秒）
	float hitStopTimer_ = 0.0f;

	// フラッシュ（アルファ 0.0~1.0）
	float flashAlpha_ = 0.0f;
	// 減衰速度（秒で 0 に向かう）
	float flashDecaySpeed_ = 5.0f; // 0.2sでほぼゼロ目安（1.0 * 5.0 = 0.2 秒）

	// レンズ・リコイル（倍率）
	float recoilScale_ = 1.0f;        // 現在の倍率
	float recoilRecoverSpeed_ = 10.0f; // 1.0 へ戻る速さ

	// --- 内部ヘルパー関数 ---
	void SpawnStars(int count);
	void SpawnShootingStar();
	void AddTrail(const Vector2& pos);

	// 爆散パーティクル生成
	void SpawnExplosion(const Vector2& pos, int count);

	// レンズ描画の共通処理（テンプレート的に使えるように関数化）
	void DrawWorldElements(bool isLensEffect);

	// ラベル
	std::unique_ptr<DrawComponent2D> sceneLabelDraw_ = nullptr;
	std::unique_ptr<DrawComponent2D> controlLabelDraw_ = nullptr;
};