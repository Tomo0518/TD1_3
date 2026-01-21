#include "DebugWindow.h"
#include "Camera2D.h"
#include "Player.h"
#include "Usagi.hpp"
#include "Easing.h"
#include "ParticleManager.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

DebugWindow::DebugWindow() {
	// デフォルト値は既にヘッダーで初期化されているので、
	// 特に何もする必要がない場合は空でOK
}

void DebugWindow::DrawDebugGui() {
#ifdef _DEBUG
	ImGui::Begin("Debug Window - Unified Control");

	ImGui::Text("=== Debug Options ===");
	ImGui::Checkbox("Show Camera Debug", &showCameraWindow_);
	ImGui::Checkbox("Show Player Debug", &showPlayerWindow_);
	ImGui::Checkbox("Show Particle Debug", &showParticleWindow_);

	ImGui::End();
#endif
}

void DebugWindow::DrawCameraDebugWindow(Camera2D* camera) {
	camera;
#ifdef _DEBUG
	if (!camera || !showCameraWindow_) return;

	ImGui::Begin("Camera Debug", &showCameraWindow_);

	// =======================================
	// ショートカット説明
	// =======================================
	if (ImGui::TreeNode("Camera Controls Shortcut Keys")) {
		ImGui::Text("Arrow Keys: Move Camera Up/Left/Down/Right");
		ImGui::Text("Q/E: Zoom Camera In/Out");
		ImGui::Text("F: Reset Camera Position and Zoom");
		ImGui::TreePop();
	}

	ImGui::Separator();
	// ========================================
	// デバッグモード切り替え
	// ========================================
	ImGui::Text("=== Debug Mode ===");
	if (ImGui::Checkbox("Enable Camera Debug Mode", &camera->isDebugCamera_)) {
	}

	if (camera->isDebugCamera_) {
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.4f);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ACTIVE");
		ImGui::SetWindowFontScale(1.0f);
	}

	ImGui::Separator();

	// ========================================
	// カメラ情報表示
	// ========================================
	if (ImGui::CollapsingHeader("Camera Info", ImGuiTreeNodeFlags_DefaultOpen)) {
		Vector2 pos = camera->GetPosition();
		float zoom = camera->GetZoom();

		ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);
		ImGui::Text("Zoom: %.2fx", zoom);
		ImGui::Text("Rotation: %.2f rad", camera->rotation_);

		ImGui::Separator();

		// 編集可能な値
		ImGui::Text("Edit Values:");

		float editPos[2] = { pos.x, pos.y };
		if (ImGui::DragFloat2("Position##edit", editPos, 1.0f)) {
			camera->SetPosition({ editPos[0], editPos[1] });
		}

		float editZoom = zoom;
		if (ImGui::SliderFloat("Zoom##edit", &editZoom, 0.1f, 5.0f)) {
			camera->SetZoom(editZoom);
		}

		float editRotation = camera->rotation_;
		if (ImGui::SliderAngle("Rotation##edit", &editRotation)) {
			camera->rotation_ = editRotation;
		}

		if (ImGui::Button("Reset Camera")) {
			camera->SetPosition({ 640.0f, 360.0f });
			camera->SetZoom(1.0f);
			camera->rotation_ = 0.0f;
		}
	}

	// ========================================
	// エフェクトテスト
	// ========================================
	if (ImGui::CollapsingHeader("Camera Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Test Camera Effects:");

		// シェイク
		if (ImGui::TreeNode("Shake Effect")) {
			static float shakeIntensity = 10.0f;
			static float shakeDuration = 0.5f;

			ImGui::SliderFloat("Intensity", &shakeIntensity, 1.0f, 50.0f);
			ImGui::SliderFloat("Duration", &shakeDuration, 0.1f, 3.0f);

			if (ImGui::Button("Start Shake")) {
				camera->Shake(shakeIntensity, shakeDuration);
			}

			ImGui::SameLine();

			if (ImGui::Button("Stop Shake")) {
				camera->StopShake();
			}

			ImGui::Text("Active: %s", camera->shakeEffect_.isActive ? "Yes" : "No");
			if (camera->shakeEffect_.isActive) {
				ImGui::Text("Elapsed: %.2f / %.2f",
					camera->shakeEffect_.elapsed,
					camera->shakeEffect_.duration);
			}

			ImGui::TreePop();
		}

		// ズーム
		if (ImGui::TreeNode("Zoom Effect")) {
			static float targetZoom = 2.0f;
			static float zoomDuration = 1.0f;

			ImGui::SliderFloat("Target Zoom", &targetZoom, 0.1f, 5.0f);
			ImGui::SliderFloat("Duration##zoom", &zoomDuration, 0.1f, 5.0f);

			if (ImGui::Button("Zoom In (2x)")) {
				camera->ZoomTo(2.0f, 1.0f, Easing::EaseOutQuad);
			}

			ImGui::SameLine();

			if (ImGui::Button("Zoom Out (1x)")) {
				camera->ZoomTo(1.0f, 1.0f, Easing::EaseInOutQuad);
			}

			if (ImGui::Button("Zoom to Target")) {
				camera->ZoomTo(targetZoom, zoomDuration, Easing::EaseOutQuad);
			}

			ImGui::Text("Active: %s", camera->zoomEffect_.isActive ? "Yes" : "No");
			if (camera->zoomEffect_.isActive) {
				ImGui::Text("Progress: %.1f%%",
					(camera->zoomEffect_.elapsed / camera->zoomEffect_.duration) * 100.0f);
			}

			ImGui::TreePop();
		}

		// 移動
		if (ImGui::TreeNode("Move Effect")) {
			static float targetPos[2] = { 640.0f, 360.0f };
			static float moveDuration = 2.0f;

			ImGui::DragFloat2("Target Position", targetPos, 1.0f);
			ImGui::SliderFloat("Duration##move", &moveDuration, 0.1f, 5.0f);

			if (ImGui::Button("Move to Center")) {
				camera->MoveTo({ 640.0f, 360.0f }, 2.0f, Easing::EaseOutCubic);
			}

			ImGui::SameLine();

			if (ImGui::Button("Move to Target")) {
				camera->MoveTo({ targetPos[0], targetPos[1] }, moveDuration, Easing::EaseOutQuad);
			}

			ImGui::Text("Active: %s", camera->moveEffect_.isActive ? "Yes" : "No");
			if (camera->moveEffect_.isActive) {
				ImGui::Text("Progress: %.1f%%",
					(camera->moveEffect_.elapsed / camera->moveEffect_.duration) * 100.0f);
			}

			ImGui::TreePop();
		}
	}

	// ========================================
	// 追従設定
	// ========================================
	if (ImGui::CollapsingHeader("Follow Settings")) {
		ImGui::Text("Follow Target: %s", camera->follow_.target ? "Active" : "None");

		float followSpeed = camera->follow_.speed;
		if (ImGui::SliderFloat("Follow Speed", &followSpeed, 0.0f, 1.0f)) {
			camera->SetFollowSpeed(followSpeed);
		}

		float deadZone[2] = { camera->follow_.deadZoneWidth, camera->follow_.deadZoneHeight };
		if (ImGui::DragFloat2("Dead Zone", deadZone, 1.0f, 0.0f, 500.0f)) {
			camera->SetDeadZone(deadZone[0], deadZone[1]);
		}

		if (ImGui::Button("Clear Target")) {
			camera->SetTarget(nullptr);
		}
	}

	// ========================================
	// 境界設定
	// ========================================
	if (ImGui::CollapsingHeader("Bounds Settings")) {
		ImGui::Text("Bounds Enabled: %s", camera->bounds_.enabled ? "Yes" : "No");

		if (camera->bounds_.enabled) {
			ImGui::Text("Left: %.1f, Top: %.1f", camera->bounds_.left, camera->bounds_.top);
			ImGui::Text("Right: %.1f, Bottom: %.1f", camera->bounds_.right, camera->bounds_.bottom);

			if (ImGui::Button("Clear Bounds")) {
				camera->ClearBounds();
			}
		}
		else {
			if (ImGui::Button("Set Default Bounds")) {
				camera->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
			}
		}
	}

	// ========================================
	// キーボード操作ガイド
	// ========================================
	if (ImGui::CollapsingHeader("Keyboard Controls")) {
		if (cameraDebugMode_) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Debug Mode Active - Keyboard Controls Enabled");

			ImGui::Separator();

			ImGui::Text("=== Movement ===");
			ImGui::BulletText("Arrow Keys: Move Camera");

			ImGui::Separator();

			ImGui::Text("=== Zoom ===");
			ImGui::BulletText("PageUp: Zoom In");
			ImGui::BulletText("PageDown: Zoom Out");

			ImGui::Separator();

			ImGui::Text("=== Rotation ===");
			ImGui::BulletText("Q: Rotate Left");
			ImGui::BulletText("E: Rotate Right");

			ImGui::Separator();

			ImGui::Text("=== Quick Actions ===");
			ImGui::BulletText("R: Reset Camera");
			ImGui::BulletText("Space: Test Shake");
			ImGui::BulletText("1: Zoom In Animation");
			ImGui::BulletText("2: Zoom Out Animation");
			ImGui::BulletText("3: Move to Center");
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Debug Mode Inactive - Enable to use keyboard controls");
		}
	}

	ImGui::End();
