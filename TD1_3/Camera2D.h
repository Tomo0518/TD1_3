#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"
#include "WindowSize.h"
#include <functional>
#include "Easing.h"

class DebugWindow; // 前方宣言

class Camera2D {
	friend class DebugWindow;
public:

	// コンストラクタ: 位置とサイズで初期化、Y軸反転オプション

	/// <summary>
	/// カメラのコンストラクタ
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="size">描画したい範囲(基本的にWindowSize)</param>
	/// <param name="invertY">描画のY軸を反転させるか(スクリーン:false,ワールド(上が+):true)</param>
	Camera2D(const Vector2& position = { 640, 360 },
		const Vector2& size = { 1280.0f, 720.0f },
		bool invertY = false);

	static Camera2D GetInstance(){
		static Camera2D instance;
		return instance;
	}

	// 更新（deltaTime対応でスムーズに）
	void Update(float deltaTime);

	// === 基本操作 ===
	void SetPosition(const Vector2& pos);
	Vector2 GetPosition() const;

	void SetZoom(float zoom);
	float GetZoom() const;

	// === イージング移動 ===
	void MoveTo(const Vector2& targetPos, float duration,
		std::function<float(float)> easingFunc = Easing::Linear);

	//  === デバッグ用カメラ操作 ===
	void DebugMove(bool isDebug, const char* keys, const char* pre);

	// === ズーム ===
	void ZoomTo(float targetZoom, float duration,
		std::function<float(float)> easingFunc = Easing::EaseOutQuad);

	// === シェイク ===
	void Shake(float intensity, float duration);
	void ShakeContinuous(float intensity);  // 停止まで継続
	void StopShake();

	// === ターゲット追従 ===
	void SetTarget(const Vector2* target);  // ポインタで追従対象を設定
	void SetFollowSpeed(float speed);       // 追従速度（0.1～1.0推奨）
	void SetDeadZone(float width, float height);  // デッドゾーン設定

	// === 境界制限 ===
	void SetBounds(float left, float top, float right, float bottom);
	void ClearBounds();

	// === 行列取得 ===
	Matrix3x3 GetVpVpMatrix() const;

	// === Y軸反転取得 ===
	bool IsInvertY() const { return invertY_; }

	// デバッグ用
	bool GetIsDebugCamera() { return isDebugCamera_; }

private:
	// 基本パラメータ
	Vector2 position_;
	Vector2 size_;
	float zoom_;
	float rotation_;
	bool invertY_;  // Y軸反転フラグ

	bool isDebugCamera_ = false;

	// イージング移動
	struct MoveEffect {
		bool isActive = false;
		Vector2 startPos;
		Vector2 targetPos;
		float elapsed = 0.0f;
		float duration = 0.0f;
		std::function<float(float)> easingFunc;
	};

	// ズーム効果
	struct ZoomEffect {
		bool isActive = false;
		float startZoom;
		float targetZoom;
		float elapsed = 0.0f;
		float duration = 0.0f;
		std::function<float(float)> easingFunc;
	};

	// シェイク効果
	struct ShakeEffect {
		bool isActive = false;
		float intensity = 0.0f;
		float duration = 0.0f;
		float elapsed = 0.0f;
		bool continuous = false;
		Vector2 offset;
	};

	// 追従設定
	struct FollowSettings {
		const Vector2* target = nullptr;
		float speed = 0.1f;
		float deadZoneWidth = 0.0f;
		float deadZoneHeight = 0.0f;
	};

	// 境界設定
	struct Bounds {
		bool enabled = false;
		float left, top, right, bottom;
	};

	MoveEffect moveEffect_;
	ZoomEffect zoomEffect_;
	ShakeEffect shakeEffect_;
	FollowSettings follow_;
	Bounds bounds_;

	// 内部更新処理
	void UpdateMoveEffect(float deltaTime);
	void UpdateZoomEffect(float deltaTime);
	void UpdateShakeEffect(float deltaTime);
	void UpdateFollow(float deltaTime);
	void ApplyBounds();

	// 行列計算
	Matrix3x3 viewMatrix_;
	Matrix3x3 projectionMatrix_;
	Matrix3x3 viewportMatrix_;
	Matrix3x3 vpVpMatrix_;

	void UpdateMatrices();
};