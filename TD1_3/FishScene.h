#pragma once
#include "GameSceneBase.h"
#include "SceneManager.h"
#include "Fish.h"
#include "Camera2D.h"
#include <vector>
#include <memory>

class FishScene : public GameSceneBase {
public:
	// SceneManagerから呼ばれるコンストラクタ
	explicit FishScene(SceneManager& manager);
	~FishScene() override;

	// GameSceneBase の純粋仮想を実装
	void Update(float deltaTime, const char* keys, const char* preKeys) override;
	void Draw() override;

	// 初期化（SceneManagerからの切替直後に呼ぶ想定ならコンストラクタ内で呼んでも良い）
	void Initialize();

private:
	SceneManager* manager_ = nullptr;

	// 仮のプレイヤー位置（当面固定 or 入力で移動させるならここで）
	Vector2 playerPos_{ 640.0f, 360.0f };

	// 魚の群れ
	std::vector<std::unique_ptr<Fish>> fishes_;

	// カメラ
	Camera2D camera_;

	// 内部更新
	void UpdateInternal(float dt);
};