#endif
}

void DebugWindow::DrawPlayerDebugWindow(Usagi* player) {
	player;
#ifdef _DEBUG
	if (!player || !showPlayerWindow_) return;

	ImGui::Begin("Player Debug", &showPlayerWindow_);

	Vector2 pos = player->GetPosition();
	Vector2 vel = player->GetVelocity();

	ImGui::Text("=== Transform ===");
	ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);
	ImGui::Text("Velocity: (%.1f, %.1f)", vel.x, vel.y);

	ImGui::Separator();

	ImGui::Text("=== Status ===");
	ImGui::Text("Alive: %s", player->IsAlive() ? "Yes" : "No");
	//ImGui::Text("Radius: %.1f", player->GetRadius());

	ImGui::Separator();

	ImGui::Text("=== Controls ===");
	ImGui::Text("WASD: Move Player");

	ImGui::Separator();

	ImGui::Text("=== Effect Controls ===");
	ImGui::BulletText("Q: Shake");
	ImGui::BulletText("E: Rotation Start");
	ImGui::BulletText("R: Rotation Stop");
	ImGui::BulletText("T: Pulse");
	ImGui::BulletText("Y: Flash");
	ImGui::BulletText("U: Hit Effect");
	ImGui::BulletText("I: Color Red");
	ImGui::BulletText("O: Color Reset");
	ImGui::BulletText("P: Fade Out");
	ImGui::BulletText("L: Fade In");
	ImGui::BulletText("F: Reset All Effects");

	ImGui::End();
