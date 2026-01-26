#include "UIManager.h"
#include <algorithm>

// =================================================================
// 内部クラス Gauge の実装
// =================================================================

UIManager::Gauge::Gauge(TextureId frameId, TextureId barId, const Vector2& offset)
	: frame_(frameId, 1, 1, 1, 0.0f)
	, bar_(barId, 1, 1, 1, 0.0f)
	, ghost_(barId, 1, 1, 1, 0.0f) // ゴーストも同じ画像を使用
	, barOffset_(offset)
{
	Vector2 anchor = { 0.5f, 0.5f };
	bar_.SetAnchorPoint(anchor);
	ghost_.SetAnchorPoint(anchor);

	// クロップ方向を水平に設定
	bar_.SetCropDirection(CropDirection::Horizontal);
	ghost_.SetCropDirection(CropDirection::Horizontal);

	ghost_.SetBaseColor(0xFF4444FF);
}

void UIManager::Gauge::Update(float deltaTime) {
	// 1. ゴーストゲージの制御（ダメージ演出）
	if (currentRatio_ < ghostRatio_) {
		ghostTimer_ += deltaTime;
		if (ghostTimer_ > 0.5f) { // ダメージを受けてから0.5秒後に減り始める（タメ）
			// 追従速度（線形補間だと最後が遅くなるので、固定値で引くかLerpか好みで）
			ghostRatio_ += (currentRatio_ - ghostRatio_) * 5.0f * deltaTime;

			if (std::abs(ghostRatio_ - currentRatio_) < 0.01f) {
				ghostRatio_ = currentRatio_;
			}
		}
	}
	else {
		// 回復時は即座に追従
		ghostRatio_ = currentRatio_;
		ghostTimer_ = 0.0f;
	}

	// クロップ率に反映
	bar_.SetCropRatio(currentRatio_);
	ghost_.SetCropRatio(ghostRatio_);

	// 2. 瀕死演出（リスクフィードバック）
	// 残り20%以下かつ0より大きい場合
	if (currentRatio_ <= 0.2f && currentRatio_ > 0.0f) {
		// 枠を揺らす（危機感）
		if (!frame_.IsShakeActive()) {

		}

		// バーを赤く点滅させる（警告）
		if (!bar_.IsFlashBlinking()) {
			// 赤色で無限回点滅
			bar_.StartFlashBlink(0xFF0000FF, 9999, 0.4f, kBlendModeAdd,2);
		}
	}
	else {
		bar_.StopFlashBlink();
	}

	frame_.Update(deltaTime);
	bar_.Update(deltaTime);
	ghost_.Update(deltaTime);
}

void UIManager::Gauge::Draw(const Vector2& pos, float scale) {
	// フレーム描画（中央基準と仮定）
	frame_.SetPosition(pos);
	frame_.SetScale(scale, scale);
	frame_.DrawScreen();

	// バーの位置計算（フレームの位置 ＋ オフセット × スケール）
	// 回転がある場合は回転行列が必要だが、UIなので省略
	Vector2 barPos = { pos.x + barOffset_.x * scale, pos.y + barOffset_.y * scale };

	// ゴースト描画（奥）
	ghost_.SetPosition(barPos);
	ghost_.SetScale(scale, scale);
	ghost_.DrawScreen();

	// メインバー描画（手前）
	bar_.SetPosition(barPos);
	bar_.SetScale(scale, scale);
	bar_.DrawScreen();
}

void UIManager::Gauge::SetRatio(float ratio) {
	float prev = currentRatio_;
	currentRatio_ = std::clamp(ratio, 0.0f, 1.0f);

	// ダメージを受けた瞬間
	if (currentRatio_ < prev) {
		ghostTimer_ = 0.0f; // タメ時間リセット

		// 大きなダメージ（10%以上）なら枠を一瞬大きく揺らす
		if (prev - currentRatio_ > 0.1f) {
			frame_.StartShake(5.0f, 0.3f);
		}
	}
	// 回復した瞬間
	else if (currentRatio_ > prev) {
		// 回復エフェクトとして一瞬緑に光らせる
		bar_.StartFlashBlink(0x00FF00FF, 1, 0.3f, kBlendModeAdd);
	}
}

// =================================================================
// UIManager 本体実装
// =================================================================

