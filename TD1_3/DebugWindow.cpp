#include "DebugWindow.h"
#include "Camera2D.h"
#include "Player.h"
#include "Usagi.hpp"
#include "Easing.h"
#include "ParticleManager.h"
#include "ParticleRegistry.h"

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
// ========================================
// パーティクルデバッグウィンドウ（レジストリベース版）
// ========================================
void DebugWindow::DrawParticleDebugWindow(ParticleManager* particleManager, Usagi* player) {
	particleManager;
	player;
#ifdef _DEBUG
	if (!particleManager || !showParticleWindow_) return;

	ImGui::Begin("Particle System Debug", &showParticleWindow_);

	// ========================================
	// ファイル操作
	// ========================================
	if (ImGui::CollapsingHeader("File Operations", ImGuiTreeNodeFlags_DefaultOpen)) {
		static char filepath[256] = "Resources/Data/particle_params.json";
		ImGui::InputText("File Path", filepath, sizeof(filepath));

		if (ImGui::Button("Save Parameters", ImVec2(140, 30))) {
			if (particleManager->SaveParamsToJson(filepath)) {
				Novice::ConsolePrintf("✓ Saved to: %s\n", filepath);
			}
			else {
				Novice::ConsolePrintf("✗ Failed to save: %s\n", filepath);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load Parameters", ImVec2(140, 30))) {
			if (particleManager->LoadParamsFromJson(filepath)) {
				Novice::ConsolePrintf("✓ Loaded from: %s\n", filepath);
			}
			else {
				Novice::ConsolePrintf("✗ Failed to load: %s\n", filepath);
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

		// レジストリから取得してタイプ別表示
		ImGui::BeginChild("ParticleTypeList", ImVec2(0, 150), true);

		const auto& allTypes = ParticleRegistry::GetAllParticleTypes();
		for (const auto& typeInfo : allTypes) {
			int count = particleCounts[typeInfo.type];

			if (count > 0) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			}

			ImGui::BulletText("%s: %d", typeInfo.name.c_str(), count);
			ImGui::PopStyleColor();
		}

		ImGui::EndChild();
	}

	ImGui::Separator();

	// ========================================
// カテゴリ別パーティクルエディタ
// ========================================
	if (ImGui::CollapsingHeader("Particle Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
		const auto& allTypes = ParticleRegistry::GetAllParticleTypes();

		// カテゴリごとにグループ化
		std::map<std::string, std::vector<const ParticleTypeInfo*>> categorized;
		for (const auto& typeInfo : allTypes) {
			categorized[typeInfo.category].push_back(&typeInfo);
		}

		// ★修正：currentEditType をタブバーの外に移動
		static ParticleType currentEditType = ParticleType::Explosion;

		// カテゴリ別タブ表示
		if (ImGui::BeginTabBar("ParticleCategories", ImGuiTabBarFlags_None)) {
			for (const auto& [category, types] : categorized) {
				if (ImGui::BeginTabItem(category.c_str())) {
					ImGui::Spacing();

					// ★追加：タブが最初に開かれたときに、このカテゴリの最初のパーティクルを選択
					if (ImGui::IsItemActivated() && !types.empty()) {
						currentEditType = types[0]->type;
					}

					// パーティクルタイプ選択ボタン
					ImGui::Text("Select Particle Type:");

					int buttonCount = 0;
					for (const auto* typeInfo : types) {
						// 選択中のパーティクルをハイライト
						bool isSelected = (currentEditType == typeInfo->type);

						// レジストリの色でボタンを装飾
						ImVec4 color = ImGui::ColorConvertU32ToFloat4(typeInfo->editorColor);

						// ★追加：選択中は明るく表示
						if (isSelected) {
							color = ImVec4(color.x * 1.5f, color.y * 1.5f, color.z * 1.5f, 1.0f);
						}

						ImGui::PushStyleColor(ImGuiCol_Button, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
							ImVec4(color.x * 1.2f, color.y * 1.2f, color.z * 1.2f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive,
							ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, 1.0f));

						if (ImGui::Button(typeInfo->name.c_str(), ImVec2(130, 30))) {
							currentEditType = typeInfo->type;
						}

						ImGui::PopStyleColor(3);

						buttonCount++;
						if (buttonCount % 4 != 0) ImGui::SameLine();
					}

					ImGui::Separator();

					// 選択中のパーティクルパラメータ編集
					ParticleParam* param = particleManager->GetParam(currentEditType);
					if (param) {
						const auto* currentInfo = ParticleRegistry::GetParticleType(currentEditType);
						if (currentInfo) {
							// ★追加：選択中のパーティクル名を強調表示
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
							ImGui::Text("Editing: %s", currentInfo->name.c_str());
							ImGui::PopStyleColor();
							ImGui::Separator();
						}

						// ========== 基本設定（既存のコードをそのまま使用）==========
						if (ImGui::TreeNode("Basic Settings")) {
							ImGui::SliderInt("Count", &param->count, 1, 100);
							ImGui::SliderInt("Life Min", &param->lifeMin, 1, 600);
							ImGui::SliderInt("Life Max", &param->lifeMax, 1, 600);
							if (param->lifeMin > param->lifeMax) param->lifeMax = param->lifeMin;
							ImGui::TreePop();
						}

						// ========== サイズ設定 ==========
						if (ImGui::TreeNode("Size Settings")) {
							ImGui::SliderFloat("Size Min", &param->sizeMin, 1.0f, 256.0f);
							ImGui::SliderFloat("Size Max", &param->sizeMax, 1.0f, 256.0f);
							if (param->sizeMin > param->sizeMax) param->sizeMax = param->sizeMin;
							ImGui::SliderFloat("Scale Start", &param->scaleStart, 0.0f, 5.0f);
							ImGui::SliderFloat("Scale End", &param->scaleEnd, 0.0f, 5.0f);
							ImGui::TreePop();
						}

						// ========== 物理設定 ==========
						if (ImGui::TreeNode("Physics Settings")) {
							ImGui::SliderFloat("Speed Min", &param->speedMin, 0.0f, 1000.0f);
							ImGui::SliderFloat("Speed Max", &param->speedMax, 0.0f, 1000.0f);
							if (param->speedMin > param->speedMax) param->speedMax = param->speedMin;

							ImGui::SliderFloat("Angle Base", &param->angleBase, -180.0f, 180.0f);
							ImGui::SameLine();
							if (ImGui::SmallButton("↑")) param->angleBase = -90.0f;
							ImGui::SameLine();
							if (ImGui::SmallButton("→")) param->angleBase = 0.0f;
							ImGui::SameLine();
							if (ImGui::SmallButton("↓")) param->angleBase = 90.0f;
							ImGui::SameLine();
							if (ImGui::SmallButton("←")) param->angleBase = 180.0f;

							ImGui::SliderFloat("Angle Range", &param->angleRange, 0.0f, 360.0f);
							ImGui::DragFloat2("Gravity", &param->gravity.x, 10.0f, -2000.0f, 2000.0f);
							ImGui::DragFloat2("Acceleration", &param->acceleration.x, 10.0f, -2000.0f, 2000.0f);
							ImGui::TreePop();
						}

						// ========= エミッター ============
						ImGui::Text("=== Emitter Settings ===");

						// EmitterShape 選択
						const char* shapeNames[] = { "Point", "Line", "Rectangle" };
						int currentShape = static_cast<int>(param->emitterShape);
						if (ImGui::Combo("Emitter Shape", &currentShape, shapeNames, 3)) {
							param->emitterShape = static_cast<EmitterShape>(currentShape);
						}

						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("Point: Spawn at a single point with random offset");
							ImGui::Text("Line: Spawn along a horizontal line");
							ImGui::Text("Rectangle: Spawn within a rectangular area");
							ImGui::EndTooltip();
						}

						// EmitterSize (width, height)
						ImGui::DragFloat2("Emitter Size", &param->emitterSize.x, 10.0f, 0.0f, 2000.0f);
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("Width and Height of emitter area");
							ImGui::Text("Line: Uses only X (width)");
							ImGui::Text("Rectangle: Uses both X and Y");
							ImGui::EndTooltip();
						}

						// EmitRange (Point用のランダムオフセット)
						ImGui::DragFloat2("Emit Range", &param->emitRange.x, 1.0f, 0.0f, 500.0f);
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("Random position offset (used for Point emitter)");
							ImGui::Text("X: Horizontal spread, Y: Vertical spread");
							ImGui::EndTooltip();
						}

						// ビジュアルプレビュー（オプション）
						if (ImGui::TreeNode("Emitter Preview")) {
							ImGui::Text("Center: (%.0f, %.0f)", 640.0f, 360.0f);

							switch (param->emitterShape) {
							case EmitterShape::Point:
								ImGui::BulletText("Single point with ±(%.0f, %.0f) random offset",
									param->emitRange.x / 2, param->emitRange.y / 2);
								break;
							case EmitterShape::Line:
								ImGui::BulletText("Horizontal line: width = %.0f", param->emitterSize.x);
								break;
							case EmitterShape::Rectangle:
								ImGui::BulletText("Rectangle: %.0f x %.0f",
									param->emitterSize.x, param->emitterSize.y);
								break;
							}

							ImGui::TreePop();
						}

						ImGui::Separator();

						// ========== 見た目設定 ==========
						if (ImGui::TreeNode("Appearance")) {
							ImGui::Text("Start Color:");
							ColorRGBA startColor = ColorRGBA::FromUInt(param->colorStart);
							float startRGBA[4] = { startColor.r, startColor.g, startColor.b, startColor.a };
							if (ImGui::ColorEdit4("##StartColor", startRGBA)) {
								param->colorStart = ColorRGBA(startRGBA[0], startRGBA[1], startRGBA[2], startRGBA[3]).ToUInt();
							}

							ImGui::Text("End Color:");
							ColorRGBA endColor = ColorRGBA::FromUInt(param->colorEnd);
							float endRGBA[4] = { endColor.r, endColor.g, endColor.b, endColor.a };
							if (ImGui::ColorEdit4("##EndColor", endRGBA)) {
								param->colorEnd = ColorRGBA(endRGBA[0], endRGBA[1], endRGBA[2], endRGBA[3]).ToUInt();
							}
							ImGui::TreePop();
						}

						// ========== ブレンドモード ==========
						if (ImGui::TreeNode("Blend Mode")) {
							const char* blendModes[] = {
								"None", "Normal", "Add", "Subtract", "Multiply", "Screen", "Exclusion"
							};
							int currentBlend = static_cast<int>(param->blendMode);
							if (ImGui::Combo("Mode", &currentBlend, blendModes, 7)) {
								param->blendMode = static_cast<BlendMode>(currentBlend);
							}
							ImGui::TreePop();
						}

						// ========== テスト発射 ==========
						ImGui::Separator();
						if (ImGui::Button("Emit at Player", ImVec2(200, 30))) {
							particleManager->Emit(currentEditType, { player->GetPosition()});
						}

						if (ImGui::Button("Reset to Default", ImVec2(200, 30))) {
							*param = particleManager->GenerateDefaultParam(currentEditType);
							const auto* info = ParticleRegistry::GetParticleType(currentEditType);
							if (info) {
								param->textureHandle = TextureManager::GetInstance().GetTexture(info->defaultTexture);
							}
							Novice::ConsolePrintf("Reset: %s\n", currentInfo->name.c_str());
						}
					}

					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
	}

	ImGui::Separator();

	// ========================================
	// クイックテスト
	// ========================================
	if (ImGui::CollapsingHeader("Quick Test")) {
		ImGui::Text("Emit at screen center:");

		const auto& allTypes = ParticleRegistry::GetAllParticleTypes();
		int count = 0;
		for (const auto& typeInfo : allTypes) {
			if (ImGui::Button(typeInfo.name.c_str(), ImVec2(100, 0))) {
				particleManager->Emit(typeInfo.type, { player->GetPosition()});
			}
			count++;
			if (count % 5 != 0) ImGui::SameLine();
		}

		ImGui::Separator();
		if (ImGui::Button("Clear All Particles", ImVec2(200, 0))) {
			particleManager->Clear();
		}
	}

	ImGui::End();
#endif
}