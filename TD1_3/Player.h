#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include <algorithm>

#include "GameObject2D.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

#undef min

class Camera2D;

/// <summary>
/// デモ用プレイヤークラス
/// 新しい DrawComponent2D と Camera2D を使用
/// </summary>
class Player : public GameObject2D{
public:
	Player();
	~Player();

	void Initialize() override;

	// ========== 更新・描画 ==========
	void Update(float deltaTime, const char* keys, const char* pre, bool isDebugMode);
	void Draw(const Camera2D& camera);
	void DrawScreen();  // UI用（カメラなし）

	// ========== 移動 ==========
	void Move(float deltaTime);

	// ========== ゲッター ==========
	Vector2 GetPosition() const { return transform_.position; }
	Vector2 GetVelocity() const { return rigidbody_.velocity; }
	float GetRadius() const { return radius_; }
	bool IsAlive() const { return info_.isActive; }

	// 位置への const 参照を返すメソッド
	const Vector2& GetPositionRef() const { return transform_.position; }


	// ========== セッター ==========
	void SetPosition(const Vector2& pos) { transform_.position = pos; }
	void SetAlive(bool alive) { info_.isActive = alive; }

	// ========== デバッグ ==========
#ifdef _DEBUG
	void DrawDebugWindow();
#endif

private:


	// ========== パラメータ ==========
	/*Vector2 position_ = { 640.0f, 360.0f };
	Vector2 velocity_ = { 0.0f, 0.0f };
	float moveSpeed_ = 400.0f;
	bool isAlive_ = true;*/
	
	float radius_ = 32.0f;

	// ========== 描画コンポーネント ==========
	/*DrawComponent2D* drawComp_ = nullptr;
	int textureHandle_ = -1;*/
};