#endif
}


// ========================================
// パーティクルデバッグウィンドウ
// ========================================
void DebugWindow::DrawParticleDebugWindow(ParticleManager* particleManager, Usagi* player) {
	particleManager;
	player;
#ifdef _DEBUG
	if (!particleManager || !showParticleWindow_) return;

	ImGui::Begin("Particle System Debug", &showParticleWindow_);

	// ========================================
	// プレイヤー情報表示
	// ========================================
	if (player) {
		ImGui::Text("Player Position: (%.1f, %.1f)", player->GetPosition().x, player->GetPosition().y);
		ImGui::Separator();
	}

	// ========================================
	// 環境パーティクルコントロール
	// ========================================
	if (ImGui::CollapsingHeader("Environment Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent();

		// 警告表示：プレイヤーが必要
		if (!player) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: Player reference not set!");
			ImGui::Text("Environment particles require player position.");
			ImGui::Unindent();
		}
		else {
			// プレイヤー位置への参照を取得
			const Vector2* playerPosPtr = &player->GetPositionRef();

			// 雨のコントロール
			ImGui::PushID("Rain");
			ImGui::Text("Rain:");
			ImGui::SameLine(150);

			bool rainActive = particleManager->continuousEmitters_.find(ParticleType::Rain) !=
				particleManager->continuousEmitters_.end() &&
				particleManager->continuousEmitters_[ParticleType::Rain].isActive;

			if (ImGui::Button(rainActive ? "Stop##Rain" : "Start##Rain", ImVec2(80, 0))) {
				if (rainActive) {
					particleManager->StopEnvironmentEffect(ParticleType::Rain);
				}
				else {
					// 追従ターゲットを設定してから開始
					particleManager->StartEnvironmentEffect(ParticleType::Rain, EmitterFollowMode::FollowTarget);
					particleManager->SetFollowTarget(ParticleType::Rain, playerPosPtr);
				}
			}

			ImGui::SameLine();
			if (rainActive) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");

				// デバッグ情報：エミッター状態
				auto& emitter = particleManager->continuousEmitters_[ParticleType::Rain];
				ImGui::SameLine();
				ImGui::Text("Timer: %.2f", emitter.timer);
			}
			else {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "INACTIVE");
			}
			ImGui::PopID();

			// 雪のコントロール
			ImGui::PushID("Snow");
			ImGui::Text("Snow:");
			ImGui::SameLine(150);

			bool snowActive = particleManager->continuousEmitters_.find(ParticleType::Snow) !=
				particleManager->continuousEmitters_.end() &&
				particleManager->continuousEmitters_[ParticleType::Snow].isActive;

			if (ImGui::Button(snowActive ? "Stop##Snow" : "Start##Snow", ImVec2(80, 0))) {
				if (snowActive) {
					particleManager->StopEnvironmentEffect(ParticleType::Snow);
				}
				else {
					// ★修正：追従ターゲットを設定してから開始
					particleManager->StartEnvironmentEffect(ParticleType::Snow, EmitterFollowMode::FollowTarget);
					particleManager->SetFollowTarget(ParticleType::Snow, playerPosPtr);
				}
			}

			ImGui::SameLine();
			if (snowActive) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");

				// デバッグ情報：エミッター状態
				auto& emitter = particleManager->continuousEmitters_[ParticleType::Snow];
				ImGui::SameLine();
				ImGui::Text("Timer: %.2f", emitter.timer);
			}
			else {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "INACTIVE");
			}
			ImGui::PopID();

			// オーブのコントロール
			ImGui::PushID("Orb");
			ImGui::Text("Orb:");
			ImGui::SameLine(150);

			bool orbActive = particleManager->continuousEmitters_.find(ParticleType::Orb) !=
				particleManager->continuousEmitters_.end() &&
				particleManager->continuousEmitters_[ParticleType::Orb].isActive;

			if (ImGui::Button(orbActive ? "Stop##Orb" : "Start##Orb", ImVec2(80, 0))) {
				if (orbActive) {
					particleManager->StopEnvironmentEffect(ParticleType::Orb);
				}
				else {
					// 追従ターゲットを設定してから開始
					particleManager->StartEnvironmentEffect(ParticleType::Orb, EmitterFollowMode::FollowTarget);
					particleManager->SetFollowTarget(ParticleType::Orb, playerPosPtr);
				}
			}

			ImGui::SameLine();
			if (orbActive) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");

				// ★デバッグ情報：エミッター状態
				auto& emitter = particleManager->continuousEmitters_[ParticleType::Orb];
				ImGui::SameLine();
				ImGui::Text("Timer: %.2f", emitter.timer);
			}
			else {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "INACTIVE");
			}
			ImGui::PopID();

			ImGui::Separator();

			// 全停止ボタン
			if (ImGui::Button("Stop All Environment Effects", ImVec2(250, 0))) {
				particleManager->StopEnvironmentEffect(ParticleType::Rain);
				particleManager->StopEnvironmentEffect(ParticleType::Snow);
				particleManager->StopEnvironmentEffect(ParticleType::Orb);
			}

			ImGui::Unindent();
		}
	}

	ImGui::Separator();

	// ========================================
	// エミッターデバッグ情報
	// ========================================
	if (ImGui::CollapsingHeader("Emitter Status")) {
		ImGui::Text("Active Emitters: %zu", particleManager->continuousEmitters_.size());

		for (const auto& [type, emitter] : particleManager->continuousEmitters_) {
			if (emitter.isActive) {
				const char* typeName = "Unknown";
				switch (type) {
				case ParticleType::Rain: typeName = "Rain"; break;
				case ParticleType::Snow: typeName = "Snow"; break;
				case ParticleType::Orb: typeName = "Orb"; break;
				default: break;
				}

				ImGui::BulletText("%s: Timer=%.2f, FollowTarget=%s",
					typeName,
					emitter.timer,
					emitter.followTarget ? "SET" : "NULL");
			}
		}
	}

	ImGui::Separator();

	// ========================================
	// アクティブパーティクル統計
	// ========================================
	if (ImGui::CollapsingHeader("Active Particles", ImGuiTreeNodeFlags_DefaultOpen)) {
		// パーティクルタイプごとにカウント
		std::map<ParticleType, int> particleCounts;
		int totalActive = 0;

		for (const auto& p : particleManager->particles_) {
			if (p.IsAlive()) {
				particleCounts[p.GetType()]++;
				totalActive++;
			}
		}

		ImGui::Text("Total Active: %d / %d", totalActive, particleManager->kMaxParticles);
		ImGui::ProgressBar(static_cast<float>(totalActive) / particleManager->kMaxParticles,
			ImVec2(-1, 0), "");

		ImGui::Separator();

		// タイプ別の詳細
		ImGui::BeginChild("ParticleTypeList", ImVec2(0, 200), true);

		const char* typeNames[] = {
			"Explosion", "Debris", "Hit", "Dust", "MuzzleFlash",
			"Rain", "Snow", "Orb", "Charge",
			"Glow", "Shockwave", "Sparkle", "Slash", "SmokeCloud"
		};

		for (int i = 0; i < 14; ++i) {
			ParticleType type = static_cast<ParticleType>(i);
			int count = particleCounts[type];

			if (count > 0) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			}

			ImGui::BulletText("%s: %d", typeNames[i], count);
			ImGui::PopStyleColor();
		}

		ImGui::EndChild();
	}

	ImGui::Separator();

	// ========================================
