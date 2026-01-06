#include "CyberOverdrive.h"
#include "SceneManager.h"
#include <Novice.h>
#include <cmath>
#include <algorithm>
#include "GameShared.h"
#include "Random.h"
#include <time.h>

// 便利関数
static inline float Length(const Vector2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }
static inline Vector2 Normalize(const Vector2& v) {
	float len = Length(v);
	if (len > 0.0001f) return { v.x / len, v.y / len };
	return { 0.0f, 0.0f };
}
static inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
static inline Vector2 LerpVec(const Vector2& a, const Vector2& b, float t) { return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) }; }

CyberOverdrive::CyberOverdrive(SceneManager& manager, GameShared& shared)
	: myPlayerPos_({ 640.0f, 360.0f })
	, myPlayerVel_({ 0.0f, 0.0f })
	, myMoveSpeed_(220.0f) // 「遅め」の通常移動
	, gridScroll_(0.0f)
	, sparkTimer_(0.0f)
	, ghostTimer_(0.0f)
{
	manager_ = &manager;
	shared_ = &shared;

	Initialize();
}

void CyberOverdrive::Initialize() {
	srand((unsigned int)time(NULL));

	int pHandle = Novice::LoadTexture("./Resources/images/test2/cyberOverdrive/player_ver1.png");
	grHandleDisitalPlayer_ = Novice::LoadTexture("./Resources/images/test2/cyberOverdrive/player_ver1.png");

	myPlayerDraw_ = std::make_unique<DrawComponent2D>(pHandle, 1, 1, 1, 0.1f, true);
	myPlayerDraw_->SetAnchorPoint({ 0.5f, 0.5f }); // 中心基準
	myPlayerDraw_->SetScale(1.0f, 1.0f);

	camera_ = new Camera2D({ 640.0f,360.0f }, { 1280.0f, 720.0f }, true);

	// カメラ設定を初期位置へリセット
	camera_->SetPosition({ 640.0f, 360.0f });
	camera_->SetZoom(1.0f);

	// プレイヤー位置リセット
	myPlayerPos_ = { 640.0f, 360.0f };

	// チャージ/ブリンク初期化
	facingDir_ = { 1.0f, 0.0f };
	isCharging_ = false;
	chargeAmount_ = 0.0f;
	chargeMax_ = 1.0f;
	chargeRate_ = 1.0f;      // 1秒でMAX
	blinkTime_ = 0.0f;
	blinkActive_ = false;
	blinkForceGlitchFrames_ = 0;

	// 背景：無限グリッド用設定
	gridXSpacing_ = 128;
	gridYSpacing_ = 80;

	blinkPower_ = 2300.0f;        // 初期速度規模（チャージ比でスケール）
	blinkFriction_ = 2.8f;        // 摩擦係数（大きいほど早く減衰）
	endSpeedThreshold_ = 60.0f;   // この速度以下でブリンク終了
	chargingMoveFactor_ = 0.55f;  // チャージ中の移動係数（通常の55%）

	int grHandleSceneLabel = Novice::LoadTexture("./Resources/images/test2/cyberOverdrive/scene1.png");
	sceneLabelDraw_ = std::make_unique<DrawComponent2D>(grHandleSceneLabel, 1, 1, 1, 0.1f, true);
	sceneLabelDraw_->SetAnchorPoint({ 0.5f, 0.5f });
	sceneLabelDraw_->SetPosition({ 170.0f, 100.0f });

	int grHandleControlLabel = Novice::LoadTexture("./Resources/images/test2/cyberOverdrive/control_ver1.png");
	controlLabelDraw_ = std::make_unique<DrawComponent2D>(grHandleControlLabel, 1, 1, 1, 0.1f, true);
	controlLabelDraw_->SetAnchorPoint({ 0.5f, 0.5f });
	controlLabelDraw_->SetPosition({ 640.0f, 670.0f });
}

