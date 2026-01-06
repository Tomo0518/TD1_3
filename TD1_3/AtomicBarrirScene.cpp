#include "AtomicBarrirScene.h"
#include <Novice.h>
#include <cmath>
#include <algorithm>

// 簡易定数
static const float kPI = 3.14159265f;

AtomicBarrierScene::AtomicBarrierScene() {
	Initialize();
}

AtomicBarrierScene::AtomicBarrierScene(SceneManager& manager) {
	manager_ = &manager;
	Initialize();
}

void AtomicBarrierScene::Initialize() {
	// カメラ初期化
	camera_ = Camera2D({ 640.0f, 360.0f }, { 1280.0f, 720.0f });

	// テクスチャロード
	particleTex_ = Novice::LoadTexture("./Resources/images/effect/star.png");
	// プレイヤー画像
	playerTex_ = particleTex_;

	// プレイヤー初期設定
	playerPos_ = { 640.0f, 360.0f };
	playerDrawer_ = std::make_unique<DrawComponent2D>(playerTex_);
	playerDrawer_->SetScale(0.4f, 0.4f); // プレイヤーは大きめ
	playerDrawer_->SetBaseColor(0xFFFFFFFF);

	// --- 核（Nucleus）の物理設定 ---
	// プレイヤーに「遅れてついてくる」ための柔らかい設定
	nucleus_.position = playerPos_;
	nucleus_.target = playerPos_;
	nucleus_.stiffness = 60.0f;  // 柔らかめ（遅れる）
	nucleus_.damping = 8.0f;     // 余韻を残す
	nucleus_.mass = 1.5f;        // 少し重い

	// 電子生成
	SpawnElectrons();

	particleManager_ = std::make_unique<ParticleManager>();

	int grHandleSceneLabel = Novice::LoadTexture("./Resources/images/test2/atomic/scene3.png");
	sceneLabelDraw_ = std::make_unique<DrawComponent2D>(grHandleSceneLabel, 1, 1, 1, 0.1f, true);
	sceneLabelDraw_->SetAnchorPoint({ 0.5f, 0.5f });
	sceneLabelDraw_->SetPosition({ 170.0f, 100.0f });

	int grHandleControlLabel = Novice::LoadTexture("./Resources/images/test2/atomic/control.png");
	controlLabelDraw_ = std::make_unique<DrawComponent2D>(grHandleControlLabel, 1, 1, 1, 0.1f, true);
	controlLabelDraw_->SetAnchorPoint({ 0.5f, 0.5f });
	controlLabelDraw_->SetPosition({ 640.0f, 670.0f });
}

void AtomicBarrierScene::SpawnElectrons() {
	electrons_.clear();

	// 3つの軌道 × 各軌道に数個のパーティクル
	const int kOrbits = 3;
	const int kParticlesPerOrbit = 4; // 各軌道に何個配置するか

	// 軌道の角度（0度, 60度, 120度）
	float orbitAngles[kOrbits] = { 0.0f, kPI / 3.0f, 2.0f * kPI / 3.0f };

	// 色（シアン、マゼンタ、ライム）
	unsigned int colors[kOrbits] = { 0x00FFFFFF, 0xFF55FFFF, 0xAAFF00FF };

	for (int i = 0; i < kOrbits; ++i) {
		for (int j = 0; j < kParticlesPerOrbit; ++j) {
			Electron e;

			// DrawComponent
			e.drawComp = std::make_unique<DrawComponent2D>(particleTex_);
			e.drawComp->SetScale(0.2f, 0.2f);

			// 色設定
			e.color = colors[i];
			e.drawComp->SetBaseColor(e.color);

			// 軌道パラメータ
			e.orbitAngleOffset = orbitAngles[i];
			e.orbitRadii = { 180.0f, 60.0f }; // 横長の楕円形状
			e.orbitSpeed = 3.0f + (j * 0.1f); // 少し速度にばらつきを出す

			// 位相をずらして配置
			e.currentPhase = (2.0f * kPI / kParticlesPerOrbit) * j;

			// --- 電子の物理設定 ---
			// 軌道に「強く吸着する」設定
			e.physics.position = playerPos_; // 初期位置
			e.physics.stiffness = 200.0f;    // 硬め（素早く戻る）
			e.physics.damping = 4.0f;        // 減衰は低め（バースト時によく揺れる）
			e.physics.mass = 1.0f;

			electrons_.push_back(std::move(e));
		}
	}
}