void UIManager::Initialize() {
	// --- ゲージ初期化 ---
	// オフセットはアセットに合わせて調整してください（例: -200px左にずらす）
	playerHP_ = std::make_unique<Gauge>(TextureId::PlayerHPFrame, TextureId::PlayerHPBar, Vector2(0.0f, 0.0f));
	playerHP_->SetColor(0x00FF00FF); // 緑

	bossHP_ = std::make_unique<Gauge>(TextureId::BossHPFrame, TextureId::BossHPBar, Vector2(-150.0f, 0.0f));
	bossHP_->SetColor(0xFF0000FF);   // 赤

	// --- キーガイド初期化 ---
	keyW_ = std::make_unique<DrawComponent2D>(TextureId::KeyW, 1, 1, 1, 0.0f);
	keyA_ = std::make_unique<DrawComponent2D>(TextureId::KeyA, 1, 1, 1, 0.0f);
	keyS_ = std::make_unique<DrawComponent2D>(TextureId::KeyS, 1, 1, 1, 0.0f);
	keyD_ = std::make_unique<DrawComponent2D>(TextureId::KeyD, 1, 1, 1, 0.0f);
	keyK_ = std::make_unique<DrawComponent2D>(TextureId::KeyK, 1, 1, 1, 0.0f);
	keyJ_ = std::make_unique<DrawComponent2D>(TextureId::KeyJ, 1, 1, 1, 0.0f);

	keyDash_ = std::make_unique<DrawComponent2D>(TextureId::Icon_Dash, 1, 1, 1, 0.0f);
	keyJump_ = std::make_unique<DrawComponent2D>(TextureId::KeyJump, 1, 1, 1, 0.0f);
	keyBoomerangThrow_ = std::make_unique<DrawComponent2D>(TextureId::Icon_BoomerangThrow, 1, 1, 1, 0.0f);
	keyBoomerangReturn_ = std::make_unique<DrawComponent2D>(TextureId::Icon_BoomerangReturn, 1, 1, 1, 0.0f);

	// ========= Padキーガイド 初期化 =========
	padStickUp_ = std::make_unique<DrawComponent2D>(TextureId::PadStickUp, 1, 1, 1, 0.0f);
	padStickDown_ = std::make_unique<DrawComponent2D>(TextureId::PadStickDown, 1, 1, 1, 0.0f);
	padStickLeft_ = std::make_unique<DrawComponent2D>(TextureId::PadStickLeft, 1, 1, 1, 0.0f);
	padStickRight_ = std::make_unique<DrawComponent2D>(TextureId::PadStickRight, 1, 1, 1, 0.0f);
	padButtonA_ = std::make_unique<DrawComponent2D>(TextureId::PadButtonA, 1, 1, 1, 0.0f);
	padButtonB_ = std::make_unique<DrawComponent2D>(TextureId::PadButtonB, 1, 1, 1, 0.0f);
	/*padButtonDash_ = std::make_unique<DrawComponent2D>(TextureId::PadButtonDash, 1, 1, 1, 0.0f);
	padButtonJump_ = std::make_unique<DrawComponent2D>(TextureId::PadButtonJump, 1, 1, 1, 0.0f);
	padButtonBoomerang_ = std::make_unique<DrawComponent2D>(TextureId::PadButtonBoomerang, 1, 1, 1, 0.0f);*/

	// =========== Icon 初期化 ===========
	iconDash_ = std::make_unique<DrawComponent2D>(TextureId::Icon_Dash, 1, 1, 1, 0.0f);
	iconBoomerangThrow_ = std::make_unique<DrawComponent2D>(TextureId::Icon_BoomerangThrow, 1, 1, 1, 0.0f);
	iconBoomerangReturn_ = std::make_unique<DrawComponent2D>(TextureId::Icon_BoomerangReturn, 1, 1, 1, 0.0f);

	skillIconDash_ = std::make_unique<SkillIcon>(skillIconPos_);
	skillIconDash_->AddIconTexture(TextureId::Icon_Dash);
	skillIconBoomerang_ = std::make_unique<SkillIcon>(skillIconPos_ + skillIconOffset_);
	skillIconBoomerang_->AddIconTexture(TextureId::Icon_BoomerangThrow);
	skillIconBoomerang_->AddIconTexture(TextureId::Icon_BoomerangReturn);

	// --- ポーズ・リザルト ---
	pauseText_ = std::make_unique<DrawComponent2D>(TextureId::PauseText, 1, 1, 1, 0.0f);
	pauseBg_ = std::make_unique<DrawComponent2D>(TextureId::White1x1, 1, 1, 1, 0.0f);
	pauseBg_->SetBaseColor(0x00000080); // 半透明の黒
	pauseBg_->SetDrawSize(1280.0f, 720.0f); // 画面全体

	resultImage_ = std::make_unique<DrawComponent2D>(TextureId::ResultClear, 1, 1, 1, 0.0f);
}

void UIManager::Update(float dt) {
	if (isTitle_) return;

	// ポーズ切り替え入力
	if (InputManager::GetInstance().TriggerKey(DIK_ESCAPE)) {
		TogglePause();
	}

	// --- ポーズ中 ---
	if (isPaused_) {
		pauseText_->Update(dt);
		return; // ゲーム内の更新は止める
	}

	// --- リザルト中 ---
	if (isResult_) {
		resultImage_->Update(dt);
		return;
	}

	// --- ゲームプレイ中 ---
	if (isGamePlay_) {
		// ゲージ更新
		playerHP_->Update(dt);
		bossHP_->Update(dt);

		// キーガイド更新
		UpdateKeyGuides();
		keyW_->Update(dt);
		keyA_->Update(dt);
		keyS_->Update(dt);
		keyD_->Update(dt);
		keyK_->Update(dt);
		keyJ_->Update(dt);
	}
}

