#include "NightSkyScene.h"
#include <Novice.h>
#include <cstdlib>
#include <cmath>

// 乱数ユーティリティ
static float RandomRange(float min, float max) {
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

static float LengthSq(const Vector2& a, const Vector2& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return dx * dx + dy * dy;
}

NightSkyScene::NightSkyScene() {
	Initialize();
}

NightSkyScene::NightSkyScene(SceneManager& manager) {
	manager_ = &manager;
	Initialize();
}

void NightSkyScene::Initialize() {
	camera_ = Camera2D({ 640.0f, 360.0f }, { 1280.0f, 720.0f });

	// 画像読み込み
	starTextureHandle_ = Novice::LoadTexture("./Resources/images/effect/star.png");
	controlUiTextureHandle_ = Novice::LoadTexture("./Resources/images/test2/nightSky/controlUI.png");
	lensFrameTextureHandle_ = Novice::LoadTexture("./Resources/images/test2/nightSky/lenseFrame.png");
	Novice::GetTextureSize(controlUiTextureHandle_, &uiWidth_, &uiHeight_);

	particleTextureHandle_ = starTextureHandle_;

	// パーティクル描画用
	particleDrawer_ = std::make_unique<DrawComponent2D>(particleTextureHandle_);
	particleDrawer_->SetAnchorPoint({ 0.5f, 0.5f }); // 中心基準

	// 星を生成
	SpawnStars(400);

	int grHandleSceneLabel = Novice::LoadTexture("./Resources/images/test2/nightSky/scene2.png");
	sceneLabelDraw_ = std::make_unique<DrawComponent2D>(grHandleSceneLabel, 1, 1, 1, 0.1f, true);
	sceneLabelDraw_->SetAnchorPoint({ 0.5f, 0.5f });
	sceneLabelDraw_->SetPosition({ 170.0f, 100.0f });

	int grHandleControlLabel = Novice::LoadTexture("./Resources/images/test2/nightSky/controlUI.png");
	controlLabelDraw_ = std::make_unique<DrawComponent2D>(grHandleControlLabel, 1, 1, 1, 0.1f, true);
	controlLabelDraw_->SetAnchorPoint({ 0.5f, 0.5f });
	controlLabelDraw_->SetPosition({ 640.0f, 670.0f });
}

void NightSkyScene::SpawnStars(int count) {
	stars_.clear();
	stars_.reserve(count);

	for (int i = 0; i < count; ++i) {
		Star star;
		float posX = RandomRange(-200.0f, 1480.0f);
		float posY = RandomRange(-200.0f, 920.0f);
		star.originalPosition = { posX, posY };

		star.drawComp = std::make_unique<DrawComponent2D>(starTextureHandle_);
		star.drawComp->SetPosition(star.originalPosition);

		// 色のバリエーション
		int type = rand() % 10;
		if (type < 6) star.drawComp->SetBaseColor(0xFFFFFFFF); // 白
		else if (type < 8) star.drawComp->SetBaseColor(0xAADDFFFF); // 青白
		else star.drawComp->SetBaseColor(0xFFFFAA00); // 金色

		star.originalScale = RandomRange(0.1f, 0.2f);
		star.drawComp->SetScale(star.originalScale, star.originalScale);

		bool isLarge = star.originalScale >= 0.05f;
		if (isLarge) {
			star.drawComp->StartPulse(star.originalScale * 0.7f, star.originalScale * 1.3f, 2.5f, true);
		}
		else {
			star.drawComp->StartPulse(star.originalScale * 0.7f, star.originalScale * 1.3f, 3.5f, true);
		}

		stars_.push_back(std::move(star));
	}
}

void NightSkyScene::SpawnShootingStar() {
	ShootingStar ss;
	ss.drawComp = std::make_unique<DrawComponent2D>(starTextureHandle_);

	// 画面外（右上や左上）からスタート
	float startX = RandomRange(0.0f, 1280.0f);
	ss.position = { startX, -50.0f }; // 画面上部から

	// 斜め下に落ちる速度
	float speed = RandomRange(250.0f, 600.0f); // かなり速く
	float angle = RandomRange(45.0f, 135.0f) * (3.14159f / 180.0f); // 下方向への角度
	ss.velocity = { cosf(angle) * speed, sinf(angle) * speed };

	ss.isActive = true;
	ss.drawComp->SetScale(0.1f, 0.2f);
	ss.drawComp->SetBaseColor(0xFFFFDDFF); // 少し黄色がかった白
	ss.radius = 18.0f;

	shootingStars_.push_back(std::move(ss));
}

void NightSkyScene::AddTrail(const Vector2& pos) {
	TrailParticle tp;
	tp.position = pos;

	// 拡散するような動きを入れると綺麗
	tp.velocity = { RandomRange(-20.0f, 20.0f), RandomRange(-20.0f, 20.0f) };

	tp.scale = RandomRange(0.08f, 0.2f);
	tp.life = 1.0f;
	tp.decayRate = RandomRange(1.5f, 3.0f); // 消える速さ

	// ランダムカラー（シアン、ピンク、白）
	int colType = rand() % 3;
	if (colType == 0) tp.color = 0x00FFFFFF; // Cyan
	else if (colType == 1) tp.color = 0xFF55FFFF; // Pink
	else tp.color = 0xFFFFFFFF; // White

	trails_.push_back(tp);
}

void NightSkyScene::SpawnExplosion(const Vector2& pos, int count) {
	for (int i = 0; i < count; ++i) {
		TrailParticle tp;
		tp.position = pos;

		// 全方位へ飛散、速度は強め、減衰で落ち着く
		float angle = RandomRange(0.0f, 2.0f * 3.14159f);
		float speed = RandomRange(200.0f, 600.0f);
		tp.velocity = { cosf(angle) * speed, sinf(angle) * speed };

		// サイズと寿命
		tp.scale = RandomRange(0.2f, 0.4f);
		tp.life = 1.0f;
		tp.decayRate = RandomRange(1.8f, 3.5f);

		// 明るめの加算用カラー（中心が光って見える）
		unsigned int base =
			(rand() % 2 == 0) ? 0xFFEEDDFF  // ほんのり黄色
			: 0xAAEEFFFF; // シアン寄り
		tp.color = base;

		trails_.push_back(tp);
	}
}

void NightSkyScene::Update(float deltaTime, const char* keys, const char* preKeys) {

	if (keys[DIK_RIGHT] && !preKeys[DIK_RIGHT]) {
		this->manager_->RequestTransition(SceneType::AtomicBarrier);
	}
	else if (keys[DIK_LEFT] && !preKeys[DIK_LEFT]) {
		this->manager_->RequestTransition(SceneType::CyberOverdrive);
	}

	// --- ヒットストップ中の分岐 ---
	if (hitStopTimer_ > 0.0f) {

		// タイマー減衰
		hitStopTimer_ -= deltaTime;
		if (hitStopTimer_ < 0.0f) hitStopTimer_ = 0.0f;

		// 衝撃は走っている表現：カメラ揺れやフラッシュ・リコイルのみ更新
		camera_.Update(deltaTime);

		// フラッシュ減衰（高速）
		if (flashAlpha_ > 0.0f) {
			flashAlpha_ -= flashDecaySpeed_ * deltaTime;
			if (flashAlpha_ < 0.0f) flashAlpha_ = 0.0f;
		}

		// レンズ・リコイル復帰
		if (recoilScale_ < 1.0f) {
			recoilScale_ += recoilRecoverSpeed_ * deltaTime;
			if (recoilScale_ > 1.0f) recoilScale_ = 1.0f;
		}

		// 星やパーティクルの位置更新は止める（重要）
		return;
	}

	// --- 通常更新 ---
	camera_.Update(deltaTime);

	// フラッシュ減衰
	if (flashAlpha_ > 0.0f) {
		flashAlpha_ -= flashDecaySpeed_ * deltaTime;
		if (flashAlpha_ < 0.0f) flashAlpha_ = 0.0f;
	}

	// レンズ・リコイル復帰（Lerp相当：線形に戻す）
	if (recoilScale_ < 1.0f) {
		recoilScale_ += recoilRecoverSpeed_ * deltaTime;
		if (recoilScale_ > 1.0f) recoilScale_ = 1.0f;
	}

	// 1. 背景星の更新（アニメ用）
	for (auto& star : stars_) {
		star.drawComp->Update(deltaTime);
	}

	// 2. 流れ星の発生制御
	shootingStarTimer_ -= deltaTime;
	if (shootingStarTimer_ <= 0.0f) {
		SpawnShootingStar();
		shootingStarTimer_ = RandomRange(0.3f, 1.0f); // 1〜3秒に1回発生
	}

	//=======================================
	// 操作入力
	//=======================================
	// デバッグ：Spaceキーで強制発生
	//if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
	//	SpawnShootingStar();
	//}

	float moveSpeed = 500.0f;

	// レンズの移動（WASDキー）
	if (keys[DIK_W]) {
		lensPosition_.y -= moveSpeed * deltaTime;
	}
	if (keys[DIK_S]) {
		lensPosition_.y += moveSpeed * deltaTime;
	}

	if (keys[DIK_A]) {
		lensPosition_.x -= moveSpeed * deltaTime;
	}
	if (keys[DIK_D]) {
		lensPosition_.x += moveSpeed * deltaTime;
	}

	// レンズ倍率調整（Q/Eキー）
	if (keys[DIK_Q]) {
		lensMagnification_ += 1.0f * deltaTime;
		lensMagnification_ = std::clamp(lensMagnification_, 0.8f, 3.0f);
	}
	else if (keys[DIK_E]) {
		lensMagnification_ -= 1.0f * deltaTime;
		lensMagnification_ = std::clamp(lensMagnification_, 0.7f, 3.0f);
	}

	// 3. 流れ星の更新と破壊判定
	bool rightClickTriggered = Novice::IsTriggerMouse(1) != 0;

	auto itSS = shootingStars_.begin();
	while (itSS != shootingStars_.end()) {
		// 移動（ヒットストップ外なら動く）
		itSS->position.x += itSS->velocity.x * deltaTime;
		itSS->position.y += itSS->velocity.y * deltaTime;
		itSS->drawComp->SetPosition(itSS->position);
		itSS->drawComp->Update(deltaTime);

		// トレイル（一定間隔）
		itSS->spawnTimer -= deltaTime;
		if (itSS->spawnTimer <= 0.0f) {
			for (int i = 0; i < 3; ++i) AddTrail(itSS->position);
			itSS->spawnTimer = 0.016f;
		}

		// 右クリック破壊判定
		bool destroyed = false;
		if (rightClickTriggered) {
			Vector2 mousePos = lensPosition_; // 既にスクリーン座標
			// レンズ内か？
			float lensR2 = lensRadius_ * lensRadius_;
			if (LengthSq(itSS->position, lensPosition_) <= lensR2) {
				// 当たり判定（円）
				float hitR2 = itSS->radius * itSS->radius;
				if (LengthSq(itSS->position, mousePos) <= hitR2) {
					// ヒット演出発動
					hitStopTimer_ = 0.1f;
					camera_.Shake(25.0f, 0.25f); // 強度と時間
					flashAlpha_ = 1.0f;          // 真っ白から高速で減衰
					recoilScale_ = 0.8f;         // 一瞬縮む

					// 爆散パーティクル生成
					SpawnExplosion(itSS->position, 28);

					// TODO: SE再生（必要ならNovice::PlayAudioなど）
					destroyed = true;
				}
			}
		}

		// 画面外 or 破壊で削除
		if (destroyed ||
			itSS->position.y > 800.0f ||
			itSS->position.x < -200.0f ||
			itSS->position.x > 1480.0f) {
			itSS = shootingStars_.erase(itSS); // イテレータを正しく更新
		}
		else {
			++itSS;
		}
	}

	// 4. トレイルパーティクルの更新（減衰・摩擦）
	auto itTrail = trails_.begin();
	while (itTrail != trails_.end()) {
		// 摩擦で減速
		itTrail->velocity.x *= (1.0f - 2.0f * deltaTime); // 適度に減衰
		itTrail->velocity.y *= (1.0f - 2.0f * deltaTime);

		itTrail->position.x += itTrail->velocity.x * deltaTime;
		itTrail->position.y += itTrail->velocity.y * deltaTime;

		itTrail->life -= itTrail->decayRate * deltaTime;

		if (itTrail->life <= 0.0f) {
			itTrail = trails_.erase(itTrail);
		}
		else {
			++itTrail;
		}
	}


	// 端部処理
	lensPosition_.x = std::clamp(lensPosition_.x, lensRadius_, 1280.0f - lensRadius_);
	lensPosition_.y = std::clamp(lensPosition_.y, lensRadius_, 720.0f - lensRadius_);
}

// ---------------------------------------------------------
// 描画処理
// ---------------------------------------------------------

void NightSkyScene::Draw() {
	// 背景クリア
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, bgColor_, kFillModeSolid);

	// === パス1：通常描画（等倍・全画面：レンズ内は除外） ===
	DrawWorldElements(false);

	// === パス2：レンズ効果描画（拡大・レンズ内のみ） ===
	DrawWorldElements(true);



	// 装飾：十字線
	Novice::DrawLine(
		(int)lensPosition_.x - 20, (int)lensPosition_.y,
		(int)lensPosition_.x + 20, (int)lensPosition_.y,
		0xFFFFFF55
	);
	Novice::DrawLine(
		(int)lensPosition_.x, (int)lensPosition_.y - 20,
		(int)lensPosition_.x, (int)lensPosition_.y + 20,
		0xFFFFFF55
	);

	// === 画面フラッシュ（全画面矩形） ===
	if (flashAlpha_ > 0.0f) {
		// 0.0 ~ 1.0 を 0~255 アルファに変換
		int alpha = (int)(std::fmin(1.0f, flashAlpha_) * 255.0f);
		unsigned int color = (0xFFFFFF00 | (unsigned int)alpha);
		Novice::DrawBox(0, 0, 1280, 720, 0.0f, color, kFillModeSolid);
	}

	// === レンズの枠描画（リコイル倍率を適用） ===
	float visualRadius = lensRadius_ * recoilScale_;

	Novice::DrawQuad(
		(int)(lensPosition_.x - visualRadius), (int)(lensPosition_.y - visualRadius),
		(int)(lensPosition_.x + visualRadius), (int)(lensPosition_.y - visualRadius),
		(int)(lensPosition_.x - visualRadius), (int)(lensPosition_.y + visualRadius),
		(int)(lensPosition_.x + visualRadius), (int)(lensPosition_.y + visualRadius),
		0, 0, 720, 720,
		lensFrameTextureHandle_, WHITE
	);

	// === UI描画 ===

	sceneLabelDraw_->DrawScreen();

	if (controlLabelDraw_) {
		controlLabelDraw_->DrawScreen();
	}
}

