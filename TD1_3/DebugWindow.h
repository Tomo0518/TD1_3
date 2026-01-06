#pragma once

// 前方宣言
class Camera2D;
class Player;
class ParticleManager;

/// <summary>
/// 統合デバッグウィンドウ
/// カメラ、プレイヤー、パーティクルのデバッグ情報を一元管理
/// </summary>
class DebugWindow {
public:
	DebugWindow();
	~DebugWindow() = default;

	// ========================================
	// 統合デバッグGUI描画
	// ========================================
	void DrawDebugGui();

	// ========================================
	// カメラデバッグGUI
	// ========================================
	void DrawCameraDebugWindow(Camera2D* camera);

	// ========================================
	// プレイヤーデバッグGUI
	// ========================================
	void DrawPlayerDebugWindow(Player* player);

	// ========================================
	// パーティクルデバッグGUI
	// ========================================
	void DrawParticleDebugWindow(ParticleManager* particleManager, Player* player = nullptr);

private:
	// カメラデバッグモードの状態
	bool cameraDebugMode_ = false;
	bool showCameraWindow_ = true;
	bool showCameraInfo_ = true;
	bool showCameraEffects_ = true;
	bool showCameraControls_ = true;

	// プレイヤーデバッグの状態
	bool showPlayerWindow_ = true;

	// ★追加：パーティクルデバッグの状態
	bool showParticleWindow_ = true;
	bool showEnvironmentParticles_ = true;
	bool showActiveParticles_ = true;
	bool showParticleParams_ = false;


};