// パラメータ編集（完全版）
// ========================================
	if (ImGui::CollapsingHeader("Parameter Editor")) {
		static int selectedType = 5; // デフォルトはRain
		const char* typeNames[] = {
			"Explosion", "Debris", "Hit", "Dust", "MuzzleFlash",
			"Rain", "Snow", "Orb", "Charge",
			"Glow", "Shockwave", "Sparkle", "Slash", "SmokeCloud"
		};

		ImGui::Combo("Particle Type", &selectedType, typeNames, 14);

		ParticleType type = static_cast<ParticleType>(selectedType);
		ParticleParam* param = particleManager->GetParam(type);

		if (param) {
			ImGui::Separator();

			// ========================================
			// 基本設定
			// ========================================
			if (ImGui::TreeNode("Basic Settings")) {
				ImGui::SliderInt("Count", &param->count, 1, 100);
				ImGui::SliderInt("Life Min", &param->lifeMin, 1, 600);
				ImGui::SliderInt("Life Max", &param->lifeMax, 1, 600);
				if (param->lifeMin > param->lifeMax) {
					param->lifeMax = param->lifeMin;
				}
				ImGui::TreePop();
			}

			// ========================================
			// サイズ設定
			// ========================================
			if (ImGui::TreeNode("Size Settings")) {
				ImGui::SliderFloat("Size Min", &param->sizeMin, 1.0f, 256.0f);
				ImGui::SliderFloat("Size Max", &param->sizeMax, 1.0f, 256.0f);
				if (param->sizeMin > param->sizeMax) {
					param->sizeMax = param->sizeMin;
				}
				ImGui::SliderFloat("Scale Start", &param->scaleStart, 0.0f, 5.0f);
				ImGui::SliderFloat("Scale End", &param->scaleEnd, 0.0f, 5.0f);
				ImGui::TreePop();
			}

			// ========================================
			// 物理設定
			// ========================================
			if (ImGui::TreeNode("Physics Settings")) {
				ImGui::SliderFloat("Speed Min", &param->speedMin, 0.0f, 1000.0f);
				ImGui::SliderFloat("Speed Max", &param->speedMax, 0.0f, 1000.0f);
				if (param->speedMin > param->speedMax) {
					param->speedMax = param->speedMin;
				}

				ImGui::SliderFloat("Angle Base", &param->angleBase, -180.0f, 180.0f);
				ImGui::SameLine();
				if (ImGui::Button("↑##Up")) param->angleBase = -90.0f;
				ImGui::SameLine();
				if (ImGui::Button("→##Right")) param->angleBase = 0.0f;
				ImGui::SameLine();
				if (ImGui::Button("↓##Down")) param->angleBase = 90.0f;
				ImGui::SameLine();
				if (ImGui::Button("←##Left")) param->angleBase = 180.0f;

				ImGui::SliderFloat("Angle Range", &param->angleRange, 0.0f, 360.0f);
				ImGui::SameLine();
				if (ImGui::Button("360°##Full")) param->angleRange = 360.0f;

				ImGui::DragFloat2("Gravity", &param->gravity.x, 10.0f, -2000.0f, 2000.0f);
				ImGui::DragFloat2("Acceleration", &param->acceleration.x, 10.0f, -2000.0f, 2000.0f);

				ImGui::TreePop();
			}

			// ========================================
			// エミッター設定
			// ========================================
			if (ImGui::TreeNode("Emitter Settings")) {
				// Emitter Shape
				const char* shapeNames[] = { "Point", "Line", "Rectangle" };
				int currentShape = static_cast<int>(param->emitterShape);
				if (ImGui::Combo("Emitter Shape", &currentShape, shapeNames, 3)) {
					param->emitterShape = static_cast<EmitterShape>(currentShape);
				}

				ImGui::DragFloat2("Emitter Size", &param->emitterSize.x, 10.0f, 0.0f, 2000.0f);
				ImGui::DragFloat2("Emit Range", &param->emitRange.x, 1.0f, 0.0f, 500.0f);

				ImGui::TreePop();
			}

			// ========================================
			// 回転設定
			// ========================================
			if (ImGui::TreeNode("Rotation Settings")) {
				ImGui::SliderFloat("Rotation Speed Min", &param->rotationSpeedMin, -1.0f, 1.0f, "%.3f rad/frame");
				ImGui::SliderFloat("Rotation Speed Max", &param->rotationSpeedMax, -1.0f, 1.0f, "%.3f rad/frame");
				if (param->rotationSpeedMin > param->rotationSpeedMax) {
					param->rotationSpeedMax = param->rotationSpeedMin;
				}
				ImGui::TreePop();
			}

			// ========================================
			// 色設定
			// ========================================
			if (ImGui::TreeNode("Color Settings")) {
				// Color Start
				ImGui::Text("Start Color:");
				unsigned int colorStart = param->colorStart;
				float startR = ((colorStart >> 24) & 0xFF) / 255.0f;
				float startG = ((colorStart >> 16) & 0xFF) / 255.0f;
				float startB = ((colorStart >> 8) & 0xFF) / 255.0f;
				float startA = (colorStart & 0xFF) / 255.0f;
				float startColor[4] = { startR, startG, startB, startA };

				if (ImGui::ColorEdit4("##StartColor", startColor)) {
					param->colorStart =
						(static_cast<unsigned int>(startColor[0] * 255.0f) << 24) |
						(static_cast<unsigned int>(startColor[1] * 255.0f) << 16) |
						(static_cast<unsigned int>(startColor[2] * 255.0f) << 8) |
						static_cast<unsigned int>(startColor[3] * 255.0f);
				}

				// Color End
				ImGui::Text("End Color:");
				unsigned int colorEnd = param->colorEnd;
				float endR = ((colorEnd >> 24) & 0xFF) / 255.0f;
				float endG = ((colorEnd >> 16) & 0xFF) / 255.0f;
				float endB = ((colorEnd >> 8) & 0xFF) / 255.0f;
				float endA = (colorEnd & 0xFF) / 255.0f;
				float endColor[4] = { endR, endG, endB, endA };

				if (ImGui::ColorEdit4("##EndColor", endColor)) {
					param->colorEnd =
						(static_cast<unsigned int>(endColor[0] * 255.0f) << 24) |
						(static_cast<unsigned int>(endColor[1] * 255.0f) << 16) |
						(static_cast<unsigned int>(endColor[2] * 255.0f) << 8) |
						static_cast<unsigned int>(endColor[3] * 255.0f);
				}

				ImGui::TreePop();
			}

			// ========================================
			// ブレンドモード設定
			// ========================================
			if (ImGui::TreeNode("Blend Mode Settings")) {
				const char* blendModes[] = {
					"None",
					"Normal (Alpha)",
					"Add (Additive)",
					"Subtract",
					"Multiply",
					"Screen",
					"Exclusion"
				};

				int currentBlendMode = static_cast<int>(param->blendMode);
				if (ImGui::Combo("Blend Mode", &currentBlendMode, blendModes, 7)) {
					param->blendMode = static_cast<BlendMode>(currentBlendMode);
				}

				ImGui::SameLine();
				ImGui::TextDisabled("(?)");
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("Normal: Standard alpha blending");
					ImGui::Text("Add: Bright, glowing effect");
					ImGui::Text("Subtract: Dark, shadow effect");
					ImGui::EndTooltip();
				}

				ImGui::TreePop();
			}

			// ========================================
			// アニメーション設定
			// ========================================
			if (ImGui::TreeNode("Animation Settings")) {
				ImGui::Checkbox("Use Animation", &param->useAnimation);

				if (param->useAnimation) {
					ImGui::Indent();
					ImGui::SliderInt("Div X", &param->divX, 1, 10);
					ImGui::SliderInt("Div Y", &param->divY, 1, 10);
					ImGui::SliderInt("Total Frames", &param->totalFrames, 1, 100);
					ImGui::SliderFloat("Animation Speed", &param->animSpeed, 0.01f, 0.5f);
					ImGui::Unindent();
				}

				ImGui::TreePop();
			}

			// ========================================
			// 連続発生設定
			// ========================================
			if (ImGui::TreeNode("Continuous Emission Settings")) {
				ImGui::Checkbox("Is Continuous", &param->isContinuous);

				if (param->isContinuous) {
					ImGui::Indent();
					ImGui::SliderFloat("Emit Interval (sec)", &param->emitInterval, 0.01f, 5.0f);
					ImGui::Unindent();
				}

				ImGui::TreePop();
			}

			// ========================================
			// Homing（追従）設定
			// ========================================
			if (ImGui::TreeNode("Homing Settings")) {
				ImGui::Checkbox("Use Homing", &param->useHoming);

				if (param->useHoming) {
					ImGui::Indent();
					ImGui::SliderFloat("Homing Strength", &param->homingStrength, 0.0f, 1000.0f);
					ImGui::Unindent();
				}

				ImGui::TreePop();
			}

			// ========================================
			// 環境パーティクル専用設定
			// ========================================
			if (type == ParticleType::Rain || type == ParticleType::Snow || type == ParticleType::Orb) {
				if (ImGui::TreeNode("Environment Specific Settings")) {
					if (type == ParticleType::Rain) {
						ImGui::SliderFloat("Bounce Damping", &param->bounceDamping, 0.0f, 1.0f);
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("Ground bounce coefficient (0 = no bounce, 1 = full bounce)");
						}
					}

					if (type == ParticleType::Snow) {
						ImGui::SliderFloat("Wind Strength", &param->windStrength, 0.0f, 100.0f);
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("Horizontal wind swaying effect");
						}
					}

					if (type == ParticleType::Orb) {
						ImGui::SliderFloat("Float Amplitude", &param->floatAmplitude, 0.0f, 100.0f);
						ImGui::SliderFloat("Float Frequency", &param->floatFrequency, 0.1f, 5.0f);
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("Floating motion parameters (sine wave)");
						}
					}

					ImGui::TreePop();
				}
			}

			// ========================================
			// プリセット・保存
			// ========================================
			ImGui::Separator();
			ImGui::Text("=== Presets & Save ===");

			if (ImGui::Button("Reset to Default", ImVec2(200, 0))) {
				particleManager->LoadParams();
				// 再度現在のパラメータを取得
				param = particleManager->GetParam(type);
			}

			if (ImGui::Button("Save Parameters to JSON", ImVec2(200, 0))) {
				if (particleManager->SaveParamsToJson("Resources/Data/particle_params.json")) {
					// 成功メッセージ（オプション）
					ImGui::OpenPopup("SaveSuccess");
				}
			}

			// 保存成功ポップアップ
			if (ImGui::BeginPopupModal("SaveSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Parameters saved successfully!");
				if (ImGui::Button("OK", ImVec2(120, 0))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (ImGui::Button("Load Parameters from JSON", ImVec2(200, 0))) {
				particleManager->LoadParamsFromJson("Resources/Data/particle_params.json");
				// 再度現在のパラメータを取得
				param = particleManager->GetParam(type);
			}
		}
	}

	// ========================================
	// クイックテスト
	// ========================================
	if (ImGui::CollapsingHeader("Quick Test")) {
		ImGui::Text("Emit particles at center:");

		if (ImGui::Button("Explosion", ImVec2(100, 0))) {
			particleManager->Emit(ParticleType::Explosion, { 640.0f, 360.0f });
		}
		ImGui::SameLine();
		if (ImGui::Button("Debris", ImVec2(100, 0))) {
			particleManager->Emit(ParticleType::Debris, { 640.0f, 360.0f });
		}
		ImGui::SameLine();
		if (ImGui::Button("Hit", ImVec2(100, 0))) {
			particleManager->Emit(ParticleType::Hit, { 640.0f, 360.0f });
		}

		if (ImGui::Button("Clear All Particles", ImVec2(250, 0))) {
			particleManager->Clear();
		}
	}

	ImGui::End();
#endif
}