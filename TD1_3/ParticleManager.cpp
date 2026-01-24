#include "ParticleManager.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "JsonUtil.h"
#include "json.hpp"
#include "Camera2D.h"
#include "Effect.h"
#include "TextureManager.h"

#include "ParticleRegistry.h"

// nlohmann/json の警告を抑制
#pragma warning(push)
#pragma warning(disable: 26495)  // 未初期化変数警告
#pragma warning(disable: 26819)  // switch フォールスルー警告
#include "json.hpp"
#pragma warning(pop)

#ifdef _DEBUG
#include <imgui.h>
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using nlohmann::json;

// 度数法 -> ラジアン変換
static const float kDeg2Rad = 3.14159265f / 180.0f;
// デフォルトのパラメータファイルパス
static const std::string kDefaultParamPath = "Resources/Data/particle_params.json";

ParticleManager::ParticleManager() {
	Initialize();
}

void ParticleManager::Load() {
	// シングルトン初期化用
	Initialize();
}

void ParticleManager::Initialize() {
#ifdef _DEBUG
	Novice::ConsolePrintf("=== ParticleManager::Initialize START ===\n");
#endif

	// JSONからパラメータを読み込む
	if (!LoadParamsFromJson(kDefaultParamPath)) {
		// JSONが無い場合はデフォルトを作成
		LoadDefaultParamsFromRegistry();
		SaveParamsToJson(kDefaultParamPath);
	}

	// 必ずテクスチャを再設定
	LoadCommonResources();

#ifdef _DEBUG
	Novice::ConsolePrintf("=== ParticleManager::Initialize Complete ===\n");
	Novice::ConsolePrintf("  Loaded %d particle types\n", static_cast<int>(params_.size()));

	// 全パラメータのテクスチャハンドルを確認
	for (const auto& [type, param] : params_) {
		const auto* info = ParticleRegistry::GetParticleType(type);
		if (info) {
			Novice::ConsolePrintf("  Final: %s -> Handle=%d\n",
				info->name.c_str(), param.textureHandle);
		}
	}
#endif
}

// ブレンドモード変換ヘルパー
const char* ParticleManager::BlendModeToString(BlendMode mode) {
	switch (mode) {
	case kBlendModeNone: return "None";
	case kBlendModeNormal: return "Normal";
	case kBlendModeAdd: return "Add";
	case kBlendModeSubtract: return "Subtract";
	case kBlendModeMultiply: return "Multiply";
	case kBlendModeScreen: return "Screen";
	case kBlendModeExclusion: return "Exclusion";
	default: return "Normal";
	}
}

BlendMode ParticleManager::StringToBlendMode(const std::string& str) {
	if (str == "None") return kBlendModeNone;
	if (str == "Normal") return kBlendModeNormal;
	if (str == "Add") return kBlendModeAdd;
	if (str == "Subtract") return kBlendModeSubtract;
	if (str == "Multiply") return kBlendModeMultiply;
	if (str == "Screen") return kBlendModeScreen;
	if (str == "Exclusion") return kBlendModeExclusion;
	return kBlendModeNormal;
}

void ParticleManager::Update(float deltaTime) {
	// 連続発生の処理（追従モード対応）
	for (auto& [type, emitter] : continuousEmitters_) {
		if (!emitter.isActive) continue;
		if (params_.find(type) == params_.end()) continue;

		const ParticleParam& param = params_[type];
		if (!param.isContinuous) continue;

		// タイマー更新
		emitter.timer += deltaTime / 60.0f;

		// 発生間隔を超えたら発生
		if (emitter.timer >= param.emitInterval) {
			emitter.timer -= param.emitInterval;

			// 追従モードに応じた位置計算
			Vector2 emitPos = emitter.position;
			if (emitter.followMode == EmitterFollowMode::FollowTarget && emitter.followTarget != nullptr) {
				emitPos = *emitter.followTarget;

				// ★環境パーティクルの場合、画面上端から発生
				if (type == ParticleType::Rain || type == ParticleType::Snow) {
					emitPos.y += 360.0f;  // 画面上端
				}
			}

			// ターゲットがあればEmitWithTarget、なければEmit
			if (emitter.target != nullptr) {
				EmitWithTarget(type, emitPos, emitter.target);
			}
			else {
				Emit(type, emitPos);
			}
		}
	}

	// パーティクルの更新
	for (auto& p : particles_) {
		if (p.IsAlive()) {
			// 環境パーティクルの特殊処理
			ParticleType pType = p.GetType();

			// 雪の横揺れ処理
			if (pType == ParticleType::Snow && params_.find(pType) != params_.end()) {
				const ParticleParam& param = params_[pType];
				if (param.windStrength > 0.0f) {
					// sine波で横揺れ
					float windOffset = sinf(p.GetPosition().y * 0.01f) * param.windStrength * deltaTime / 60.0f;
					Vector2 currentPos = p.GetPosition();
					currentPos.x += windOffset;
					// 注意：Particleクラスに SetPosition を追加する必要があります
				}
			}

			// 通常の更新処理
			p.Update(deltaTime / 60.0f);

			// 地面衝突判定（雨と雪のみ）
			if (pType == ParticleType::Rain || pType == ParticleType::Snow) {
				p.CheckGroundCollision(groundLevel_);
			}
		}
	}
}