// すべての要素を描画する関数
void NightSkyScene::DrawWorldElements(bool isLensEffect) {
	float baseRadius = lensRadius_ * recoilScale_;
	float r2 = baseRadius * baseRadius;

	auto FitsInLens = [&](const Vector2& centerWorld, float drawScale, const DrawComponent2D* dc) -> bool {
		Vector2 img = dc->GetImageSize();
		float w = img.x * drawScale;
		float h = img.y * drawScale;

		const float radiusScale = 0.93f; // 小さくするほど緩い
		float spriteRadius = radiusScale * (0.5f * std::sqrt(w * w + h * h));

		// 厳しさ調整パラメータ
		const float tightenPx = 2.0f;      // きつめ余白（小さくするほど緩い）

		// 許容半径を少し広げる（緩める）
		const float lensRelaxScale = 1.03f; // 1.0より大きいと緩い
		float allowed = (baseRadius * lensRelaxScale) - spriteRadius - tightenPx;

		if (allowed <= 0.0f) return false;

		float dx = centerWorld.x - lensPosition_.x;
		float dy = centerWorld.y - lensPosition_.y;
		return (dx * dx + dy * dy) <= (allowed * allowed);
		};

	// ===================================
	// A. 背景の星
	// ===================================

	Novice::SetBlendMode(kBlendModeAdd); // 加算ブレンドでキラキラ感アップ
	for (auto& star : stars_) {
		Vector2 drawPos = star.originalPosition;
		float drawScale = star.originalScale;

		// 通常描画パスではレンズ内を除外して二重表示を防ぐ
		if (!isLensEffect) {
			float dx = star.originalPosition.x - lensPosition_.x;
			float dy = star.originalPosition.y - lensPosition_.y;
			if (dx * dx + dy * dy < r2) continue;
		}

		if (isLensEffect) {
			// レンズ中心からのベクトル
			float dx = star.originalPosition.x - lensPosition_.x;
			float dy = star.originalPosition.y - lensPosition_.y;
			float distSq = dx * dx + dy * dy;

			// レンズ範囲外ならスキップ
			if (distSq > r2) continue;

			// レンズ拡大
			drawPos.x = lensPosition_.x + dx * lensMagnification_;
			drawPos.y = lensPosition_.y + dy * lensMagnification_;
			drawScale *= lensMagnification_;

			// 厳しめクリップ：スプライト全体がレンズに収まるか
			if (!FitsInLens(drawPos, drawScale, star.drawComp.get())) continue;
		}

		star.drawComp->SetPosition(drawPos);
		star.drawComp->SetScale(drawScale, drawScale);
		star.drawComp->Draw(camera_);

		if (isLensEffect) {
			star.drawComp->SetPosition(star.originalPosition);
			star.drawComp->SetScale(star.originalScale, star.originalScale);
		}
	}

	// ===================================
	// B. 流れ星本体
	// ===================================
	for (auto& ss : shootingStars_) {
		Vector2 originalPos = ss.position;
		Vector2 drawPos = originalPos;
		float baseScale = 0.02f; // 初期設定値
		float drawScale = baseScale;

		// 通常描画パスでレンズ内除外
		if (!isLensEffect) {
			float dx0 = originalPos.x - lensPosition_.x;
			float dy0 = originalPos.y - lensPosition_.y;
			if (dx0 * dx0 + dy0 * dy0 < r2) continue;
		}

		if (isLensEffect) {
			float dx = originalPos.x - lensPosition_.x;
			float dy = originalPos.y - lensPosition_.y;
			float distSq = dx * dx + dy * dy;

			if (distSq > r2) continue;

			drawPos.x = lensPosition_.x + dx * lensMagnification_;
			drawPos.y = lensPosition_.y + dy * lensMagnification_;
			drawScale *= lensMagnification_;

			if (!FitsInLens(drawPos, drawScale, ss.drawComp.get())) continue;
		}

		ss.drawComp->SetPosition(drawPos);
		ss.drawComp->SetScale(drawScale, drawScale);
		ss.drawComp->Draw(camera_);

		if (isLensEffect) {
			ss.drawComp->SetPosition(originalPos);
			ss.drawComp->SetScale(baseScale, baseScale);
		}
	}

	// ===================================
	// C. 軌跡パーティクル
	// ===================================
	for (auto& trail : trails_) {
		Vector2 drawPos = trail.position;
		float drawScale = trail.scale * trail.life; // 寿命で小さくなる

		// 通常描画パスでレンズ内除外
		if (!isLensEffect) {
			float dx0 = trail.position.x - lensPosition_.x;
			float dy0 = trail.position.y - lensPosition_.y;
			if (dx0 * dx0 + dy0 * dy0 < r2) continue;
		}

		if (isLensEffect) {
			float dx = trail.position.x - lensPosition_.x;
			float dy = trail.position.y - lensPosition_.y;
			float distSq = dx * dx + dy * dy;

			if (distSq > r2) continue;

			drawPos.x = lensPosition_.x + dx * lensMagnification_;
			drawPos.y = lensPosition_.y + dy * lensMagnification_;
			drawScale *= lensMagnification_;

			if (!FitsInLens(drawPos, drawScale, particleDrawer_.get())) continue;
		}

		particleDrawer_->SetPosition(drawPos);
		particleDrawer_->SetScale(drawScale, drawScale);

		// アルファ値を寿命に合わせる（だんだん透明に）
		unsigned int color = trail.color;
		unsigned int alpha = (unsigned int)(255.0f * std::fmax(0.0f, std::fmin(1.0f, trail.life)));
		color = (color & 0xFFFFFF00) | alpha;
		particleDrawer_->SetBaseColor(color);

		particleDrawer_->Draw(camera_);
	}

	Novice::SetBlendMode(kBlendModeNormal); // 元に戻す
}