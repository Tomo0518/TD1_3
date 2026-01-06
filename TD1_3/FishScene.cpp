#include "FishScene.h"
#include <Novice.h>
#include <cstdlib>
#include <ctime>
#include "SceneManager.h"

// シーンのコンストラクタ・デストラクタ
FishScene::FishScene(SceneManager& manager) : camera_({ 640.0f, 360.0f }, { 1280.0f, 720.0f }, false) {
	manager_ = &manager;
	Initialize();
}

FishScene::~FishScene() = default;

void FishScene::Initialize() {
	// ランダム初期化（必要なら外部でのみ実施）
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// 魚を生成
	const int fishCount = 20;
	fishes_.reserve(fishCount);
	for (int i = 0; i < fishCount; ++i) {
		auto fish = std::make_unique<Fish>();
		// 画面内のランダム位置に出現
		Vector2 spawn{
			static_cast<float>(std::rand() % 1280),
			static_cast<float>(std::rand() % 720)
		};
		fish->Initialize(spawn);
		fishes_.push_back(std::move(fish));
	}

	// カメラ設定（必要なら追従など）
	camera_.SetPosition({ 640.0f, 360.0f });
	camera_.SetZoom(1.0f);
}

void FishScene::Update(float deltaTime, const char* keys, const char* preKeys) {
	// キー入力でプレイヤー位置を簡易移動（矢印キー）
	if (keys && preKeys) {
		float speed = 250.0f * deltaTime;
		if (keys[DIK_LEFT])  playerPos_.x -= speed;
		if (keys[DIK_RIGHT]) playerPos_.x += speed;
		if (keys[DIK_UP])    playerPos_.y -= speed;
		if (keys[DIK_DOWN])  playerPos_.y += speed;
	}

	UpdateInternal(deltaTime);
}

void FishScene::UpdateInternal(float dt) {
	// カメラ更新
	camera_.Update(dt);

	// 魚更新
	for (auto& f : fishes_) {
		f->Update(playerPos_);
	}
}

void FishScene::Draw() {
	// 魚の描画（カメラを使用）
	for (auto& f : fishes_) {
		f->Draw(camera_);
	}

	// プレイヤー位置の目安として簡易表示
	Novice::DrawEllipse(static_cast<int>(playerPos_.x), static_cast<int>(playerPos_.y),
		6, 6, 0.0f, 0xFFFFFFFF, kFillModeSolid);
}