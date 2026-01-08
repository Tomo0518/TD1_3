#include "GamePlayScene.h"
#include "SceneManager.h"
#include "DebugWindow.h"

#include <Novice.h>

#include "Player.h"
#include "ParticleManager.h"

#include "SceneUtilityIncludes.h"

GamePlayScene::GamePlayScene(SceneManager& mgr)
	: manager_(mgr){

	grHandleBackground_ = Novice::LoadTexture("./Resources/images/explanation/background.png");
	grHandleFrame_ = Novice::LoadTexture("./Resources/images/explanation/frame.png");

	Initialize();

	particleManager_ = new ParticleManager();

	const float groundY = 0.0f;
	particleManager_->SetGroundLevel(groundY);

	debugWindow_ = std::make_unique<DebugWindow>();
}

GamePlayScene::~GamePlayScene() {
	/*if (shared_ && shared_->particleManager_) {
		shared_->particleManager_->StopAllContinuousEmit();
	}*/
}

void GamePlayScene::Initialize() {
	fade_ = 0.0f;

	objectManager_.Clear();
	player_ = nullptr;

	InitializeCamera();
	InitializeObjects();
	InitializeBackground();
}

void GamePlayScene::InitializeCamera() {
	const bool isWorldYUp = true;
	camera_ = std::make_unique<Camera2D>(Vector2{ 640.0f, 360.0f }, Vector2{ 1280.0f, 720.0f }, isWorldYUp);
	camera_->SetFollowSpeed(0.1f);
	camera_->SetDeadZone(150.0f, 100.0f);
	camera_->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
}

void GamePlayScene::InitializeObjects() {
	// Player を GameObjectManager で生成して所有させる
	player_ = objectManager_.Spawn<Player>(nullptr, "Player");
	player_->SetPosition({ 640.0f, 560.0f });

	// カメラ追従（Scene側は参照だけ持つ）
	if (camera_ && player_) {
		camera_->SetTarget(&player_->GetPositionRef());
	}
}

void GamePlayScene::InitializeBackground() {
	background_.clear();

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background0_0)));
	background_[0]->SetPosition({ -kWindowWidth, kWindowHeight*2 });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::BackgroundBlack)));
	background_[1]->SetPosition({ 0.0f, kWindowHeight*2 });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background0_2)));
	background_[2]->SetPosition({ kWindowWidth, kWindowHeight*2 });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background1_0)));
	background_[3]->SetPosition({ -kWindowWidth, kWindowHeight });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background1_1)));
	background_[4]->SetPosition({ 0.0f, kWindowHeight });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background1_2)));
	background_[5]->SetPosition({ kWindowWidth, kWindowHeight });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background2_0)));
	background_[6]->SetPosition({ -kWindowWidth, 0.0f });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background2_1)));
	background_[7]->SetPosition({ 0.0f, 0.0f });

	background_.push_back(std::make_unique<Background>(Tex().GetTexture(TextureId::Background2_2)));
	background_[8]->SetPosition({ kWindowWidth, 0.0f });
}

void GamePlayScene::Update(float dt, const char* keys, const char* pre) {
	//shared_->pad.Update();
	if (fade_ < 1.0f) {
		fade_ += dt * 4.0f;
	}

	const bool openPause =
		Input().GetPad()->Trigger(Pad::Button::Start) ||
		(!pre[DIK_ESCAPE] && keys[DIK_ESCAPE]) ||
		(!pre[DIK_RETURN] && keys[DIK_RETURN]);

	if (openPause) {
		//shared_->PlayBackSe();
		Sound().PlaySe(SeId::Decide);
		manager_.RequestPause();
		return;
	}

#ifdef _DEBUG
	if (camera_ ) {
		if (Input().TriggerKey(keys[DIK_C])) {
			isDebugCameraMove_ = !isDebugCameraMove_;
		}

		camera_->DebugMove(isDebugCameraMove_, keys, pre);
	}
	else if (camera_) {
		camera_->DebugMove(false, keys, pre);
	}
#endif

	// GameObjectManager 経由で更新
	objectManager_.Update(dt);

	// パーティクル
	particleManager_->Update(dt);

	// テスト入力（player_ は参照として使える）
	if (player_) {
		
		if (Input().TriggerKey(DIK_SPACE)) {
		particleManager_->Emit(ParticleType::Explosion, player_->GetPosition());
		}
		if (Input().TriggerKey(DIK_J)) {
			particleManager_->Emit(ParticleType::Debris, player_->GetPosition());
		}
		if (Input().TriggerKey(DIK_L)) {
			particleManager_->Emit(ParticleType::Hit, player_->GetPosition());
		}
	}

	if (camera_) {
		camera_->Update(dt);
	}
}

void GamePlayScene::Draw() {
	for (auto& background : background_) {
		background->Draw(*camera_);
	}

	particleManager_->Draw(*camera_);

	// GameObjectManager 経由で描画
	if (camera_) {
		objectManager_.Draw(*camera_);
	}

#ifdef _DEBUG
	if (debugWindow_) {
		debugWindow_->DrawDebugGui();
		debugWindow_->DrawCameraDebugWindow(camera_.get());
		debugWindow_->DrawPlayerDebugWindow(player_);
		debugWindow_->DrawParticleDebugWindow(particleManager_, player_);
	}
#endif
}