void CyberOverdrive::Update(float dt, const char* keys, const char* pre) {

	if (keys[DIK_RIGHT] && !pre[DIK_RIGHT]) {
		this->manager_->RequestTransition(SceneType::NightSky);
	}
	else if (keys[DIK_LEFT] && !pre[DIK_LEFT]) {
		this->manager_->RequestTransition(SceneType::AtomicBarrier);
	}

	// ---------------------------------------------------------
	// 1. 入力：向きベクトル（8方向/WASD組み合わせ）
	// ---------------------------------------------------------
	Vector2 inputDir = { 0.0f, 0.0f };
	if (keys[DIK_W]) inputDir.y += 1.0f;
	if (keys[DIK_S]) inputDir.y -= 1.0f;
	if (keys[DIK_A]) inputDir.x -= 1.0f;
	if (keys[DIK_D]) inputDir.x += 1.0f;

	if (inputDir.x != 0.0f || inputDir.y != 0.0f) {
		facingDir_ = Normalize(inputDir);
	}

	// ---------------------------------------------------------
	// 2. チャージ操作
	// ---------------------------------------------------------
	bool pressSpace = keys[DIK_SPACE] != 0;
	bool releaseSpace = (!keys[DIK_SPACE] && pre[DIK_SPACE]);

	// チャージ中
	if (pressSpace) {
		isCharging_ = true;
		chargeAmount_ = std::min(chargeMax_, chargeAmount_ + chargeRate_ * dt);

		if (chargeAmount_ >= 0.01f) {
			myPlayerDraw_->StartShake(6.0f * (chargeAmount_ / chargeMax_), 0.2f);
		}

		chargeEmitTimer_ += dt;
		float emitInterval = Lerp(0.12f, 0.03f, chargeAmount_ / chargeMax_);
		if (chargeEmitTimer_ >= emitInterval && shared_ && shared_->particleManager_) {
			Vector2 center = myPlayerPos_;
			shared_->particleManager_->EmitWithTarget(ParticleType::Charge, center, &center);
			chargeEmitTimer_ = 0.0f;
		}
	}

	// 離した瞬間（ブリンク開始
	if (releaseSpace && isCharging_) {
		isCharging_ = false;

		blinkActive_ = true;
		blinkTime_ = 0.0f;
		blinkStartPos_ = myPlayerPos_;

		// 初期速度（チャージ比でスケール）
		float blinkSpeed = blinkPower_ * (chargeAmount_ / chargeMax_);
		myPlayerVel_ = { facingDir_.x * blinkSpeed, facingDir_.y * blinkSpeed };

		// ブリンク方向を記録（疾走感演出用に使用）
		lastBlinkDir_ = Normalize(facingDir_);

		camera_->Shake(25.0f, 0.18f);

		if (shared_ && shared_->particleManager_) {
			// 角度算出（度数法）
			float angleRad = std::atan2f(facingDir_.y, facingDir_.x);
			float backDeg = angleRad * 180.0f / 3.14159265f + 180.0f;

			// 一時的にHitパラメータを向き付け
			if (auto* hitParam = shared_->particleManager_->GetParam(ParticleType::Debris)) {
				float prevBase = hitParam->angleBase;
				float prevRange = hitParam->angleRange;
				int   prevCount = hitParam->count;
				float prevSpMin = hitParam->speedMin;
				float prevSpMax = hitParam->speedMax;

				// コーン幅と強度をチャージに応じて調整
				float ratio = (chargeMax_ > 0.0f) ? (chargeAmount_ / chargeMax_) : 0.0f;
				hitParam->angleBase = backDeg;
				hitParam->angleRange = Lerp(50.0f, 30.0f, ratio);   // MAXで少し絞る
				hitParam->count = (int)Lerp(18.0f, 28.0f, ratio);
				hitParam->speedMin = Lerp(200.0f, 300.0f, ratio);
				hitParam->speedMax = Lerp(500.0f, 900.0f, ratio);

				// プレイヤー位置からEmit（複数回に分けて連続噴射）
				for (int i = 0; i < 3; ++i) {
					Vector2 emitPos = {
						myPlayerPos_.x - facingDir_.x * (12.0f * i),
						myPlayerPos_.y - facingDir_.y * (12.0f * i)
					};
					shared_->particleManager_->Emit(ParticleType::Debris, emitPos);
				}

				// 元に戻す
				hitParam->angleBase = prevBase;
				hitParam->angleRange = prevRange;
				hitParam->count = prevCount;
				hitParam->speedMin = prevSpMin;
				hitParam->speedMax = prevSpMax;
			}
			else {
				// パラメタ取得不可時のフォールバック（単発）
				shared_->particleManager_->Emit(ParticleType::Debris, myPlayerPos_);
			}
		}

		// ブリンク演出：背景グリッチ強制
		blinkForceGlitchFrames_ = 6;

		if (myPlayerDraw_) {
			myPlayerDraw_->StartSquash({ 1.4f, 1.4f }, 0.12f);
			myPlayerDraw_->StartFlash(ColorRGBA{ 255, 255, 255, 255 }, 0.08f, 1.0f);
		}
	}

	// ---------------------------------------------------------
	// 3. プレイヤー移動
	//    - チャージ中は停止
	//    - ブリンク中は補間で高速移動
	//    - 通常は「遅め」移動
	// ---------------------------------------------------------
	// プレイヤー移動
	if (blinkActive_) {
		// 摩擦減衰
		myPlayerVel_.x += -myPlayerVel_.x * blinkFriction_ * dt;
		myPlayerVel_.y += -myPlayerVel_.y * blinkFriction_ * dt;

		// 位置更新
		myPlayerPos_.x += myPlayerVel_.x * dt;
		myPlayerPos_.y += myPlayerVel_.y * dt;

		// 速度が閾値以下ならブリンク終了
		if (Length(myPlayerVel_) <= endSpeedThreshold_) {
			blinkActive_ = false;
			myPlayerVel_ = { 0.0f, 0.0f };
		}
	}
	else {
		// チャージ中でも移動可能（減速係数で弱めに動く）
		float moveFactor = isCharging_ ? chargingMoveFactor_ : 1.0f;

		// 入力に応じて加速（facingDir_は無入力でも保持）
		float inputLen = Length(inputDir);
		if (inputLen > 0.0f) {
			Vector2 moveDir = Normalize(inputDir);
			myPlayerVel_.x = moveDir.x * myMoveSpeed_ * moveFactor;
			myPlayerVel_.y = moveDir.y * myMoveSpeed_ * moveFactor;

			myPlayerPos_.x += myPlayerVel_.x * dt;
			myPlayerPos_.y += myPlayerVel_.y * dt;
		}
		else {
			// 入力なしなら軽い摩擦で自然停止
			myPlayerVel_.x += -myPlayerVel_.x * 6.0f * dt;
			myPlayerVel_.y += -myPlayerVel_.y * 6.0f * dt;
			if (Length(myPlayerVel_) > 0.1f) {
				myPlayerPos_.x += myPlayerVel_.x * dt;
				myPlayerPos_.y += myPlayerVel_.y * dt;
			}
			else {
				myPlayerVel_ = { 0.0f, 0.0f };
			}
		}
	}

	// DrawComponentに位置を反映
	if (myPlayerDraw_) {
		myPlayerDraw_->SetPosition(myPlayerPos_);

		// 矢印（facingDir_）の方向にプレイヤー画像も回す
		float rot = std::atan2f(facingDir_.y, facingDir_.x);

		myPlayerDraw_->SetRotation(rot);

		myPlayerDraw_->Update(dt);
	}

	// ---------------------------------------------------------
	// 4. ダイナミックカメラ (速度でズーム)
	// ---------------------------------------------------------
	float speedLen = Length(myPlayerVel_);
	{
		// 現カメラの中心と半サイズ（固定 1280x720 の半分）
		const Vector2 camCenter = camera_->GetPosition();
		const Vector2 halfSize{ 640.0f, 360.0f };

		// カメラの描画範囲（ワールド矩形）
		const float left = camCenter.x - halfSize.x * 0.9f;
		const float right = camCenter.x + halfSize.x * 0.9f;
		const float top = camCenter.y - halfSize.y * 0.9f;
		const float bottom = camCenter.y + halfSize.y * 0.9f;

		// プレイヤーが画面内かどうか
		const bool playerInView =
			(myPlayerPos_.x >= left && myPlayerPos_.x <= right &&
				myPlayerPos_.y >= top && myPlayerPos_.y <= bottom);

		// 条件:
		// - チャージ中 かつ 画面外 → 引く（ズームアウト）
		// - チャージしていない かつ 画面内 → 等倍へ戻す
		// - それ以外（チャージ中だが画面内／チャージしてないが画面外）は、現在のズームを維持 or 緩やかに遷移
		if (/*isCharging_ && */!playerInView) {
			camera_->ZoomTo(1.65f, 0.20f, Easing::EaseOutQuad); // 引き
		}
		else if (!isCharging_ && playerInView) {
			camera_->ZoomTo(1.0f, 0.30f, Easing::EaseOutQuad);  // 等倍
		}
		else {
			// 中間ケース：過度なゆれを避けるため、緩い戻し（任意）
			camera_->ZoomTo(1.15f, 0.45f, Easing::EaseOutQuad);
		}

		// カメラ位置追従（差分追従のまま維持）
		Vector2 camPos = camCenter;
		const float followStrength = 2.0f;
		camPos.x += (myPlayerPos_.x - camPos.x) * followStrength * dt;
		camPos.y += (myPlayerPos_.y - camPos.y) * followStrength * dt;
		camera_->SetPosition(camPos);
	}

	// ---------------------------------------------------------
	// 5. 演出：パーティクル & 残像（既存維持）
	// ---------------------------------------------------------
	if (speedLen > 100.0f && !isCharging_) {
		// デジタル火花
		sparkTimer_ += dt;
		if (sparkTimer_ > 0.08f) {
			if (shared_ && shared_->particleManager_) {
				shared_->particleManager_->Emit(ParticleType::DigitalSpark, myPlayerPos_);
			}
			sparkTimer_ = 0.0f;
		}

		// 残像生成：向きで回転を設定（生成時のプレイヤー向き）
		if (speedLen > 100.0f) {
			ghostTimer_ += dt;
			if (ghostTimer_ > 0.05f) {
				// 生成瞬間の向き
				float ghostRot = std::atan2f(facingDir_.y, facingDir_.x);
				bool flipX = false; // 回転を使うので反転は不要。必要なら向きに応じて設定。

				if (shared_ && shared_->particleManager_) {
					shared_->particleManager_->EmitDashGhost(
						myPlayerPos_,
						1.0f,
						ghostRot,           // ← 向きを渡す
						flipX,
						grHandleDisitalPlayer_
					);
				}
				ghostTimer_ = 0.0f;
			}
		}
	}

	if (shared_ && shared_->particleManager_) {
		shared_->particleManager_->Update(dt);
	}

	// 背景グリッチ強制フレーム減衰
	if (blinkForceGlitchFrames_ > 0) {
		blinkForceGlitchFrames_--;
	}

	camera_->Update(dt);
}