// ========== Draw メソッド ==========
void ParticleManager::Draw(const Camera2D& camera) {
	// カメラから ViewProjectionMatrix を取得
	Matrix3x3 vpMatrix = camera.GetVpVpMatrix();

	// ズーム倍率（描画サイズにも反映させる）
	const float cameraZoom = camera.GetZoom();

	// パーティクルタイプごとにブレンドモードをグループ化して描画
	for (auto it = params_.begin(); it != params_.end(); ++it) {
		ParticleType type = it->first;
		const ParticleParam& param = it->second;

		Novice::SetBlendMode(param.blendMode);

		for (auto& p : particles_) {
			if (!p.IsAlive() || p.GetType() != type) continue;

			// ワールド座標（中心）
			Vector2 worldPos = p.GetPosition();

			// テクスチャ情報
			int texWidth, texHeight;
			Novice::GetTextureSize(p.GetTextureHandle(), &texWidth, &texHeight);

			// ソース矩形
			int srcX = 0, srcY = 0;
			int srcW = texWidth, srcH = texHeight;
			if (p.UseAnimation()) {
				int divX = p.GetDivX();
				int divY = p.GetDivY();
				int frame = p.GetCurrentFrame();
				srcW = texWidth / divX;
				srcH = texHeight / divY;
				int frameX = frame % divX;
				int frameY = frame / divX;
				srcX = frameX * srcW;
				srcY = frameY * srcH;
			}

			// 描画サイズ（ピクセル基準）
			float baseSize = p.GetDrawSize();
			if (baseSize <= 0.0f) {
				baseSize = static_cast<float>(srcW);
			}
			float finalScale = p.GetCurrentScale();

			// カメラズームを描画サイズに反映
			float drawWidth = baseSize * finalScale * cameraZoom;
			float drawHeight = baseSize * finalScale * cameraZoom;

			float rot = p.GetRotation();

			if (std::fabs(rot) > 1e-4f) {
				// 回転付き：ワールド空間で回転 → 各頂点を行列でスクリーンへ
				float hw = drawWidth * 0.5f;
				float hh = drawHeight * 0.5f;

				Vector2 ltLocal = { -hw,  hh };
				Vector2 rtLocal = { hw,  hh };
				Vector2 lbLocal = { -hw, -hh };
				Vector2 rbLocal = { hw, -hh };

				float c = std::cos(rot);
				float s = std::sin(rot);

				auto RotateAddCenter = [&](const Vector2& v) -> Vector2 {
					return {
						worldPos.x + (v.x * c - v.y * s),
						worldPos.y + (v.x * s + v.y * c)
					};
					};

				Vector2 wLT = RotateAddCenter(ltLocal);
				Vector2 wRT = RotateAddCenter(rtLocal);
				Vector2 wLB = RotateAddCenter(lbLocal);
				Vector2 wRB = RotateAddCenter(rbLocal);

				Vector2 vLT = Matrix3x3::Transform(wLT, vpMatrix);
				Vector2 vRT = Matrix3x3::Transform(wRT, vpMatrix);
				Vector2 vLB = Matrix3x3::Transform(wLB, vpMatrix);
				Vector2 vRB = Matrix3x3::Transform(wRB, vpMatrix);

				Novice::DrawQuad(
					static_cast<int>(vLT.x), static_cast<int>(vLT.y),
					static_cast<int>(vRT.x), static_cast<int>(vRT.y),
					static_cast<int>(vLB.x), static_cast<int>(vLB.y),
					static_cast<int>(vRB.x), static_cast<int>(vRB.y),
					srcX, srcY, srcW, srcH,
					p.GetTextureHandle(),
					p.GetCurrentColor()
				);
			}
			else {
				Vector2 screenPos = Matrix3x3::Transform(worldPos, vpMatrix);

				float offsetX = screenPos.x - drawWidth * 0.5f;
				float offsetY = screenPos.y - drawHeight * 0.5f;

				Novice::DrawQuad(
					static_cast<int>(offsetX), static_cast<int>(offsetY),
					static_cast<int>(offsetX + drawWidth), static_cast<int>(offsetY),
					static_cast<int>(offsetX), static_cast<int>(offsetY + drawHeight),
					static_cast<int>(offsetX + drawWidth), static_cast<int>(offsetY + drawHeight),
					srcX, srcY, srcW, srcH,
					p.GetTextureHandle(),
					p.GetCurrentColor()
				);
			}
		}
	}

	Novice::SetBlendMode(kBlendModeNormal);
}

// ========== Emit メソッド（拡張版） ==========
void ParticleManager::Emit(ParticleType type, const Vector2& pos) {
	// 指定されたタイプの設定を取得
	if (params_.find(type) == params_.end()) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager::Emit - Invalid ParticleType\n");
#endif
		return;
	}

	const ParticleParam& param = params_[type];

	// テクスチャが無効な場合はスキップ
	if (param.textureHandle < 0) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager::Emit - Invalid texture handle: %d\n", param.textureHandle);
#endif
		return;
	}

	// 設定された個数ぶん発生させる
	for (int i = 0; i < param.count; ++i) {
		Particle& p = GetNextParticle();

		// パーティクルタイプを設定
		p.SetType(type);

		// --- ランダム計算 ---
		int life = static_cast<int>(RandomFloat(static_cast<float>(param.lifeMin), static_cast<float>(param.lifeMax)));

		// Emitter Shape に応じた座標生成
		Vector2 spawnPos = GenerateEmitPosition(pos, param);

		// 速度ベクトル
		float speed = RandomFloat(param.speedMin, param.speedMax);
		float halfRange = param.angleRange / 2.0f;
		float angleDeg = param.angleBase + RandomFloat(-halfRange, halfRange);
		float angleRad = angleDeg * (3.14159265f / 180.0f);
		Vector2 vel = { cosf(angleRad) * speed, sinf(angleRad) * speed };

		// 加速度の合成（重力 + acceleration）
		Vector2 totalAcc = param.gravity + param.acceleration;

		// 回転速度のランダム化
		float rotSpeed = RandomFloat(param.rotationSpeedMin, param.rotationSpeedMax);

		// サイズをランダムに決定（これを描画サイズとして使用）
		float size = RandomFloat(param.sizeMin, param.sizeMax);

		// パーティクル初期化
		p.Initialize(
			spawnPos, vel, totalAcc, life,
			param.textureHandle,
			param.scaleStart, param.scaleEnd,
			param.colorStart, param.colorEnd,
			0.0f, rotSpeed,
			param.blendMode,
			size,  // 描画サイズを渡す
			// アニメーションパラメータを渡す
			param.useAnimation,
			param.divX,
			param.divY,
			param.totalFrames,
			param.animSpeed
		);

		// ★オーブの場合は Stationary に設定
		if (type == ParticleType::Orb) {
			p.SetBehavior(ParticleBehavior::Stationary);
		}
		else {
			p.SetBehavior(ParticleBehavior::Physics);
		}
	}
}

