#pragma once

#include "Novice.h"
#include "Vector2.h"
#include "WindowSize.h"
#include "DrawComponent2D.h"
#include "Camera2D.h"
#include <vector>
#include <memory>

/// <summary>
/// 新・背景クラス（DrawComponent2D を使用）
/// - カメラ対応
/// - 簡単な初期化
/// - エフェクト対応
/// </summary>
class Background {
public:
	// ========== コンストラクタ ==========

	/// <summary>
	/// 静止画背景の作成
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="position">配置位置（ワールド座標）</param>
	Background(int textureHandle, const Vector2& position = { 0.0f, 0.0f });

	/// <summary>
	/// アニメーション背景の作成
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="divX">横分割数</param>
	/// <param name="divY">縦分割数</param>
	/// <param name="totalFrames">総フレーム数</param>
	/// <param name="speed">アニメーション速度</param>
	/// <param name="position">配置位置（ワールド座標）</param>
	/// <param name="isLoop">ループ再生するか</param>
	Background(int textureHandle, int divX, int divY, int totalFrames,
		float speed, const Vector2& position = { 0.0f, 0.0f }, bool isLoop = true);

	~Background() = default;

	// ========== 更新・描画 ==========

	/// <summary>
	/// 更新処理（アニメーション・エフェクト）
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// カメラを使った描画（ゲーム内背景）
	/// </summary>
	void Draw(const Camera2D& camera);

	/// <summary>
	/// スクリーン座標での描画（UI背景）
	/// </summary>
	void DrawScreen();

	/// <summary>
	/// カスタムオフセットでの描画（旧互換用）
	/// </summary>
	void DrawWithOffset(const Vector2& offset, unsigned int color = 0xFFFFFFFF);

	// ========== 位置・サイズ設定 ==========

	void SetPosition(const Vector2& pos);
	Vector2 GetPosition() const;

	void SetDrawSize(float width, float height);
	void SetDrawSize(const Vector2& size);
	Vector2 GetDrawSize() const;

	void SetScale(float x, float y);
	void SetScale(const Vector2& scale);

	void SetColor(unsigned int color);
	unsigned int GetColor() const;

	// ========== カリング設定 ==========

	/// <summary>
	/// カメラの視界内にあるかチェック
	/// </summary>
	bool IsVisible(const Camera2D& camera, float margin = 0.0f) const;

	/// <summary>
	/// カリングを有効化/無効化（デフォルト: 有効）
	/// </summary>
	void SetCullingEnabled(bool enabled) { cullingEnabled_ = enabled; }
	bool IsCullingEnabled() const { return cullingEnabled_; }

	// ========== エフェクト ==========

	void StartPulse(float minScale, float maxScale, float speed);
	void StartFadeIn(float duration);
	void StartFadeOut(float duration);
	void StartColorTransition(const ColorRGBA& targetColor, float duration);
	void StopAllEffects();

private:
	std::unique_ptr<DrawComponent2D> drawComp_;
	bool cullingEnabled_ = true;

	// カリング用の境界ボックスを計算
	void CalculateBounds(float& left, float& right, float& top, float& bottom) const;
};


// ========================================
// 新しい背景レイヤーシステム
// ========================================

/// <summary>
/// 背景レイヤーの情報を保持する構造体
/// パララックススクロール対応
/// </summary>
class BackgroundLayer {
public:
	BackgroundLayer() = default;

	/// <summary>
	/// レイヤーを初期化（タイル配置用）
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="tileWidth">タイルの横幅</param>
	/// <param name="tileHeight">タイルの縦幅</param>
	/// <param name="parallaxFactor">視差係数（0.0～1.0）</param>
	void Initialize(int textureHandle, float tileWidth, float tileHeight, float parallaxFactor = 1.0f);

	/// <summary>
	/// タイルを追加
	/// </summary>
	void AddTile(const Vector2& position);

	/// <summary>
	/// 複数タイルを自動配置（横方向）
	/// </summary>
	/// <param name="startX">開始X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="count">タイル数</param>
	void AddTilesHorizontal(float startX, float y, int count);

	/// <summary>
	/// 複数タイルを自動配置（縦方向）
	/// </summary>
	void AddTilesVertical(float x, float startY, int count);

	/// <summary>
	/// グリッド状にタイルを配置
	/// </summary>
	void AddTilesGrid(float startX, float startY, int countX, int countY);

	/// <summary>
	/// レイヤー全体を更新
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// レイヤー全体を描画（パララックス対応）
	/// </summary>
	void Draw(const Camera2D& camera);

	/// <summary>
	/// 色を設定
	/// </summary>
	void SetColor(unsigned int color);

	/// <summary>
	/// 視差係数を設定
	/// </summary>
	void SetParallaxFactor(float factor) { parallaxFactor_ = factor; }
	float GetParallaxFactor() const { return parallaxFactor_; }

private:
	std::vector<std::unique_ptr<Background>> tiles_;
	int textureHandle_ = -1;
	float tileWidth_ = 1280.0f;
	float tileHeight_ = 720.0f;
	float parallaxFactor_ = 1.0f;
	unsigned int color_ = 0xFFFFFFFF;
};