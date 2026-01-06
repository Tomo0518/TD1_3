#include "Player.h"
#include "Camera2D.h"
#include <Novice.h>

Player::Player() {
	// テクスチャをロード
	textureHandle_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");

	// 新しい DrawComponent2D を使用（アニメーション付き）
	// 80x80のスプライトシートを 5x5分割、5フレーム、0.1秒/フレーム
	drawComp_ = new DrawComponent2D(textureHandle_, 5, 1, 5, 0.1f, true);

	// 初期設定
	drawComp_->SetPosition(position_);
	drawComp_->SetScale({ 1.0f, 1.0f });  // 2倍サイズで描画
	drawComp_->SetAnchorPoint({ 0.5f, 0.5f });  // 中心を基準点に
}

Player::~Player() {
	delete drawComp_;
}

void Player::Move(float deltaTime, const char* keys) {
	velocity_ = { 0.0f, 0.0f };

	// WASD で移動
	if (keys[DIK_W]) {
		velocity_.y = moveSpeed_;
	}

	if (keys[DIK_S]) {
		velocity_.y = -moveSpeed_;
	}

	if (keys[DIK_A]) {
		velocity_.x = -moveSpeed_;
	}

	if (keys[DIK_D]) {
		velocity_.x = moveSpeed_;
	}

	// 斜め移動の速度を正規化
	if (velocity_.x != 0.0f && velocity_.y != 0.0f) {
		float length = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		if (length > 0.0f) {
			velocity_.x = (velocity_.x / length) * moveSpeed_;
			velocity_.y = (velocity_.y / length) * moveSpeed_;
		}
	}

	// 位置を更新
	position_.x += velocity_.x * deltaTime;
	position_.y += velocity_.y * deltaTime;

	// 画面内に制限
	//position_.x = std::clamp(position_.x, 32.0f, 1280.0f - 32.0f);
	//position_.y = std::clamp(position_.y, 32.0f, 720.0f - 32.0f);
}

void Player::Update(float deltaTime, const char* keys, const char* pre, bool isDebugMode) {
	if (!isAlive_) return;

	// 移動処理
	Move(deltaTime, keys);

	// ========== エフェクトテスト用のキー入力 ==========

	if (!isDebugMode) {
		// Q: シェイクエフェクト
		if (keys[DIK_Q] && !pre[DIK_Q]) {
			drawComp_->StartShake(10.0f, 0.3f);
		}

		// E: 回転エフェクト
		if (keys[DIK_R] && !pre[DIK_R]) {
			if (drawComp_->IsRotationActive()) {
				drawComp_->StopRotation();
			}
			else {
				drawComp_->StartRotationContinuous(3.0f);
			}
		}

		// T: パルス（拡大縮小）
		if (keys[DIK_E] && !pre[DIK_E]) {
			if (drawComp_->IsScaleEffectActive()) {
				drawComp_->StopScale();
			}
			else {
				drawComp_->StartPulse(0.8f, 1.2f, 3.0f, true);
			}
		}

		// Y: フラッシュ（白）
		if (keys[DIK_Y] && !pre[DIK_Y]) {
			drawComp_->StartFlash(ColorRGBA::White(), 0.2f, 0.8f);
		}

		// U: ヒットエフェクト（複合）
		if (keys[DIK_U] && !pre[DIK_U]) {
			drawComp_->StartHitEffect();
		}

		// I: 色変更（赤）
		if (keys[DIK_I] && !pre[DIK_I]) {
			drawComp_->StartColorTransition(ColorRGBA::Red(), 0.5f);
		}

		// O: 色リセット（白）
		if (keys[DIK_O] && !pre[DIK_O]) {
			drawComp_->StartColorTransition(ColorRGBA::White(), 0.5f);
		}

		// P: フェードアウト
		if (keys[DIK_P] && !pre[DIK_P]) {
			drawComp_->StartFadeOut(1.0f);
		}

		// L: フェードイン
		if (keys[DIK_L] && !pre[DIK_L]) {
			drawComp_->StartFadeIn(0.5f);
		}

		// F: 全エフェクトリセット
		if (keys[DIK_F] && !pre[DIK_F]) {
			drawComp_->StopAllEffects();
		}
	}


	// DrawComponent2D の位置を更新
	drawComp_->SetPosition(position_);

	// DrawComponent2D を更新（アニメーション・エフェクト）
	drawComp_->Update(deltaTime);
}

void Player::Draw(const Camera2D& camera) {
	if (!isAlive_) return;

	// カメラを使って描画（ゲーム内オブジェクト）
	drawComp_->Draw(camera);
}

void Player::DrawScreen() {
	if (!isAlive_) return;

	// スクリーン座標で描画（UI用）
	drawComp_->DrawScreen();
}

#ifdef _DEBUG
void Player::DrawDebugWindow() {
	ImGui::Begin("Player Debug");

	ImGui::Text("=== Transform ===");
	ImGui::Text("Position: (%.1f, %.1f)", position_.x, position_.y);
	ImGui::Text("Velocity: (%.1f, %.1f)", velocity_.x, velocity_.y);

	ImGui::Text("=== Status ===");
	ImGui::Text("Alive: %s", isAlive_ ? "Yes" : "No");
	ImGui::Text("Radius: %.1f", radius_);

	ImGui::Text("=== Effect Controls ===");
	ImGui::Text("Q: Shake");
	ImGui::Text("E: Rotation Start");
	ImGui::Text("R: Rotation Stop");
	ImGui::Text("T: Pulse");
	ImGui::Text("Y: Flash (White)");
	ImGui::Text("U: Hit Effect");
	ImGui::Text("I: Color Transition (Red)");
	ImGui::Text("O: Color Reset (White)");
	ImGui::Text("P: Fade Out");
	ImGui::Text("L: Fade In");
	ImGui::Text("F: Reset All Effects");

	ImGui::Separator();

	ImGui::Text("=== DrawComponent Status ===");
	if (drawComp_) {
		ImGui::Text("Any Effect Active: %s",
			drawComp_->IsAnyEffectActive() ? "Yes" : "No");
		ImGui::Text("Shake Active: %s",
			drawComp_->IsShakeActive() ? "Yes" : "No");
		ImGui::Text("Rotation Active: %s",
			drawComp_->IsRotationActive() ? "Yes" : "No");
		ImGui::Text("Fade Active: %s",
			drawComp_->IsFadeActive() ? "Yes" : "No");
	}

	ImGui::End();
}
#endif