Vector2 ParticleManager::GenerateEmitPosition(const Vector2& basePos, const ParticleParam& param) {
	Vector2 spawnPos = basePos;

	switch (param.emitterShape) {
	case EmitterShape::Point:
		// 点発生：emitRange を適用
		if (param.emitRange.x > 0.0f) {
			spawnPos.x += RandomFloat(-param.emitRange.x * 0.5f, param.emitRange.x * 0.5f);
		}
		if (param.emitRange.y > 0.0f) {
			spawnPos.y += RandomFloat(-param.emitRange.y * 0.5f, param.emitRange.y * 0.5f);
		}
		break;

	case EmitterShape::Line:
		// 線発生：X方向にランダム配置
		spawnPos.x += RandomFloat(-param.emitterSize.x * 0.5f, param.emitterSize.x * 0.5f);
		break;

	case EmitterShape::Rectangle:
		// 矩形発生：X, Y 両方向にランダム配置
		spawnPos.x += RandomFloat(-param.emitterSize.x * 0.5f, param.emitterSize.x * 0.5f);
		spawnPos.y += RandomFloat(-param.emitterSize.y * 0.5f, param.emitterSize.y * 0.5f);
		break;
	}

	return spawnPos;
}

// ターゲット指定版 Emit（Homing用）
void ParticleManager::EmitWithTarget(ParticleType type, const Vector2& pos, const Vector2* target) {
	if (params_.find(type) == params_.end()) return;

	const ParticleParam& param = params_[type];
	if (param.textureHandle < 0) return;

	for (int i = 0; i < param.count; ++i) {
		Particle& p = GetNextParticle();
		p.SetType(type);

		int life = static_cast<int>(RandomFloat(static_cast<float>(param.lifeMin), static_cast<float>(param.lifeMax)));

		// Emitter Shape に応じた座標生成
		Vector2 spawnPos = GenerateEmitPosition(pos, param);

		// 速度ベクトル
		float speed = RandomFloat(param.speedMin, param.speedMax);
		float halfRange = param.angleRange / 2.0f;
		float angleDeg = param.angleBase + RandomFloat(-halfRange, halfRange);
		float angleRad = angleDeg * (3.14159265f / 180.0f);
		Vector2 vel = { cosf(angleRad) * speed, sinf(angleRad) * speed };

		Vector2 totalAcc = param.gravity + param.acceleration;
		float rotSpeed = RandomFloat(param.rotationSpeedMin, param.rotationSpeedMax);
		float size = RandomFloat(param.sizeMin, param.sizeMax);

		p.Initialize(
			spawnPos, vel, totalAcc, life,
			param.textureHandle,
			param.scaleStart, param.scaleEnd,
			param.colorStart, param.colorEnd,
			0.0f, rotSpeed,
			param.blendMode,
			size,
			param.useAnimation,
			param.divX, param.divY,
			param.totalFrames, param.animSpeed
		);

		// Homing 設定
		if (param.useHoming && target != nullptr) {
			p.SetBehavior(ParticleBehavior::Homing);
			p.SetHomingTarget(target, param.homingStrength);
		}
		else if (type == ParticleType::Orb) {
			p.SetBehavior(ParticleBehavior::Stationary);
		}
		else {
			p.SetBehavior(ParticleBehavior::Physics);
		}
	}
}

void ParticleManager::EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle) {
	isFlipX; // 未使用警告回避
	if (texHandle < 0) return;

	Particle& p = GetNextParticle();

	// Ghost タイプのパーティクルとして初期化
	p.Initialize(
		pos,                          // 位置
		{ 0.0f, 0.0f },              // 速度なし
		{ 0.0f, 0.0f },              // 加速度なし
		20,                          // 寿命（フレーム）
		texHandle,                   // テクスチャハンドル
		scale,                       // 開始スケール
		scale * 0.8f,                // 終了スケール（少し縮小）
		0x8888FFFF,                  // 開始色（半透明の青）
		0x8888FF00,                  // 終了色（完全に透明）
		rotation,                    // 回転角
		0.0f,                        // 回転速度なし
		kBlendModeNormal,            // 通常ブレンド
		0.0f,                        // 描画サイズ（0 = 画像サイズ）
		false, 1, 1, 1, 0.0f         // アニメーションなし
	);

	p.SetBehavior(ParticleBehavior::Ghost);
	p.SetType(ParticleType::Dust);
}

void ParticleManager::Clear() {
	for (auto& p : particles_) {
		p.Initialize(
			{ 0,0 }, { 0,0 }, { 0,0 }, 0, 0,
			1.0f, 1.0f, WHITE, 0xFFFFFF00,
			0.0f, 0.0f, kBlendModeNone,
			0.0f,                        // 描画サイズ
			false, 1, 1, 1, 0.0f         // アニメーションなし
		);
	}
	nextIndex_ = 0;
}

