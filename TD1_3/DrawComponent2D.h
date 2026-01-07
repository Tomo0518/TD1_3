#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"
#include "Camera2D.h"
#include "Effect.h"
#include "Animation.h"
#include <Novice.h>
#include <memory>
#include "TextureManager.h"
#include "Transform2D.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

/// <summary>
/// DrawComponent2D
/// </summary>
class DrawComponent2D {
public:
	// ========== コンストラクタ ==========

	/// <summary>
	/// アニメーション用コンストラクタ
	/// </summary>
	/// <param name="graphHandle">画像ハンドル</param>
	/// <param name="divX">横分割数</param>
	/// <param name="divY">縦分割数</param>
	/// <param name="totalFrames">総フレーム数</param>
	/// <param name="speed">アニメーション速度（秒/フレーム）</param>
	/// <param name="isLoop">ループ再生するか</param>
	DrawComponent2D(int graphHandle, int divX, int divY, int totalFrames,
		float speed, bool isLoop = true);

	/// <summary>
	/// 静止画用コンストラクタ
	/// </summary>
	/// <param name="graphHandle">画像ハンドル</param>
	explicit DrawComponent2D(int graphHandle);

	/// <summary>
	/// デフォルトコンストラクタ（後で画像を設定する場合）
	/// </summary>
	DrawComponent2D();

	~DrawComponent2D() = default;

	// コピー・ムーブ
	DrawComponent2D(const DrawComponent2D& other);
	DrawComponent2D(DrawComponent2D&& other) noexcept;
	DrawComponent2D& operator=(const DrawComponent2D& other);
	DrawComponent2D& operator=(DrawComponent2D&& other) noexcept;

	// ========== 更新 ==========

	/// <summary>
	/// 毎フレーム呼ぶ更新処理
	/// - アニメーションのフレーム進行
	/// - エフェクトの更新
	/// </summary>
	void Update(float deltaTime);

	// ========== 描画 ==========

	/// <summary>
	/// カメラを使った描画（ゲーム内オブジェクト用）
	/// </summary>
	void Draw(const Camera2D& camera);


	/// <summary>
	/// アニメーション、エフェクト、基本パラメータの初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// ワールド座標での描画（カメラなし）
	/// </summary>
	void DrawWorld();

	/// <summary>
	/// スクリーン座標での描画（UI用）
	/// </summary>
	void DrawScreen();

	/// <summary>
	/// Y軸反転描画
	/// </summary>
	void DrawInternal(const Matrix3x3* vpMatrix);


	// ========== 位置・変形設定 ==========
	void SetTransform(const Transform2D& transform) { transform_ = transform; }
	Transform2D GetTransform() const { return transform_; }

	void SetPosition(const Vector2& pos) { transform_.position = pos; }
	Vector2 GetPosition() const { return transform_.position; }

	void SetScale(const Vector2& scale) { transform_.scale = scale; }
	void SetScale(float x, float y) { transform_.scale = { x, y }; }
	Vector2 GetScale() const { return transform_.scale; }

	void SetRotation(float radians) { transform_.rotation = radians; }
	float GetRotation() const { return transform_.rotation; }

	void SetAnchorPoint(const Vector2& anchor) { anchorPoint_ = anchor; }
	Vector2 GetAnchorPoint() const { return anchorPoint_; }

	// ========== サイズ設定 ==========

	/// <summary>
	/// 画像の元サイズを取得（1フレーム分、読み取り専用）
	/// </summary>
	Vector2 GetImageSize() const { return imageSize_; }

	/// <summary>
	/// 描画サイズを設定（画像と異なるサイズで描画したい場合）
	/// </summary>
	void SetDrawSize(const Vector2& size) { drawSize_ = size; }
	void SetDrawSize(float width, float height) { drawSize_ = { width, height }; }
	Vector2 GetDrawSize() const { return drawSize_; }

	/// <summary>
	/// 描画サイズを画像サイズに戻す
	/// </summary>
	void ResetDrawSize() { drawSize_ = imageSize_; }

	/// <summary>
	/// 最終的な描画サイズを取得（drawSize * scale * effectScale）
	/// </summary>
	Vector2 GetFinalDrawSize() const;

	// ========== 色設定 ==========

	/// <summary>
	/// ベースカラーを設定（エフェクトなし時の色）
	/// </summary>
	void SetBaseColor(unsigned int color) { baseColor_ = color; }
	void SetBaseColor(const ColorRGBA& color) { baseColor_ = color.ToUInt(); }
	unsigned int GetBaseColor() const { return baseColor_; }

	/// <summary>
	/// エフェクト適用後の最終的な色を取得
	/// </summary>
	unsigned int GetFinalColor() const;

	// ========== 反転設定 ==========

	void SetFlipX(bool flip) { flipX_ = flip; }
	void SetFlipY(bool flip) { flipY_ = flip; }
	void SetFlip(bool flipX, bool flipY) { flipX_ = flipX; flipY_ = flipY; }
	bool IsFlipX() const { return flipX_; }
	bool IsFlipY() const { return flipY_; }

	// ========== 画像設定 ==========

	/// <summary>
	/// 途中で画像を変更したい場合
	/// </summary>
	void SetGraphHandle(int handle);
	int GetGraphHandle() const { return graphHandle_; }

