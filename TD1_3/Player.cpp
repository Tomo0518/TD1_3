#include "Player.h"
#include "Camera2D.h"
#include <Novice.h>

#include "SceneUtilityIncludes.h"

Player::Player() {
	// テクスチャをロード

	// 新しい DrawComponent2D を使用（アニメーション付き）
	// 80x80のスプライトシートを 5x5分割、5フレーム、0.1秒/フレーム
	drawComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::PlayerAnimeNormal), 5, 1, 5, 0.1f, true);

	// 初期設定
	drawComp_->SetPosition(transform_.position);
	drawComp_->SetScale({ 1.0f, 1.0f });  // 2倍サイズで描画
	drawComp_->SetAnchorPoint({ 0.5f, 0.5f });  // 中心を基準点に
}

Player::~Player() {
	//delete drawComp_;

	Initialize();
}

void Player::Initialize() {
	transform_.position = { 640.0f, 360.0f };
}

void Player::Move(float deltaTime) {
	rigidbody_.velocity = { 0.0f, 0.0f };

	// WASD で移動
	if (Input().PressKey(DIK_W)) {
		rigidbody_.velocity.y = rigidbody_.maxSpeed;
	}

	if (Input().PressKey(DIK_S)) {
		rigidbody_.velocity.y = -rigidbody_.maxSpeed;
	}

	if (Input().PressKey(DIK_A)) {
		rigidbody_.velocity.x = -rigidbody_.maxSpeed;
	}

	if (Input().PressKey(DIK_D)) {
		rigidbody_.velocity.x = rigidbody_.maxSpeed;
	}

	// 斜め移動の速度を正規化
	if (rigidbody_.velocity.x != 0.0f && rigidbody_.velocity.y != 0.0f) {
		rigidbody_.velocity = Vector2::Normalize(rigidbody_.velocity) * rigidbody_.maxSpeed;
	}

	// 位置を更新
	Vector2 moveDelta = rigidbody_.velocity * deltaTime;
	transform_.position += moveDelta;

	// 画面内に制限
	//position_.x = std::clamp(position_.x, 32.0f, 1280.0f - 32.0f);
	//position_.y = std::clamp(position_.y, 32.0f, 720.0f - 32.0f);
}

void Player::Update(float deltaTime) {
	if (!info_.isActive) return;

	// 移動処理
	Move(deltaTime);

	// ========== エフェクトテスト用のキー入力 ==========

		// Q: シェイクエフェクト
	if (Input().TriggerKey(DIK_Q)) {
		drawComp_->StartShake(10.0f, 0.3f);
	}

	// E: 回転エフェクト
	if (Input().TriggerKey(DIK_R)) {
		if (drawComp_->IsRotationActive()) {
			drawComp_->StopRotation();
		}
		else {
			drawComp_->StartRotationContinuous(3.0f);
		}
	}

	// T: パルス（拡大縮小）
	if (Input().TriggerKey(DIK_E)) {
		if (drawComp_->IsScaleEffectActive()) {
			drawComp_->StopScale();
		}
		else {
			drawComp_->StartPulse(0.8f, 1.2f, 3.0f, true);
		}
	}

	// Y: フラッシュ（白）
	if (Input().TriggerKey(DIK_Y)) {
		drawComp_->StartFlash(ColorRGBA::White(), 0.2f, 0.8f);
	}

	// U: ヒットエフェクト（複合）
	if (Input().TriggerKey(DIK_U)) {
		drawComp_->StartHitEffect();
	}

	// I: 色変更（赤）
	if (Input().TriggerKey(DIK_I)) {
		drawComp_->StartColorTransition(ColorRGBA::Red(), 0.5f);
	}

	// O: 色リセット（白）
	if (Input().TriggerKey(DIK_O)) {
		drawComp_->StartColorTransition(ColorRGBA::White(), 0.5f);
	}

	// P: フェードアウト
	if (Input().TriggerKey(DIK_P)) {
		drawComp_->StartFadeOut(1.0f);
	}

	// L: フェードイン
	if (Input().TriggerKey(DIK_L)) {
		drawComp_->StartFadeIn(0.5f);
	}

	// F: 全エフェクトリセット
	if (Input().TriggerKey(DIK_F)) {
		drawComp_->StopAllEffects();
	}

	// DrawComponent2D の位置を更新
	drawComp_->SetPosition(transform_.position);

	// DrawComponent2D を更新（アニメーション・エフェクト）
	drawComp_->Update(deltaTime);
}

void Player::Draw(const Camera2D& camera) {
	if (!info_.isActive) return;

	// カメラを使って描画（ゲーム内オブジェクト）
	drawComp_->Draw(camera);

	DrawDebugWindow();
}

void Player::DrawScreen() {
	if (!info_.isActive) return;

	// スクリーン座標で描画（UI用）
	drawComp_->DrawScreen();
}

#ifdef _DEBUG
void Player::DrawDebugWindow() {
	ImGui::Begin("Player Debug");

	ImGui::Text("=== Transform ===");
	ImGui::Text("Position: (%.1f, %.1f)", transform_.position.x, transform_.position.y);
	ImGui::Text("Velocity: (%.1f, %.1f)", rigidbody_.velocity.x, rigidbody_.velocity.y);
	ImGui::End();

	/*
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

	ImGui::End();*/
}
#endif