void CyberOverdrive::Draw() {
	// === 背景（固定グリッド：無限風） ===
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x000011FF, kFillModeSolid);

	unsigned int gridColor = 0x00FFFF44; // シアンの半透明

	// カメラのVP行列
	Matrix3x3 vp = camera_->GetVpVpMatrix();

	// 画面のワールド矩形を算出（カメラ中心±半サイズ）
	Vector2 camCenter = camera_->GetPosition();
	Vector2 half = { 640.0f * 1.8f, 360.0f * 1.8f };
	float left = camCenter.x - half.x;
	float right = camCenter.x + half.x;
	float top = camCenter.y - half.y;
	float bottom = camCenter.y + half.y;

	// グリッド開始位置を計算（ワールド座標で等間隔）
	int startX = static_cast<int>(std::floor(left / gridXSpacing_)) * gridXSpacing_;
	int endX = static_cast<int>(std::ceil(right / gridXSpacing_)) * gridXSpacing_;
	int startY = static_cast<int>(std::floor(top / gridYSpacing_)) * gridYSpacing_;
	int endY = static_cast<int>(std::ceil(bottom / gridYSpacing_)) * gridYSpacing_;

	// 縦線
	for (int gx = startX; gx <= endX; gx += gridXSpacing_) {
		Vector2 p1 = { (float)gx, top };
		Vector2 p2 = { (float)gx, bottom };
		Vector2 s1 = Matrix3x3::Transform(p1, vp);
		Vector2 s2 = Matrix3x3::Transform(p2, vp);
		Novice::DrawLine((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, gridColor);
	}
	// 横線
	for (int gy = startY; gy <= endY; gy += gridYSpacing_) {
		Vector2 p1 = { left, (float)gy };
		Vector2 p2 = { right, (float)gy };
		Vector2 s1 = Matrix3x3::Transform(p1, vp);
		Vector2 s2 = Matrix3x3::Transform(p2, vp);
		Novice::DrawLine((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, gridColor);
	}

	// ブリンク時は背景グリッチを強制（速度を疑似的に大きく渡す）
	float pseudoSpeedForGlitch = 0.0f;
	Vector2 glitchDir = lastBlinkDir_;
	if (blinkForceGlitchFrames_ > 0) {
		pseudoSpeedForGlitch = 600.0f;
	}
	else {
		if (myPlayerVel_.x != 0.0f || myPlayerVel_.y != 0.0f) {
			pseudoSpeedForGlitch = Length(myPlayerVel_);
			glitchDir = Normalize(myPlayerVel_);
		}
	}
	DrawBackgroundGlitchDirectional(pseudoSpeedForGlitch, glitchDir);

	// パーティクルの描画
	if (shared_ && shared_->particleManager_) {
		shared_->particleManager_->Draw(*camera_);
	}

	// === プレイヤー描画（グリッチ演出付き） ===
	DrawGlitchPlayer();

	// === ラベル描画 ===
	if (sceneLabelDraw_) {
		sceneLabelDraw_->DrawScreen();
	}

	if (controlLabelDraw_) {
		controlLabelDraw_->DrawScreen();
	}
}

// RGBズレ描画の実装
void CyberOverdrive::DrawGlitchPlayer() {
	if (!myPlayerDraw_) return;

	float speedLen = Length(myPlayerVel_);

	if (speedLen > 150.0f && !isCharging_) {
		Vector2 originalPos = myPlayerDraw_->GetPosition();
		unsigned int originalColor = myPlayerDraw_->GetBaseColor();

		float offset = speedLen * 0.02f;

		// R
		myPlayerDraw_->SetPosition({ originalPos.x + offset, originalPos.y });
		myPlayerDraw_->SetBaseColor(0xFF000080);
		myPlayerDraw_->Draw(*camera_);

		// B
		myPlayerDraw_->SetPosition({ originalPos.x - offset, originalPos.y });
		myPlayerDraw_->SetBaseColor(0x0000FF80);
		myPlayerDraw_->Draw(*camera_);

		// 本体
		myPlayerDraw_->SetPosition(originalPos);
		myPlayerDraw_->SetBaseColor(originalColor);
		myPlayerDraw_->Draw(*camera_);
	}
	else {
		myPlayerDraw_->Draw(*camera_);
	}
}

// 方向付き背景グリッチ
void CyberOverdrive::DrawBackgroundGlitchDirectional(float speed, const Vector2& dir) {
	if (speed < 100.0f) return;

	// カメラ行列取得
	Matrix3x3 vp = camera_->GetVpVpMatrix();

	// 方向の正規化（ゼロ回避）
	Vector2 ndir = Normalize(dir);
	if (Length(ndir) < 0.0001f) ndir = { 1.0f, 0.0f };

	// 直交軸（右手系：dirを90度回転）
	Vector2 perp = { -ndir.y, ndir.x };

	float intensity = std::clamp((speed - 100.0f) / 400.0f, 0.0f, 1.0f);
	int count = static_cast<int>(Lerp(8.0f, 30.0f, intensity));

	// 長辺（進行方向へ伸ばす）・厚み
	float w = Lerp(120.0f, 420.0f, intensity);
	float h = Lerp(8.0f, 18.0f, intensity);
	float hw = w * 0.5f;
	float hh = h * 0.5f;

	// 画面を覆うワールド矩形内でランダム配置（カメラ中心±半サイズ）
	Vector2 camCenter = camera_->GetPosition();
	Vector2 half = { 640.0f * camera_->GetZoom(), 360.0f * camera_->GetZoom() };
	float left = camCenter.x - half.x;
	float right = camCenter.x + half.x;
	float top = camCenter.y - half.y;
	float bottom = camCenter.y + half.y;

	for (int i = 0; i < count; ++i) {
		// 矩形中心（ワールド）をランダムに
		float cx = Lerp(left, right, (float)(rand() % 1000) / 1000.0f);
		float cy = Lerp(top, bottom, (float)(rand() % 1000) / 1000.0f);
		Vector2 center = { cx, cy };

		// ローカル軸合成（ワールド頂点）
		auto W = [&](float lx, float ly) -> Vector2 {
			return {
				center.x + ndir.x * lx + perp.x * ly,
				center.y + ndir.y * lx + perp.y * ly
			};
			};

		// 4頂点（ワールド）
		Vector2 wLT = W(-hw, -hh);
		Vector2 wRT = W(hw, -hh);
		Vector2 wLB = W(-hw, hh);
		Vector2 wRB = W(hw, hh);

		// スクリーンへ
		Vector2 vLT = Matrix3x3::Transform(wLT, vp);
		Vector2 vRT = Matrix3x3::Transform(wRT, vp);
		Vector2 vLB = Matrix3x3::Transform(wLB, vp);
		Vector2 vRB = Matrix3x3::Transform(wRB, vp);

		// 色（速度でアルファ強化）
		unsigned int alpha = static_cast<unsigned int>(Lerp(5.0f, 20.0f, intensity));
		unsigned int color;
		switch (rand() % 4) {
		case 0: color = (0x00FFFF00) | alpha; break; // シアン
		case 1: color = (0xFF00FF00) | alpha; break; // マゼンタ
		case 2: color = (0xFFFFFF00) | alpha; break; // 白
		default: color = (0x00000000) | alpha; break; // 黒
		}

		Novice::DrawQuad(
			(int)vLT.x, (int)vLT.y,
			(int)vRT.x, (int)vRT.y,
			(int)vLB.x, (int)vLB.y,
			(int)vRB.x, (int)vRB.y,
			0, 0, 1, 1,
			shared_->texWhite,
			color
		);
	}
}