void UIManager::Draw() {
	if (isTitle_) return;

	// --- HUD (ゲームプレイ中のみ) ---
	if (isGamePlay_) {
		// 座標は適宜調整
		playerHP_->Draw({ 250.0f, 650.0f }, 1.0f);
		//bossHP_->Draw({ 640.0f, 50.0f }, 0.8f);

		InputManager& input = InputManager::GetInstance();

		// キーガイド表示
		if (input.GetInputMode() == InputMode::Gamepad) {
			// パッド操作時は非表示
		}
		else {
			// キーボード操作時は表示
			keyW_->DrawScreen();
			keyA_->DrawScreen();
			keyS_->DrawScreen();
			keyD_->DrawScreen();
			keyK_->DrawScreen();
			keyJ_->DrawScreen();
		}
	}

	// アイコン表示
	if (isGamePlay_) {
		DrawIcons();
	}

	// --- ポーズ画面 ---
	if (isPaused_) {
		pauseBg_->SetPosition({ 640.0f, 360.0f }); // 中心
		pauseBg_->DrawScreen();

		pauseText_->SetPosition({ 640.0f, 360.0f });
		pauseText_->DrawScreen();
	}

	// --- リザルト画面 ---
	if (isResult_) {
		pauseBg_->SetPosition({ 640.0f, 360.0f });
		pauseBg_->DrawScreen(); // 背景を暗くする

		resultImage_->SetPosition({ 640.0f, 360.0f });
		resultImage_->DrawScreen();
	}
}

void UIManager::DrawIcons() {
	// アイコン表示
	if (isGamePlay_) {
		skillIconDash_->Draw();
		skillIconBoomerang_->Draw();
	}
}

void UIManager::UpdateIcons(float dt, const PlayerSkillState& state) {
	if (!isGamePlay_) return;

	// ダッシュアイコン更新
	if (skillIconDash_) {
		skillIconDash_->Update(dt, state.isDashing, state.canDash);
	}

	// ブーメランアイコン更新
	if (skillIconBoomerang_) {
		bool isRecalling = (state.boomerangMode == PlayerSkillState::BoomerangMode::Recalling);

		// canUseBoomerang = true → 不透明、false → 半透明
		skillIconBoomerang_->Update(dt, isRecalling, state.canUseBoomerang, !state.canUseBoomerang);
	}
}

void UIManager::UpdateKeyGuides() {
	InputManager& input = InputManager::GetInstance();

	// キー配置（左上）
	Vector2 basePos = { 100.0f, 100.0f };
	float offset = 60.0f;

	// 各キーの位置設定と入力アニメーション
	// 入力があった瞬間、StartSquashで「むにゅっ」とさせて手触りを良くする

	if (input.GetInputMode() == InputMode::Gamepad) {
		// パッド操作時は非表示
		return;
	}
	else {
		// W
		keyW_->SetPosition({ basePos.x, basePos.y - offset });
		if (input.TriggerKey(DIK_W)) {
			keyW_->StartSquash({ 0.8f, 1.2f }, 0.1f);
		}

		// A
		keyA_->SetPosition({ basePos.x - offset, basePos.y });
		if (input.TriggerKey(DIK_A)) {
			keyA_->StartSquash({ 1.2f, 0.8f }, 0.1f);
		}

		// S
		keyS_->SetPosition({ basePos.x, basePos.y });
		if (input.TriggerKey(DIK_S)) {
			keyS_->StartSquash({ 0.8f, 1.2f }, 0.1f);
		}

		// D
		keyD_->SetPosition({ basePos.x + offset, basePos.y });
		if (input.TriggerKey(DIK_D)) {
			keyD_->StartSquash({ 1.2f, 0.8f }, 0.1f);
		}

		// Kはダッシュアイコンの下に描画
		keyK_->SetPosition({ skillIconPos_ + controllKeyOffset_ });
		if (input.TriggerKey(DIK_K)) {
			keyK_->StartSquash({ 1.2f, 0.8f }, 0.1f);
		}

		// Jはブーメランアイコンの下に描画
		keyJ_->SetPosition({ skillIconPos_ + controllKeyOffset_ + skillIconOffset_ });
		if (input.TriggerKey(DIK_J)) {
			keyJ_->StartSquash({ 1.2f, 0.8f }, 0.1f);
		}
	}
}

void UIManager::TogglePause() {
	//isPaused_ = !isPaused_;
	//if (isPaused_) {
	//	// ポーズ開始時にテキストをPopさせる
	//	pauseText_->StartPulse(0.0f, 1.0f, 0.2f, false);
	//}
}

void UIManager::ShowResult(bool isClear) {
	isResult_ = true;
	isGamePlay_ = false; // ゲーム停止
	isGameClear_ = isClear;

	if (isClear) {
		resultImage_->SetTexture(TextureId::ResultClear);
		// 勝利時は派手に
		resultImage_->StartPulse(1.0f, 1.2f, 0.5f, true);
	}
	else {
		resultImage_->SetTexture(TextureId::ResultOver);
		// 敗北時はドスンと落とすなどの演出（今回はシェイクで代用）
		resultImage_->StartShake(10.0f, 0.5f);
	}
}