// =================================
//  連続発生の管理メソッド（既存）
// =================================
void ParticleManager::StartContinuousEmit(ParticleType type, const Vector2& pos) {
	StartContinuousEmitWithTarget(type, pos, nullptr);
}

void ParticleManager::StartContinuousEmitWithTarget(ParticleType type, const Vector2& pos, const Vector2* target) {
	if (params_.find(type) == params_.end()) return;

	ContinuousEmitter& emitter = continuousEmitters_[type];
	emitter.type = type;
	emitter.position = pos;
	emitter.followMode = EmitterFollowMode::None;  // デフォルトは固定
	emitter.followTarget = nullptr;
	emitter.target = target;
	emitter.timer = 0.0f;
	emitter.isActive = true;
}

void ParticleManager::StopContinuousEmit(ParticleType type) {
	if (continuousEmitters_.find(type) != continuousEmitters_.end()) {
		continuousEmitters_[type].isActive = false;
	}
}

void ParticleManager::StopAllContinuousEmit() {
	for (auto& [type, emitter] : continuousEmitters_) {
		emitter.isActive = false;
	}
}

// =================================
//  環境パーティクル専用API
// =================================
void ParticleManager::StartEnvironmentEffect(ParticleType type, EmitterFollowMode mode, const Vector2& basePos) {
	if (params_.find(type) == params_.end()) return;

	// 強制設定：環境パーティクルは必ず連続発生にする
	params_[type].isContinuous = true;
	if (params_[type].emitInterval <= 0.0f) {
		switch (type) {
		case ParticleType::Rain:
			params_[type].emitInterval = 0.1f;
			params_[type].emitterShape = EmitterShape::Line;
			params_[type].emitterSize = { 1280.0f, 0.0f };
			break;
		case ParticleType::Snow:
			params_[type].emitInterval = 0.15f;
			params_[type].emitterShape = EmitterShape::Line;
			params_[type].emitterSize = { 1280.0f, 0.0f };
			break;
		case ParticleType::Orb:
			params_[type].emitInterval = 0.2f;
			params_[type].emitterShape = EmitterShape::Rectangle;
			params_[type].emitterSize = { 1280.0f, 720.0f };
			break;
		default:
			params_[type].emitInterval = 0.1f;
			break;
		}
	}

	ContinuousEmitter& emitter = continuousEmitters_[type];
	emitter.type = type;
	emitter.position = basePos;
	emitter.followMode = mode;
	emitter.followTarget = nullptr;
	emitter.target = nullptr;
	emitter.timer = 0.0f;
	emitter.isActive = true;

#ifdef _DEBUG
	const char* modeName = "Unknown";
	switch (mode) {
	case EmitterFollowMode::None: modeName = "None"; break;
	case EmitterFollowMode::FollowTarget: modeName = "FollowTarget"; break;
	case EmitterFollowMode::WorldPoint: modeName = "WorldPoint"; break;
	}
	Novice::ConsolePrintf("[INFO] StartEnvironmentEffect: Type=%d, Mode=%s, isContinuous=%d, Interval=%.2f, EmitterShape=%d\n",
		static_cast<int>(type), modeName, params_[type].isContinuous, params_[type].emitInterval,
		static_cast<int>(params_[type].emitterShape));
#endif
}

void ParticleManager::StopEnvironmentEffect(ParticleType type) {
	StopContinuousEmit(type);
}

void ParticleManager::UpdateEnvironmentParams(ParticleType type, const ParticleParam& newParams) {
	if (params_.find(type) != params_.end()) {
		params_[type] = newParams;
	}
}

void ParticleManager::SetFollowTarget(ParticleType type, const Vector2* target) {
	if (continuousEmitters_.find(type) != continuousEmitters_.end()) {
		continuousEmitters_[type].followTarget = target;
	}
}

void ParticleManager::UpdateFollowPosition(ParticleType type, const Vector2& newPos) {
	if (continuousEmitters_.find(type) != continuousEmitters_.end()) {
		continuousEmitters_[type].position = newPos;
	}
}

void ParticleManager::SetGroundLevel(float groundY) {
	groundLevel_ = groundY;
}

ParticleParam* ParticleManager::GetParam(ParticleType type) {
	auto it = params_.find(type);
	return (it != params_.end()) ? &it->second : nullptr;
}

const ParticleParam* ParticleManager::GetParam(ParticleType type) const {
	auto it = params_.find(type);
	return (it != params_.end()) ? &it->second : nullptr;
}

void ParticleManager::LoadCommonResources() {
	const auto& allTypes = ParticleRegistry::GetAllParticleTypes();

#ifdef _DEBUG
	Novice::ConsolePrintf("=== LoadCommonResources START ===\n");
#endif

	for (const auto& typeInfo : allTypes) {
		// エントリが存在しない場合は作成
		if (params_.find(typeInfo.type) == params_.end()) {
			params_[typeInfo.type] = GenerateDefaultParam(typeInfo.type);
		}

		// テクスチャを再取得
		int texHandle = TextureManager::GetInstance().GetTexture(typeInfo.defaultTexture);
		params_[typeInfo.type].textureHandle = texHandle;

#ifdef _DEBUG
		Novice::ConsolePrintf("  %s: TextureId=%d -> Handle=%d (Before: %d)\n",
			typeInfo.name.c_str(),
			static_cast<int>(typeInfo.defaultTexture),
			texHandle,
			params_[typeInfo.type].textureHandle);
#endif
	}

#ifdef _DEBUG
	Novice::ConsolePrintf("=== LoadCommonResources END ===\n");
#endif
}