void AtomicBarrierScene::Update(float dt, const char* keys, const char* preKeys) {

	if (keys[DIK_RIGHT] && !preKeys[DIK_RIGHT]) {
		this->manager_->RequestTransition(SceneType::CyberOverdrive);
	}
	else if (keys[DIK_LEFT] && !preKeys[DIK_LEFT]) {
		this->manager_->RequestTransition(SceneType::NightSky);
	}

	// ==========================================
	// 1. プレイヤー移動 (WASD)
	// ==========================================
	Vector2 input = { 0, 0 };
	if (keys[DIK_W]) input.y -= 1.0f; // Y軸上向き
	if (keys[DIK_S]) input.y += 1.0f;
	if (keys[DIK_A]) input.x -= 1.0f;
	if (keys[DIK_D]) input.x += 1.0f;

	// 正規化
	if (input.x != 0.0f || input.y != 0.0f) {
		float len = sqrtf(input.x * input.x + input.y * input.y);
		input.x /= len;
		input.y /= len;
	}

	playerPos_.x += input.x * playerSpeed_ * dt;
	playerPos_.y += input.y * playerSpeed_ * dt;

	// プレイヤー描画更新
	playerDrawer_->SetPosition(playerPos_);

	// 移動に合わせてプレイヤーを少し傾けるとスピード感が出る（オプション）
	float tilt = -input.x * 0.2f;
	playerDrawer_->SetRotation(tilt);

	// 残像（ゴースト）
	ghostTimer_ += dt;
	if (ghostTimer_ > 0.08f) { // 発生頻度

		// パーティクルマネージャーの便利関数で残像を発生
		particleManager_->EmitDashGhost(
			playerDrawer_->GetPosition(),
			0.4f, // Scale
			0.0f, // Rotation
			true,
			particleTex_
		);
		ghostTimer_ = 0.0f;
	}

	// ==========================================
	// 2. 核（Nucleus）の物理更新
	// ==========================================
	// 核の目標地点 ＝ プレイヤーの現在位置
	nucleus_.target = playerPos_;
	nucleus_.Update(dt);

	// ==========================================
	// 3. バースト・アクション (Space)
	// ==========================================
	bool isBurst = false;
	if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
		isBurst = true;

		// 演出：画面シェイク＆フラッシュ
		camera_.Shake(15.0f, 0.2f);
		flashIntensity_ = 0.6f;

		particleManager_->Emit(ParticleType::Hit, nucleus_.position);

		// プレイヤーも一瞬縮む
		playerDrawer_->StartSquash({ 2.1f, 2.1f }, 0.3f);
		//playerDrawer_->StartPulse(0.8f, 1.5f, 0.4f, false);
	}

	if (keys[DIK_J]) {
		particleManager_->Emit(ParticleType::Charge, nucleus_.position);
	}

	// 演出パラメータ更新
	camera_.Update(dt);
	flashIntensity_ = (std::max)(0.0f, flashIntensity_ - dt * 4.0f);


	// ==========================================
	// 4. 電子（パーティクル）の更新
	// ==========================================
	for (auto& e : electrons_) {
		e.currentPhase += e.orbitSpeed * dt; // 回転

		float localX = cosf(e.currentPhase) * e.orbitRadii.x;
		float localY = sinf(e.currentPhase) * e.orbitRadii.y;

		// 軌道の回転行列適用
		float rotatedX = localX * cosf(e.orbitAngleOffset) - localY * sinf(e.orbitAngleOffset);
		float rotatedY = localX * sinf(e.orbitAngleOffset) + localY * cosf(e.orbitAngleOffset);

		// 目標位置セット
		e.physics.target = {
			nucleus_.position.x + rotatedX,
			nucleus_.position.y + rotatedY
		};

		// B. バースト処理
		if (isBurst) {
			// 中心（核）から外側へのベクトル
			Vector2 dir = {
				e.physics.position.x - nucleus_.position.x,
				e.physics.position.y - nucleus_.position.y
			};

			// 正規化して強い衝撃を加える
			float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
			if (len > 0.01f) {
				// 距離に応じて強さを変えると有機的になる
				float burstPower = 1400.0f;
				e.physics.ApplyImpulse({ (dir.x / len) * burstPower, (dir.y / len) * burstPower });
			}
		}

		// C. 物理更新（目標位置に向かってバネ移動）
		e.physics.Update(dt);

		// D. 描画コンポーネントへの反映
		e.drawComp->SetPosition(e.physics.position);
		e.drawComp->Update(dt);

		// E. 軌跡（トレイル）の記録
		e.trailHistory.push_front(e.physics.position);
		if (e.trailHistory.size() > 15) {
			e.trailHistory.pop_back();
		}
	}

	// 端部処理
	playerPos_.x = std::clamp(playerPos_.x, 50.0f, 1280.0f - 50.0f);
	playerPos_.y = std::clamp(playerPos_.y, 50.0f, 720.0f - 50.0f);

	playerDrawer_->Update(dt);

	particleManager_->Update(dt);
}