	void SetTexture(TextureId textureId) {
		SetGraphHandle(TextureManager::GetInstance().GetTexture(textureId));
	}

	// ========== アニメーション制御 ==========

	void PlayAnimation();
	void StopAnimation();
	void PauseAnimation();
	void ResumeAnimation();
	void SetAnimationFrame(int frame);

	bool HasAnimation() const { return animation_ != nullptr; }
	bool IsAnimationPlaying() const;
	int GetCurrentFrame() const;
	int GetTotalFrames() const;

	// ========== エフェクト制御（Effectクラスへの委譲） ==========

	// シェイク
	void StartShake(float intensity, float duration) { effect_.StartShake(intensity, duration); }
	void StartShakeContinuous(float intensity) { effect_.StartShakeContinuous(intensity); }
	void StopShake() { effect_.StopShake(); }

	// スケール
	void StartPulse(float minScale, float maxScale, float speed, bool continuous = true) {
		effect_.StartPulse(minScale, maxScale, speed, continuous);
	}
	void StartSquash(const Vector2& targetScale, float duration) {
		effect_.StartSquash(targetScale, duration);
	}
	void StopScale() { effect_.StopScale(); }

	// 回転
	void StartRotation(float speed, float duration) { effect_.StartRotation(speed, duration); }
	void StartRotationContinuous(float speed) { effect_.StartRotationContinuous(speed); }
	void StartWobble(float angle, float speed) { effect_.StartWobble(angle, speed); }
	void StopRotation() { effect_.StopRotation(); }

	// フェード・色
	void StartFadeOut(float duration) { effect_.StartFadeOut(duration); }
	void StartFadeIn(float duration) { effect_.StartFadeIn(duration); }
	void StartColorTransition(const ColorRGBA& targetColor, float duration) {
		effect_.StartColorTransition(targetColor, duration);
	}
	void StartFlash(const ColorRGBA& flashColor, float duration, float intensity = 1.0f) {
		effect_.StartFlash(flashColor, duration, intensity);
	}
	void StopFade() { effect_.StopFade(); }

	// 複合エフェクト
	void StartHitEffect() { effect_.StartHitEffect(); }
	void StartDeathEffect() { effect_.StartDeathEffect(); }
	void StartSpawnEffect() { effect_.StartSpawnEffect(); }

	// エフェクトリセット
	void StopAllEffects() { effect_.StopAll(); }

	// エフェクト状態確認
	bool IsAnyEffectActive() const { return effect_.IsAnyActive(); }
	bool IsShakeActive() const { return effect_.IsShakeActive(); }
	bool IsRotationActive() const { return effect_.IsRotationActive(); }
	bool IsFadeActive() const { return effect_.IsFadeActive(); }
	bool IsScaleEffectActive() const { return effect_.IsScaleActive(); }

	/// <summary>
	/// 内容を再設定する（パーティクルの再利用などで使用）
	/// </summary>
	void Setup(int graphHandle, int divX = 1, int divY = 1, int totalFrames = 1, float speed = 0.0f, bool isLoop = false);


	// ========== デバッグ ==========

#ifdef _DEBUG
	void DrawDebugWindow(const char* windowName);
#endif

private:
	// ========== 画像情報 ==========
	int graphHandle_ = -1;
	Vector2 imageSize_ = { 0.0f, 0.0f };      // 画像の元サイズ（1フレーム分）
	Vector2 drawSize_ = { 0.0f, 0.0f };       // 描画サイズ
	std::unique_ptr<Animation> animation_;   // アニメーション管理

	// ========== 変形パラメータ ==========
	//Vector2 position_ = { 0.0f, 0.0f };
	//Vector2 scale_ = { 1.0f, 1.0f };
	//float rotation_ = 0.0f;

	Transform2D transform_;          // 変換行列計算用ヘルパー

	Vector2 anchorPoint_ = { 0.5f, 0.5f };    // 中心点（0.0～1.0）

	// ========== 描画設定 ==========
	unsigned int baseColor_ = 0xFFFFFFFF;
	bool flipX_ = false;
	bool flipY_ = false;

	// ========== エフェクト ==========
	Effect effect_;

	// ========== 内部処理 ==========

	/// <summary>
	/// 画像サイズを自動取得してimageSize_を設定
	/// </summary>
	void InitializeImageSize(int divX, int divY);

	/// <summary>
	/// エフェクト適用後の最終的な変換行列を取得
	/// </summary>
	Matrix3x3 GetFinalTransformMatrix() const;

	/// <summary>
	/// エフェクト適用後の最終的な位置を取得
	/// </summary>
	Vector2 GetFinalPosition() const;

	/// <summary>
	/// エフェクト適用後の最終的なスケールを取得
	/// </summary>
	Vector2 GetFinalScale() const;

	/// <summary>
	/// エフェクト適用後の最終的な回転を取得
	/// </summary>
	float GetFinalRotation() const;

	/// <summary>
	/// 描画用の頂点座標を計算
	/// </summary>
	//void CalculateVertices(Vector2 vertices[4]) const;

	/// <summary>
	/// アニメーションのソース矩形を取得
	/// </summary>
	void GetSourceRect(int& srcX, int& srcY, int& srcW, int& srcH) const;
};