Particle& ParticleManager::GetNextParticle() {
	Particle& p = particles_[nextIndex_];
	nextIndex_ = (nextIndex_ + 1) % kMaxParticles;
	return p;
}

float ParticleManager::RandomFloat(float min, float max) {
	if (min >= max) return min;
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void ParticleManager::DrawDebugWindow() {
#ifdef _DEBUG
	ImGui::Begin("Particle Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// ========== ファイル操作（既存のまま） ==========
	ImGui::Text("=== File Operations ===");
	static char filepath[256] = "Resources/Data/particle_params.json";
	ImGui::InputText("File Path", filepath, sizeof(filepath));

	ImGui::BeginGroup();
	if (ImGui::Button("Save Parameters", ImVec2(140, 30))) {
		if (SaveParamsToJson(filepath)) {
			Novice::ConsolePrintf("Successfully saved to: %s\n", filepath);
		}
		else {
			Novice::ConsolePrintf("Failed to save to: %s\n", filepath);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Parameters", ImVec2(140, 30))) {
		if (LoadParamsFromJson(filepath)) {
			LoadCommonResources();
			Novice::ConsolePrintf("Successfully loaded from: %s\n", filepath);
		}
		else {
			Novice::ConsolePrintf("Failed to load from: %s\n", filepath);
		}
	}
	ImGui::EndGroup();

	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::Text("Save: Export current parameters to JSON file");
		ImGui::Text("Load: Import parameters from JSON file");
		ImGui::EndTooltip();
	}

	ImGui::Separator();

	// ========== ★新規：カテゴリ別タブ表示 ==========
	const auto& allTypes = ParticleRegistry::GetAllParticleTypes();

	// カテゴリごとにグループ化
	std::map<std::string, std::vector<const ParticleTypeInfo*>> categorized;
	for (const auto& typeInfo : allTypes) {
		categorized[typeInfo.category].push_back(&typeInfo);
	}

	if (ImGui::BeginTabBar("ParticleCategories", ImGuiTabBarFlags_None)) {
		for (const auto& [category, types] : categorized) {
			if (ImGui::BeginTabItem(category.c_str())) {
				ImGui::Spacing();

				// ★自動生成：カテゴリ内のパーティクルタイプ選択
				ImGui::Text("Select Particle Type:");

				for (const auto* typeInfo : types) {
					// レジストリの色を使用してボタン色を設定
					ImVec4 color = ImGui::ColorConvertU32ToFloat4(typeInfo->editorColor);
					ImGui::PushStyleColor(ImGuiCol_Button, color);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 1.2f, color.y * 1.2f, color.z * 1.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, 1.0f));

					if (ImGui::Button(typeInfo->name.c_str(), ImVec2(150, 30))) {
						currentDebugType_ = typeInfo->type;
					}

					ImGui::PopStyleColor(3);

					// 4個ずつ横に並べる
					static int count = 0;
					count++;
					if (count % 4 != 0) ImGui::SameLine();
				}

				ImGui::Separator();

				// ★パラメータ編集UI（既存のものを使用）
				if (params_.find(currentDebugType_) != params_.end()) {
					ParticleParam& p = params_[currentDebugType_];

					// 現在選択中のパーティクル名を表示
					const auto* currentInfo = ParticleRegistry::GetParticleType(currentDebugType_);
					if (currentInfo) {
						ImGui::Text("Editing: %s", currentInfo->name.c_str());
						ImGui::Separator();
					}

					// ========== 基本設定（既存のコードをそのまま使用） ==========
					ImGui::Text("=== Basic Settings ===");
					ImGui::SliderInt("Count", &p.count, 1, 50);
					ImGui::SliderInt("Life Min", &p.lifeMin, 1, 300);
					ImGui::SliderInt("Life Max", &p.lifeMax, 1, 300);
					if (p.lifeMin > p.lifeMax) p.lifeMax = p.lifeMin;

					ImGui::Separator();

					// ========== サイズ設定 ==========
					ImGui::Text("=== Size (Pixels) ===");
					ImGui::SliderFloat("Size Min", &p.sizeMin, 4.0f, 256.0f);
					ImGui::SliderFloat("Size Max", &p.sizeMax, 4.0f, 256.0f);
					if (p.sizeMin > p.sizeMax) p.sizeMax = p.sizeMin;

					ImGui::Separator();

					// ========== 物理設定 ==========
					ImGui::Text("=== Physics ===");
					ImGui::SliderFloat("Speed Min", &p.speedMin, 0.0f, 1000.0f);
					ImGui::SliderFloat("Speed Max", &p.speedMax, 0.0f, 1000.0f);
					ImGui::SliderFloat("Base Angle", &p.angleBase, -180.0f, 180.0f);
					ImGui::SliderFloat("Angle Range", &p.angleRange, 0.0f, 360.0f);
					ImGui::DragFloat2("Gravity", &p.gravity.x, 10.0f, -2000.0f, 2000.0f);
					ImGui::DragFloat2("Acceleration", &p.acceleration.x, 10.0f, -2000.0f, 2000.0f);

					ImGui::Separator();

					// ========== 環境パーティクル専用（既存） ==========
					if (currentDebugType_ == ParticleType::Rain ||
						currentDebugType_ == ParticleType::Snow ||
						currentDebugType_ == ParticleType::Orb) {
						ImGui::Text("=== Environment Specific ===");
						if (currentDebugType_ == ParticleType::Rain) {
							ImGui::SliderFloat("Bounce Damping", &p.bounceDamping, 0.0f, 1.0f);
						}
						if (currentDebugType_ == ParticleType::Snow) {
							ImGui::SliderFloat("Wind Strength", &p.windStrength, 0.0f, 100.0f);
						}
						if (currentDebugType_ == ParticleType::Orb) {
							ImGui::SliderFloat("Float Amplitude", &p.floatAmplitude, 0.0f, 100.0f);
							ImGui::SliderFloat("Float Frequency", &p.floatFrequency, 0.1f, 5.0f);
						}
						ImGui::Separator();
					}

					// ========== 見た目設定（既存） ==========
					ImGui::Text("=== Appearance ===");
					ImGui::SliderFloat("Start Scale", &p.scaleStart, 0.1f, 5.0f);
					ImGui::SliderFloat("End Scale", &p.scaleEnd, 0.0f, 5.0f);

					ImGui::Text("Start Color:");
					ColorRGBA startColor = ColorRGBA::FromUInt(p.colorStart);
					float startRGBA[4] = { startColor.r, startColor.g, startColor.b, startColor.a };
					if (ImGui::ColorEdit4("##StartColor", startRGBA)) {
						p.colorStart = ColorRGBA(startRGBA[0], startRGBA[1], startRGBA[2], startRGBA[3]).ToUInt();
					}

					ImGui::Text("End Color:");
					ColorRGBA endColor = ColorRGBA::FromUInt(p.colorEnd);
					float endRGBA[4] = { endColor.r, endColor.g, endColor.b, endColor.a };
					if (ImGui::ColorEdit4("##EndColor", endRGBA)) {
						p.colorEnd = ColorRGBA(endRGBA[0], endRGBA[1], endRGBA[2], endRGBA[3]).ToUInt();
					}

					ImGui::Separator();

					// ========== テスト発射 ==========
					ImGui::Text("=== Test ===");
					if (ImGui::Button("Emit at Center (640, 360)", ImVec2(200, 30))) {
						Emit(currentDebugType_, { 640.0f, 360.0f });
					}

					if (ImGui::Button("Reset to Default", ImVec2(200, 30))) {
						// ★変更：LoadParams() → レジストリから再生成
						p = GenerateDefaultParam(currentDebugType_);
						const auto* info = ParticleRegistry::GetParticleType(currentDebugType_);
						if (info) {
							p.textureHandle = TextureManager::GetInstance().GetTexture(info->defaultTexture);
						}
						Novice::ConsolePrintf("Parameters reset to default values\n");
					}
				}

				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

	// 活性パーティクル数表示
	int aliveCount = 0;
	for (const auto& particle : particles_) {
		if (particle.IsAlive()) aliveCount++;
	}
	ImGui::Separator();
	ImGui::Text("Active Particles: %d / %d", aliveCount, kMaxParticles);

	ImGui::End();
#endif
}

// ============================================
// JSON入出力関連
// ============================================

// ========== JSONシリアライズ ==========
json ParticleManager::SerializeParams() const {
	json root = json::object();

	for (const auto& [type, param] : params_) {
		// レジストリから名前を取得
		const auto* typeInfo = ParticleRegistry::GetParticleType(type);
		if (!typeInfo) {
			// レジストリに登録されていない場合はスキップ
			continue;
		}

		root[typeInfo->name] = SerializeParam(param);
	}

	return root;
}

bool ParticleManager::DeserializeParams(const nlohmann::json& j) {
	try {
		params_.clear();

		// レジストリから全タイプを取得
		const auto& allTypes = ParticleRegistry::GetAllParticleTypes();

		for (const auto& typeInfo : allTypes) {
			if (j.contains(typeInfo.name)) {
				params_[typeInfo.type] = DeserializeParam(j[typeInfo.name], typeInfo.type);
			}
		}

		return true;
	}
	catch (const std::exception& e) {
		e;
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager: Failed to deserialize params: %s\n", e.what());
#endif
		return false;
	}
}

// ========== JSON 保存/読み込み ==========
bool ParticleManager::SaveParamsToJson(const std::string& filepath) {
	try {
		nlohmann::json j = SerializeParams();

		if (JsonUtil::SaveToFile(filepath, j)) {
#ifdef _DEBUG
			Novice::ConsolePrintf("ParticleManager: Parameters saved to %s\n", filepath.c_str());
#endif
			return true;
		}

		return false;
	}
	catch (const std::exception& e) {
		e;
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager: Failed to save params: %s\n", e.what());
#endif
		return false;
	}
}

bool ParticleManager::LoadParamsFromJson(const std::string& filepath) {
	nlohmann::json j;

	if (!JsonUtil::LoadFromFile(filepath, j)) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager: JSON file not found. Creating default parameters...\n");
#endif
		LoadDefaultParamsFromRegistry();
		return SaveParamsToJson(filepath);
	}

	if (DeserializeParams(j)) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager: Parameters loaded from %s\n", filepath.c_str());
#endif
		// Load後にテクスチャを再設定
		LoadCommonResources();
		return true;
	}

#ifdef _DEBUG
	Novice::ConsolePrintf("ParticleManager: Failed to load parameters. Using defaults.\n");
#endif
	LoadDefaultParamsFromRegistry();
	return false;
}

// ==============================================
// レジストリベースの初期化（Phase 3 で有効化）
// ==============================================
void ParticleManager::LoadDefaultParamsFromRegistry() {
	params_.clear();

	const auto& allTypes = ParticleRegistry::GetAllParticleTypes();
	for (const auto& typeInfo : allTypes) {
		ParticleParam defaultParam = GenerateDefaultParam(typeInfo.type);

		// テクスチャをレジストリから取得
		defaultParam.textureHandle = TextureManager::GetInstance().GetTexture(typeInfo.defaultTexture);

		params_[typeInfo.type] = defaultParam;
	}
}

ParticleParam ParticleManager::GenerateDefaultParam(ParticleType type) {
	ParticleParam param;

	// カテゴリごとのデフォルト設定
	const auto* typeInfo = ParticleRegistry::GetParticleType(type);
	if (!typeInfo) return param;

	// カテゴリベースのデフォルト設定
	if (typeInfo->category == "Combat") {
		param.count = 10;
		param.lifeMin = 30;
		param.lifeMax = 60;
		param.speedMin = 100.0f;
		param.speedMax = 300.0f;
		param.angleRange = 360.0f;
		param.blendMode = kBlendModeAdd;
		param.scaleStart = 1.0f;
		param.scaleEnd = 0.3f;
		param.colorStart = 0xFFFFFFFF;
		param.colorEnd = 0xFFFFFF00;
	}
	else if (typeInfo->category == "Environment") {
		param.count = 30;
		param.lifeMin = 120;
		param.lifeMax = 180;
		param.speedMin = 0.0f;
		param.speedMax = 0.0f;
		param.gravity = { 0.0f, -100.0f };
		param.isContinuous = true;
		param.emitInterval = 0.1f;
		param.blendMode = kBlendModeNormal;
		param.scaleStart = 1.0f;
		param.scaleEnd = 1.0f;
	}
	else if (typeInfo->category == "Visual") {
		param.count = 5;
		param.lifeMin = 20;
		param.lifeMax = 40;
		param.speedMin = 50.0f;
		param.speedMax = 150.0f;
		param.blendMode = kBlendModeAdd;
		param.scaleStart = 1.0f;
		param.scaleEnd = 0.0f;
		param.colorStart = 0xFFFFFFFF;
		param.colorEnd = 0xFFFFFF00;
	}
	else { // General
		param.count = 8;
		param.lifeMin = 30;
		param.lifeMax = 60;
		param.speedMin = 50.0f;
		param.speedMax = 100.0f;
		param.blendMode = kBlendModeNormal;
		param.scaleStart = 1.0f;
		param.scaleEnd = 0.5f;
	}

	// タイプ固有の調整（既存の LoadParams から移植）
	switch (type) {
	case ParticleType::Explosion:
		param.count = 1;
		param.lifeMin = 40;
		param.lifeMax = 40;
		param.speedMin = 0.0f;
		param.speedMax = 0.0f;
		param.sizeMin = 64.0f;
		param.sizeMax = 64.0f;
		param.useAnimation = true;
		param.divX = 4;
		param.divY = 1;
		param.totalFrames = 4;
		param.animSpeed = 0.05f;
		break;

	case ParticleType::Rain:
		param.gravity = { 0.0f, -800.0f };
		param.emitterShape = EmitterShape::Line;
		param.emitterSize = { 1280.0f, 0.0f };
		param.sizeMin = 2.0f;
		param.sizeMax = 4.0f;
		param.bounceDamping = 0.3f;
		param.colorStart = 0xAAAAFFFF;
		param.colorEnd = 0xAAAAFF00;
		break;

	case ParticleType::Snow:
		param.gravity = { 0.0f, -50.0f };
		param.emitterShape = EmitterShape::Line;
		param.emitterSize = { 1280.0f, 0.0f };
		param.sizeMin = 4.0f;
		param.sizeMax = 8.0f;
		param.windStrength = 30.0f;
		param.rotationSpeedMin = -0.02f;
		param.rotationSpeedMax = 0.02f;
		param.emitInterval = 0.15f;
		param.colorStart = 0xFFFFFFFF;
		param.colorEnd = 0xFFFFFFFF;
		break;

	case ParticleType::Orb:
		param.count = 5;
		param.lifeMin = 300;
		param.lifeMax = 360;
		param.emitterShape = EmitterShape::Rectangle;
		param.emitterSize = { 1280.0f, 720.0f };
		param.sizeMin = 12.0f;
		param.sizeMax = 24.0f;
		param.floatAmplitude = 30.0f;
		param.floatFrequency = 1.0f;
		param.emitInterval = 0.2f;
		param.colorStart = 0xFFFF88FF;
		param.colorEnd = 0xFFFF8880;
		break;

	case ParticleType::Charge:
		param.useHoming = true;
		param.homingStrength = 500.0f;
		param.emitRange = { 100.0f, 100.0f };
		param.sizeMin = 8.0f;
		param.sizeMax = 16.0f;
		param.colorStart = 0x00FFFFFF;
		param.colorEnd = 0x00FFFF00;
		break;

		// その他のタイプは基本設定を使用
	default:
		break;
	}

	return param;
}

// ==============================================
// JSON処理の簡略化（Phase 4 で SerializeParams を置き換え）
// ==============================================
nlohmann::json ParticleManager::SerializeParam(const ParticleParam& param) const {
	nlohmann::json j;

	j["count"] = param.count;
	j["lifeMin"] = param.lifeMin;
	j["lifeMax"] = param.lifeMax;
	j["speedMin"] = param.speedMin;
	j["speedMax"] = param.speedMax;
	j["angleBase"] = param.angleBase;
	j["angleRange"] = param.angleRange;
	j["gravity"] = { {"x", param.gravity.x}, {"y", param.gravity.y} };
	j["acceleration"] = { {"x", param.acceleration.x}, {"y", param.acceleration.y} };
	j["emitRange"] = { {"x", param.emitRange.x}, {"y", param.emitRange.y} };
	j["sizeMin"] = param.sizeMin;
	j["sizeMax"] = param.sizeMax;
	j["scaleStart"] = param.scaleStart;
	j["scaleEnd"] = param.scaleEnd;
	j["colorStart"] = param.colorStart;
	j["colorEnd"] = param.colorEnd;
	j["rotationSpeedMin"] = param.rotationSpeedMin;
	j["rotationSpeedMax"] = param.rotationSpeedMax;
	j["useAnimation"] = param.useAnimation;
	j["divX"] = param.divX;
	j["divY"] = param.divY;
	j["totalFrames"] = param.totalFrames;
	j["animSpeed"] = param.animSpeed;
	j["blendMode"] = BlendModeToString(param.blendMode);
	j["bounceDamping"] = param.bounceDamping;
	j["windStrength"] = param.windStrength;
	j["floatAmplitude"] = param.floatAmplitude;
	j["floatFrequency"] = param.floatFrequency;

	return j;
}

ParticleParam ParticleManager::DeserializeParam(const nlohmann::json& j, ParticleType type) {
	ParticleParam param;

	param.count = JsonUtil::GetValue<int>(j, "count", 1);
	param.lifeMin = JsonUtil::GetValue<int>(j, "lifeMin", 30);
	param.lifeMax = JsonUtil::GetValue<int>(j, "lifeMax", 60);
	param.speedMin = JsonUtil::GetValue<float>(j, "speedMin", 100.0f);
	param.speedMax = JsonUtil::GetValue<float>(j, "speedMax", 200.0f);
	param.angleBase = JsonUtil::GetValue<float>(j, "angleBase", 0.0f);
	param.angleRange = JsonUtil::GetValue<float>(j, "angleRange", 360.0f);

	if (j.contains("gravity")) {
		param.gravity.x = JsonUtil::GetValue<float>(j["gravity"], "x", 0.0f);
		param.gravity.y = JsonUtil::GetValue<float>(j["gravity"], "y", 0.0f);
	}

	if (j.contains("acceleration")) {
		param.acceleration.x = JsonUtil::GetValue<float>(j["acceleration"], "x", 0.0f);
		param.acceleration.y = JsonUtil::GetValue<float>(j["acceleration"], "y", 0.0f);
	}

	if (j.contains("emitRange")) {
		param.emitRange.x = JsonUtil::GetValue<float>(j["emitRange"], "x", 0.0f);
		param.emitRange.y = JsonUtil::GetValue<float>(j["emitRange"], "y", 0.0f);
	}

	param.sizeMin = JsonUtil::GetValue<float>(j, "sizeMin", 16.0f);
	param.sizeMax = JsonUtil::GetValue<float>(j, "sizeMax", 32.0f);
	param.scaleStart = JsonUtil::GetValue<float>(j, "scaleStart", 1.0f);
	param.scaleEnd = JsonUtil::GetValue<float>(j, "scaleEnd", 0.0f);
	param.colorStart = JsonUtil::GetValue<unsigned int>(j, "colorStart", 0xFFFFFFFF);
	param.colorEnd = JsonUtil::GetValue<unsigned int>(j, "colorEnd", 0xFFFFFF00);
	param.rotationSpeedMin = JsonUtil::GetValue<float>(j, "rotationSpeedMin", 0.0f);
	param.rotationSpeedMax = JsonUtil::GetValue<float>(j, "rotationSpeedMax", 0.0f);
	param.useAnimation = JsonUtil::GetValue<bool>(j, "useAnimation", false);
	param.divX = JsonUtil::GetValue<int>(j, "divX", 1);
	param.divY = JsonUtil::GetValue<int>(j, "divY", 1);
	param.totalFrames = JsonUtil::GetValue<int>(j, "totalFrames", 1);
	param.animSpeed = JsonUtil::GetValue<float>(j, "animSpeed", 0.1f);

	std::string blendModeStr = JsonUtil::GetValue<std::string>(j, "blendMode", "Normal");
	param.blendMode = StringToBlendMode(blendModeStr);

	if (j.contains("emitterShape")) {
		std::string shapeStr = JsonUtil::GetValue<std::string>(j, "emitterShape", "Point");
		if (shapeStr == "Line") {
			param.emitterShape = EmitterShape::Line;
		}
		else if (shapeStr == "Rectangle") {
			param.emitterShape = EmitterShape::Rectangle;
		}
		else {
			param.emitterShape = EmitterShape::Point;
		}
	}

	if (j.contains("emitterSize")) {
		param.emitterSize.x = JsonUtil::GetValue<float>(j["emitterSize"], "x", 0.0f);
		param.emitterSize.y = JsonUtil::GetValue<float>(j["emitterSize"], "y", 0.0f);
	}

	param.useHoming = JsonUtil::GetValue<bool>(j, "useHoming", false);
	param.homingStrength = JsonUtil::GetValue<float>(j, "homingStrength", 0.0f);
	param.isContinuous = JsonUtil::GetValue<bool>(j, "isContinuous", false);
	param.emitInterval = JsonUtil::GetValue<float>(j, "emitInterval", 0.0f);
	param.bounceDamping = JsonUtil::GetValue<float>(j, "bounceDamping", 0.3f);
	param.windStrength = JsonUtil::GetValue<float>(j, "windStrength", 0.0f);
	param.floatAmplitude = JsonUtil::GetValue<float>(j, "floatAmplitude", 0.0f);
	param.floatFrequency = JsonUtil::GetValue<float>(j, "floatFrequency", 1.0f);

	// テクスチャハンドルが-1の場合、レジストリから取得
	const auto* typeInfo = ParticleRegistry::GetParticleType(type);
	if (typeInfo) {
		param.textureHandle = TextureManager::GetInstance().GetTexture(typeInfo->defaultTexture);
	}
	else {
		param.textureHandle = -1;
#ifdef _DEBUG
		Novice::ConsolePrintf("  DeserializeParam: Type not found in registry!\n");
#endif
	}

	return param;
}