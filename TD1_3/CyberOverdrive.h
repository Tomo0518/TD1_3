#pragma once
#include "GameSceneBase.h"
#include "GameShared.h"
#include "DrawComponent2D.h"
#include <memory>

class SceneManager;
class GameShared;
class SceneManager;

/// <summary>
/// サイバー空間 "Cyber Overdrive" シーン
/// </summary>
class CyberOverdrive : public GameSceneBase {
public:
	// コンストラクタ
	CyberOverdrive(SceneManager& manager, GameShared& shared);
	virtual ~CyberOverdrive() = default;

	// GameSceneBase の純粋仮想関数をオーバーライド
	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

	void Initialize();

private:
	// 内部描画・演出ヘルパー
	void DrawGlitchPlayer();
	void DrawBackgroundGlitchDirectional(float speed, const Vector2& dir);

private:
	// 参照
	SceneManager* manager_ = nullptr;
	GameShared* shared_ = nullptr;

	// プレイヤー
	Vector2 myPlayerPos_{ 640.0f, 360.0f };
	Vector2 myPlayerVel_{ 0.0f, 0.0f };
	float myMoveSpeed_ = 220.0f;
	std::unique_ptr<DrawComponent2D> myPlayerDraw_;
	int grHandleDisitalPlayer_ = -1;

	// カメラ
	Camera2D* camera_ = nullptr;

	// 既存演出タイマー
	float gridScroll_ = 0.0f; // 旧ロジック由来（現在は固定グリッド）
	float sparkTimer_ = 0.0f;
	float ghostTimer_ = 0.0f;

	// 入力・向き
	Vector2 facingDir_{ 1.0f, 0.0f };

	// 直近ブリンク方向（スペース離した瞬間の向き）
	Vector2 lastBlinkDir_{ 1.0f, 0.0f };

	// チャージ
	bool isCharging_ = false;
	float chargeAmount_ = 0.0f;   // 0.0 ~ chargeMax_
	float chargeMax_ = 1.0f;
	float chargeRate_ = 1.0f;     // 1秒で最大
	float chargeEmitTimer_ = 0.0f;

	// ブリンク（補間ダッシュ）
	bool blinkActive_ = false;
	float blinkTime_ = 0.0f;
	float blinkPower_;   // チャージ1.0時の移動規模
	Vector2 blinkStartPos_{};
	Vector2 blinkTargetPos_{};
	int blinkForceGlitchFrames_ = 0; // ブリンク直後、背景グリッチ強制フレーム

	// ブリンク摩擦・終了条件
	float blinkFriction_ = 2.8f;
	float endSpeedThreshold_ = 60.0f;

	// チャージ中の移動係数
	float chargingMoveFactor_ = 0.55f;

	// 背景（固定グリッドの間隔）
	int gridXSpacing_ = 128;
	int gridYSpacing_ = 80;

	// ラベル
	std::unique_ptr<DrawComponent2D> sceneLabelDraw_ = nullptr;
	std::unique_ptr<DrawComponent2D> controlLabelDraw_ = nullptr;
};