void AtomicBarrierScene::Draw() {
	// 背景描画（濃い宇宙色）
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x050011FF, kFillModeSolid);

	// ==========================================
	// 1. 軌跡（トレイル）の描画
	// ==========================================
	Novice::SetBlendMode(kBlendModeAdd);

	for (const auto& e : electrons_) {
		if (e.trailHistory.size() < 2) continue;

		// 履歴を線でつなぐ
		for (size_t i = 0; i < e.trailHistory.size() - 1; ++i) {
			Vector2 p1 = e.trailHistory[i];
			Vector2 p2 = e.trailHistory[i + 1];

			// 行列取得
			Matrix3x3 vpMatrix = camera_.GetVpVpMatrix();
			Vector2 s1 = Matrix3x3::Transform(p1, vpMatrix);
			Vector2 s2 = Matrix3x3::Transform(p2, vpMatrix);

			// アルファフェード（古い履歴ほど薄く）
			float alphaRatio = 1.0f - (float)i / e.trailHistory.size();
			unsigned int alpha = (unsigned int)(255.0f * alphaRatio);
			unsigned int color = (e.color & 0xFFFFFF00) | alpha;

			// 線を描画
			Novice::DrawLine((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, color);
		}
	}

	// ==========================================
	// 2. 本体描画
	// ==========================================
	// ブレンドモードを戻す
	Novice::SetBlendMode(kBlendModeNormal);

	// 電子描画
	Novice::SetBlendMode(kBlendModeAdd);
	for (auto& e : electrons_) {
		e.drawComp->Draw(camera_);
	}
	Novice::SetBlendMode(kBlendModeNormal);

	particleManager_->Draw(camera_);

	// プレイヤー描画
	playerDrawer_->Draw(camera_);

	// ==========================================
	// 3. 画面フラッシュ
	// ==========================================
	if (flashIntensity_ > 0.01f) {
		unsigned int alpha = (unsigned int)(255.0f * flashIntensity_);
		unsigned int color = 0xFFFFFF00 | alpha;
		Novice::SetBlendMode(kBlendModeAdd);
		Novice::DrawBox(0, 0, 1280, 720, 0.0f, color, kFillModeSolid);
		Novice::SetBlendMode(kBlendModeNormal);
	}

	sceneLabelDraw_->DrawScreen();

	if (controlLabelDraw_) {
		controlLabelDraw_->DrawScreen();
	}
}