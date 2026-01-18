#include "ParticleManager.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "JsonUtil.h"
#include "json.hpp"
#include "Camera2D.h"
#include "Effect.h"

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
	LoadCommonResources();  // 先にテクスチャをロード

	// JSONからパラメータを読み込み（ファイルがなければデフォルトで作成）
	if (!LoadParamsFromJson(kDefaultParamPath)) {
		// 読み込み失敗時はデフォルトパラメータを設定して保存
		LoadParams();
		SaveParamsToJson(kDefaultParamPath);
	}
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

void ParticleManager::LoadParams() {
	// 1. 爆発（加算ブレンドで明るく光る）
	ParticleParam explosion;
	explosion.count = 1;
	explosion.lifeMin = 40;
	explosion.lifeMax = 40;
	explosion.speedMin = 0.0f;
	explosion.speedMax = 0.0f;
	explosion.angleBase = 0.0f;
	explosion.angleRange = 0.0f;
	explosion.gravity = { 0.0f, 0.0f };
	explosion.acceleration = { 0.0f, 0.0f };
	explosion.emitRange = { 0.0f, 0.0f };
	explosion.sizeMin = 64.0f;
	explosion.sizeMax = 64.0f;
	explosion.scaleStart = 1.0f;
	explosion.scaleEnd = 1.0f;
	explosion.colorStart = 0xFFFFFFFF;
	explosion.colorEnd = 0xFFFFFFFF;
	explosion.rotationSpeedMin = 0.0f;
	explosion.rotationSpeedMax = 0.0f;
	explosion.useAnimation = true;
	explosion.divX = 4;
	explosion.divY = 1;
	explosion.totalFrames = 4;
	explosion.animSpeed = 0.05f;
	explosion.blendMode = kBlendModeAdd;
	params_[ParticleType::Explosion] = explosion;

	// 2. デブリ（通常ブレンド）
	ParticleParam debris;
	debris.count = 10;
	debris.lifeMin = 40;
	debris.lifeMax = 60;
	debris.speedMin = 150.0f;
	debris.speedMax = 300.0f;
	debris.angleBase = 0.0f;
	debris.angleRange = 360.0f;
	debris.gravity = { 0.0f, -800.0f };
	debris.acceleration = { 0.0f, 0.0f };
	debris.emitRange = { 20.0f, 20.0f };      //少し散らす
	debris.sizeMin = 12.0f;
	debris.sizeMax = 24.0f;
	debris.scaleStart = 1.0f;
	debris.scaleEnd = 0.3f;
	debris.colorStart = 0xFFFFFFFF;
	debris.colorEnd = 0xFFFFFF00;
	debris.rotationSpeedMin = -0.1f;          // 回転しながら落ちる
	debris.rotationSpeedMax = 0.1f;
	debris.useAnimation = false;
	debris.divX = 1;
	debris.divY = 1;
	debris.totalFrames = 1;
	debris.animSpeed = 0.0f;
	debris.blendMode = kBlendModeNormal;
	params_[ParticleType::Debris] = debris;

	// 3. ヒットエフェクト（加算ブレンド）
	ParticleParam hit;
	hit.count = 40;
	hit.lifeMin = 20;
	hit.lifeMax = 55;
	hit.speedMin = 1225.0f;
	hit.speedMax = 1500.0f;
	hit.angleBase = 90.0f;
	hit.angleRange = 1550.0f;
	hit.gravity = { 0.0f, -300.0f };
	hit.acceleration = { 0.0f, 0.0f };
	hit.emitRange = { 10.0f, 10.0f };
	hit.sizeMin = 27.0f;
	hit.sizeMax = 43.0f;
	hit.scaleStart = 0.8f;
	hit.scaleEnd = 0.2f;
	hit.colorStart = 0xFFFFFFFF;
	hit.colorEnd = 0xFFFFFF00;
	hit.rotationSpeedMin = 0.0f;
	hit.rotationSpeedMax = 0.0f;
	hit.useAnimation = false;
	hit.divX = 1;
	hit.divY = 1;
	hit.totalFrames = 1;
	hit.animSpeed = 0.0f;
	hit.blendMode = kBlendModeAdd;
	params_[ParticleType::Hit] = hit;

	// 4. 土煙（通常ブレンド）
	ParticleParam dust;
	dust.count = 8;
	dust.lifeMin = 30;
	dust.lifeMax = 45;
	dust.speedMin = 50.0f;
	dust.speedMax = 100.0f;
	dust.angleBase = -90.0f;
	dust.angleRange = 60.0f;
	dust.gravity = { 0.0f, -50.0f };
	dust.acceleration = { 0.0f, 0.0f };
	dust.emitRange = { 30.0f, 10.0f };
	dust.sizeMin = 32.0f;
	dust.sizeMax = 64.0f;
	dust.scaleStart = 0.5f;
	dust.scaleEnd = 1.5f;
	dust.colorStart = 0xAAAAAAAA;
	dust.colorEnd = 0xAAAAAA00;
	dust.rotationSpeedMin = -0.05f;
	dust.rotationSpeedMax = 0.05f;
	dust.useAnimation = false;
	dust.divX = 1;
	dust.divY = 1;
	dust.totalFrames = 1;
	dust.animSpeed = 0.0f;
	dust.blendMode = kBlendModeNormal;
	params_[ParticleType::Dust] = dust;

	// 5. マズルフラッシュ（加算ブレンド）
	ParticleParam muzzle;
	muzzle.count = 1;
	muzzle.lifeMin = 8;
	muzzle.lifeMax = 12;
	muzzle.speedMin = 0.0f;
	muzzle.speedMax = 0.0f;
	muzzle.angleBase = 0.0f;
	muzzle.angleRange = 0.0f;
	muzzle.gravity = { 0.0f, 0.0f };
	muzzle.acceleration = { 0.0f, 0.0f };
	muzzle.emitRange = { 0.0f, 0.0f };
	muzzle.sizeMin = 32.0f;
	muzzle.sizeMax = 48.0f;
	muzzle.scaleStart = 1.2f;
	muzzle.scaleEnd = 0.3f;
	muzzle.colorStart = 0xFFFFFFFF;
	muzzle.colorEnd = 0xFFFFFF00;
	muzzle.rotationSpeedMin = 0.0f;
	muzzle.rotationSpeedMax = 0.0f;
	muzzle.useAnimation = false;
	muzzle.divX = 1;
	muzzle.divY = 1;
	muzzle.totalFrames = 1;
	muzzle.animSpeed = 0.0f;
	muzzle.blendMode = kBlendModeAdd;
	params_[ParticleType::MuzzleFlash] = muzzle;

	// 6. 雨（連続発生 - 改良版）
	ParticleParam rain;
	rain.count = 30;                           // 1回の発生数
	rain.lifeMin = 120;
	rain.lifeMax = 180;
	rain.speedMin = 0.0f;
	rain.speedMax = 0.0f;
	rain.angleBase = -90.0f;                   // 下向き（Y+が上なので-90度）
	rain.angleRange = 0.0f;
	rain.gravity = { 0.0f, -800.0f };          // 重力で高速落下
	rain.acceleration = { 0.0f, 0.0f };
	rain.emitterShape = EmitterShape::Line;
	rain.emitterSize = { 1280.0f, 0.0f };      // 画面幅
	rain.sizeMin = 2.0f;
	rain.sizeMax = 4.0f;
	rain.scaleStart = 1.0f;
	rain.scaleEnd = 1.0f;
	rain.colorStart = 0xAAAAFFFF;              // 薄い青
	rain.colorEnd = 0xAAAAFF00;                // フェードアウト
	rain.rotationSpeedMin = 0.0f;
	rain.rotationSpeedMax = 0.0f;
	rain.useAnimation = false;
	rain.blendMode = kBlendModeNormal;
	rain.isContinuous = true;
	rain.emitInterval = 0.1f;                  // 0.1秒ごとに発生
	rain.bounceDamping = 0.3f;                 // 跳ね返り係数
	params_[ParticleType::Rain] = rain;

	// 7. 雪（連続発生 - 改良版）
	ParticleParam snow;
	snow.count = 50;
	snow.lifeMin = 180;
	snow.lifeMax = 240;
	snow.speedMin = 0.0f;
	snow.speedMax = 0.0f;
	snow.angleBase = -90.0f;                   // 下向き
	snow.angleRange = 0.0f;
	snow.gravity = { 0.0f, -50.0f };           // ゆっくり落下
	snow.acceleration = { 0.0f, 0.0f };
	snow.emitterShape = EmitterShape::Line;
	snow.emitterSize = { 1280.0f, 0.0f };
	snow.sizeMin = 4.0f;
	snow.sizeMax = 8.0f;
	snow.scaleStart = 1.0f;
	snow.scaleEnd = 1.0f;
	snow.colorStart = 0xFFFFFFFF;              // 白
	snow.colorEnd = 0xFFFFFFFF;
	snow.rotationSpeedMin = -0.02f;
	snow.rotationSpeedMax = 0.02f;
	snow.useAnimation = false;
	snow.blendMode = kBlendModeNormal;
	snow.isContinuous = true;
	snow.emitInterval = 0.15f;
	snow.windStrength = 30.0f;                 // ★横風の強さ
	params_[ParticleType::Snow] = snow;

	// 8. オーブ（ふわふわ浮遊 - 改良版）
	ParticleParam orb;
	orb.count = 5;                             // 少なめに変更
	orb.lifeMin = 300;                         // 長寿命
	orb.lifeMax = 360;
	orb.speedMin = 0.0f;
	orb.speedMax = 0.0f;
	orb.angleBase = 0.0f;
	orb.angleRange = 0.0f;
	orb.gravity = { 0.0f, 0.0f };              // 重力なし
	orb.acceleration = { 0.0f, 0.0f };
	orb.emitterShape = EmitterShape::Rectangle;
	orb.emitterSize = { 1280.0f, 720.0f };     // 画面全体
	orb.sizeMin = 12.0f;
	orb.sizeMax = 24.0f;
	orb.scaleStart = 0.8f;
	orb.scaleEnd = 1.2f;
	orb.colorStart = 0xFFFF88FF;               // 黄色（不透明）
	orb.colorEnd = 0xFFFF8880;                 // やや透明化
	orb.rotationSpeedMin = -0.05f;
	orb.rotationSpeedMax = 0.05f;
	orb.useAnimation = false;
	orb.blendMode = kBlendModeAdd;
	orb.isContinuous = true;
	orb.emitInterval = 0.2f;
	orb.floatAmplitude = 30.0f;                // ★浮遊の振幅
	orb.floatFrequency = 1.0f;                 // ★浮遊の周波数
	params_[ParticleType::Orb] = orb;

	// 9. チャージ（Homing）
	ParticleParam charge;
	charge.count = 20;
	charge.lifeMin = 30;
	charge.lifeMax = 60;
	charge.speedMin = 50.0f;
	charge.speedMax = 100.0f;
	charge.angleBase = 0.0f;
	charge.angleRange = 360.0f;
	charge.gravity = { 0.0f, 0.0f };
	charge.acceleration = { 0.0f, 0.0f };
	charge.emitRange = { 100.0f, 100.0f };
	charge.sizeMin = 8.0f;
	charge.sizeMax = 16.0f;
	charge.scaleStart = 1.0f;
	charge.scaleEnd = 0.5f;
	charge.colorStart = 0x00FFFFFF;  // 青
	charge.colorEnd = 0x00FFFF00;
	charge.rotationSpeedMin = 0.0f;
	charge.rotationSpeedMax = 0.0f;
	charge.useAnimation = false;
	charge.blendMode = kBlendModeAdd;
	charge.useHoming = true;           // Homing 有効
	charge.homingStrength = 500.0f;    // 追従の強さ
	params_[ParticleType::Charge] = charge;

	// 10. Glow (汎用の光・オーラ)
	ParticleParam glow;
	glow.textureHandle = texGlow_; // ※LoadCommonResourcesで上書きされますが念のため
	glow.count = 1;
	glow.lifeMin = 20; glow.lifeMax = 40;
	glow.scaleStart = 1.0f; glow.scaleEnd = 0.0f;
	glow.colorStart = 0xFFFFFFFF;
	glow.colorEnd = 0xFFFFFF00;
	glow.speedMin = 0.0f; glow.speedMax = 20.0f;
	glow.blendMode = kBlendModeAdd;
	params_[ParticleType::Glow] = glow;

	// 11. Shockwave (衝撃波リング)
	ParticleParam ring;
	ring.textureHandle = texRing_;
	ring.count = 1;
	ring.lifeMin = 15; ring.lifeMax = 20;
	ring.scaleStart = 0.5f; ring.scaleEnd = 2.5f; // 急激に広がる
	ring.colorStart = 0xFFFFFFCC;
	ring.colorEnd = 0xFFFFFF00;
	ring.speedMin = 0.0f; ring.speedMax = 0.0f;
	ring.blendMode = kBlendModeAdd;
	params_[ParticleType::Shockwave] = ring;

	// 12. Sparkle (キラキラ)
	ParticleParam spark;
	spark.textureHandle = texSparkle_;
	spark.count = 3;
	spark.lifeMin = 30; spark.lifeMax = 50;
	spark.scaleStart = 0.8f; spark.scaleEnd = 0.0f;
	spark.colorStart = 0xFFFF80FF;
	spark.colorEnd = 0xFFFF8000;
	spark.speedMin = 100.0f; spark.speedMax = 200.0f;
	spark.angleRange = 360.0f;
	spark.gravity = { 0.0f, -400.0f };
	spark.blendMode = kBlendModeAdd;
	params_[ParticleType::Sparkle] = spark;

	// 13. Slash (斬撃・軌跡)
	ParticleParam slash;
	slash.textureHandle = texScratch_;
	slash.count = 1;
	slash.lifeMin = 10; slash.lifeMax = 15;
	slash.scaleStart = 1.5f; slash.scaleEnd = 0.5f;
	slash.colorStart = 0xFFFFFFFF;
	slash.colorEnd = 0xFFFFFF00;
	slash.speedMin = 0.0f; slash.speedMax = 0.0f;
	slash.blendMode = kBlendModeAdd;
	params_[ParticleType::Slash] = slash;

	// 14. SmokeCloud (もくもく煙)
	ParticleParam smoke;
	smoke.textureHandle = texSmoke_;
	smoke.count = 2;
	smoke.lifeMin = 60; smoke.lifeMax = 90;
	smoke.scaleStart = 0.5f; smoke.scaleEnd = 1.5f;
	smoke.colorStart = 0x808080DD; // 灰色
	smoke.colorEnd = 0x00000000;
	smoke.speedMin = 20.0f; smoke.speedMax = 50.0f;
	smoke.gravity = { 0.0f, 100.0f }; // 上へ昇る
	smoke.blendMode = kBlendModeNormal;
	params_[ParticleType::SmokeCloud] = smoke;

	// 15. デジタルスパーク（サイバー空間用）
	ParticleParam digitalSpark;
	digitalSpark.count = 30;
	digitalSpark.lifeMin = 20;
	digitalSpark.lifeMax = 40;
	digitalSpark.speedMin = 200.0f;
	digitalSpark.speedMax = 500.0f;
	digitalSpark.angleBase = 0.0f;
	digitalSpark.angleRange = 360.0f;
	digitalSpark.gravity = { 0.0f, 0.0f }; // 無重力
	digitalSpark.sizeMin = 50.0f;
	digitalSpark.sizeMax = 70.0f;
	digitalSpark.scaleStart = 1.0f;
	digitalSpark.scaleEnd = 0.0f;
	digitalSpark.colorStart = 0x00FFFFDD; // シアン
	digitalSpark.colorEnd = 0x0000FF00;   // 青に消える
	digitalSpark.useAnimation = false;
	digitalSpark.textureHandle = -1;      // 後でロード
	params_[ParticleType::DigitalSpark] = digitalSpark;
}


void ParticleManager::Update(float deltaTime) {
	// 連続発生の処理（追従モード対応）
	for (auto& [type, emitter] : continuousEmitters_) {
		if (!emitter.isActive) continue;
		if (params_.find(type) == params_.end()) continue;

		const ParticleParam& param = params_[type];
		if (!param.isContinuous) continue;

		// タイマー更新
		emitter.timer += deltaTime;

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
					float windOffset = sinf(p.GetPosition().y * 0.01f) * param.windStrength * deltaTime;
					Vector2 currentPos = p.GetPosition();
					currentPos.x += windOffset;
					// 注意：Particleクラスに SetPosition を追加する必要があります
				}
			}

			// 通常の更新処理
			p.Update(deltaTime);

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
	// テクスチャを一括ロード
	texExplosion_ = Novice::LoadTexture("./Resources/images/effect/explosion.png");
	texDebris_ = Novice::LoadTexture("./Resources/images/effect/debris.png");
	texHit_ = Novice::LoadTexture("./Resources/images/effect/star.png");
	texDust_ = Novice::LoadTexture("./Resources/images/effect/star.png");
	texRain_ = Novice::LoadTexture("./Resources/images/effect/rain.png");
	texSnow_ = Novice::LoadTexture("./Resources/images/effect/snow.png");
	texOrb_ = Novice::LoadTexture("./Resources/images/effect/orb.png");
	texGlow_ = Novice::LoadTexture("./Resources/images/effect/particle_output/particle_glow.png");
	texRing_ = Novice::LoadTexture("./Resources/images/effect/particle_output/particle_ring.png");
	texSparkle_ = Novice::LoadTexture("./Resources/images/effect/particle_output/particle_sparkle.png");
	texScratch_ = Novice::LoadTexture("./Resources/images/effect/particle_output/particle_scratch.png");
	texSmoke_ = Novice::LoadTexture("./Resources/images/effect/particle_output/particle_smoke.png");


#ifdef _DEBUG
	Novice::ConsolePrintf("ParticleManager::LoadCommonResources\n");
	Novice::ConsolePrintf("  Explosion: %d\n", texExplosion_);
	Novice::ConsolePrintf("  Debris: %d\n", texDebris_);
	Novice::ConsolePrintf("  Hit: %d\n", texHit_);
	Novice::ConsolePrintf("  Dust: %d\n", texDust_);
#endif

	// パラメータにハンドルをセット
	params_[ParticleType::Explosion].textureHandle = texExplosion_;
	params_[ParticleType::Debris].textureHandle = texDebris_;
	params_[ParticleType::Hit].textureHandle = texHit_;
	params_[ParticleType::Dust].textureHandle = texDust_;
	params_[ParticleType::MuzzleFlash].textureHandle = texHit_;
	params_[ParticleType::Rain].textureHandle = texRain_;
	params_[ParticleType::Snow].textureHandle = texSnow_;
	params_[ParticleType::Orb].textureHandle = texOrb_;
	params_[ParticleType::Charge].textureHandle = texGlow_;
	params_[ParticleType::Glow].textureHandle = texGlow_;
	params_[ParticleType::Shockwave].textureHandle = texRing_;
	params_[ParticleType::Sparkle].textureHandle = texSparkle_;
	params_[ParticleType::Slash].textureHandle = texScratch_;
	params_[ParticleType::SmokeCloud].textureHandle = texSmoke_;
	params_[ParticleType::DigitalSpark].textureHandle = texHit_;
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

	// ========== ファイル操作 ==========
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

	// ========== 環境パーティクルコントロール ==========
	if (ImGui::CollapsingHeader("Environment Effects Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent();

		// 雨のコントロール
		ImGui::Text("Rain:");
		ImGui::SameLine(100);
		static bool rainActive = false;
		if (ImGui::Button(rainActive ? "Stop##Rain" : "Start##Rain", ImVec2(80, 0))) {
			if (rainActive) {
				StopEnvironmentEffect(ParticleType::Rain);
			}
			else {
				StartEnvironmentEffect(ParticleType::Rain, EmitterFollowMode::FollowTarget);
			}
			rainActive = !rainActive;
		}

		// 雪のコントロール
		ImGui::Text("Snow:");
		ImGui::SameLine(100);
		static bool snowActive = false;
		if (ImGui::Button(snowActive ? "Stop##Snow" : "Start##Snow", ImVec2(80, 0))) {
			if (snowActive) {
				StopEnvironmentEffect(ParticleType::Snow);
			}
			else {
				StartEnvironmentEffect(ParticleType::Snow, EmitterFollowMode::FollowTarget);
			}
			snowActive = !snowActive;
		}

		// オーブのコントロール
		ImGui::Text("Orb:");
		ImGui::SameLine(100);
		static bool orbActive = false;
		if (ImGui::Button(orbActive ? "Stop##Orb" : "Start##Orb", ImVec2(80, 0))) {
			if (orbActive) {
				StopEnvironmentEffect(ParticleType::Orb);
			}
			else {
				StartEnvironmentEffect(ParticleType::Orb, EmitterFollowMode::FollowTarget);
			}
			orbActive = !orbActive;
		}

		ImGui::Unindent();
	}

	ImGui::Separator();

	// ========== エフェクトタイプ選択 ==========
	ImGui::Text("=== Effect Type ===");
	const char* items[] = { "Explosion", "Debris", "Hit", "Dust", "MuzzleFlash","Rain","Snow","Orb","Charge","Glow","Shockwave","Sparkle","Slash","SmokeCloud" };
	int currentItem = static_cast<int>(currentDebugType_);
	if (ImGui::Combo("Type", &currentItem, items, IM_ARRAYSIZE(items))) {
		currentDebugType_ = static_cast<ParticleType>(currentItem);
	}

	ParticleParam& p = params_[currentDebugType_];

	ImGui::Separator();

	// ========== 基本設定 ==========
	ImGui::Text("=== Basic Settings ===");

	ImGui::SliderInt("Count", &p.count, 1, 50);
	ImGui::SameLine();
	if (ImGui::Button("-##Count")) p.count = std::max(1, p.count - 1);
	ImGui::SameLine();
	if (ImGui::Button("+##Count")) p.count = std::min(50, p.count + 1);

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
	if (p.speedMin > p.speedMax) p.speedMax = p.speedMin;

	ImGui::SliderFloat("Base Angle", &p.angleBase, -180.0f, 180.0f);
	ImGui::SameLine();
	if (ImGui::Button("↑##Up")) p.angleBase = -90.0f;
	ImGui::SameLine();
	if (ImGui::Button("→##Right")) p.angleBase = 0.0f;
	ImGui::SameLine();
	if (ImGui::Button("↓##Down")) p.angleBase = 90.0f;
	ImGui::SameLine();
	if (ImGui::Button("←##Left")) p.angleBase = 180.0f;

	ImGui::SliderFloat("Angle Range", &p.angleRange, 0.0f, 360.0f);
	ImGui::SameLine();
	if (ImGui::Button("360°##Full")) p.angleRange = 360.0f;

	ImGui::DragFloat2("Gravity", &p.gravity.x, 10.0f, -2000.0f, 2000.0f);
	ImGui::DragFloat2("Acceleration", &p.acceleration.x, 10.0f, -2000.0f, 2000.0f);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::Text("Continuous force applied every frame (e.g., wind)");
		ImGui::EndTooltip();
	}

	ImGui::Separator();

	// ========== 環境パーティクル専用パラメータ ==========
	if (currentDebugType_ == ParticleType::Rain ||
		currentDebugType_ == ParticleType::Snow ||
		currentDebugType_ == ParticleType::Orb) {
		ImGui::Text("=== Environment Specific ===");

		if (currentDebugType_ == ParticleType::Rain) {
			ImGui::SliderFloat("Bounce Damping", &p.bounceDamping, 0.0f, 1.0f);
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Ground bounce coefficient (0 = no bounce, 1 = full bounce)");
			}
		}

		if (currentDebugType_ == ParticleType::Snow) {
			ImGui::SliderFloat("Wind Strength", &p.windStrength, 0.0f, 100.0f);
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Horizontal wind swaying effect");
			}
		}

		if (currentDebugType_ == ParticleType::Orb) {
			ImGui::SliderFloat("Float Amplitude", &p.floatAmplitude, 0.0f, 100.0f);
			ImGui::SliderFloat("Float Frequency", &p.floatFrequency, 0.1f, 5.0f);
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Floating motion parameters (sine wave)");
			}
		}

		ImGui::Separator();
	}

	// ========== 生成範囲設定 ==========
	ImGui::Text("=== Emit Range ===");
	ImGui::DragFloat2("Range (X, Y)", &p.emitRange.x, 1.0f, 0.0f, 500.0f);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::Text("Random spawn position offset");
		ImGui::Text("X: Horizontal spread, Y: Vertical spread");
		ImGui::EndTooltip();
	}

	ImGui::Separator();

	// ========== 回転設定 ==========
	ImGui::Text("=== Rotation ===");
	ImGui::SliderFloat("Rot Speed Min", &p.rotationSpeedMin, -1.0f, 1.0f, "%.3f rad/frame");
	ImGui::SliderFloat("Rot Speed Max", &p.rotationSpeedMax, -1.0f, 1.0f, "%.3f rad/frame");
	if (p.rotationSpeedMin > p.rotationSpeedMax) {
		p.rotationSpeedMax = p.rotationSpeedMin;
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::Text("Rotation speed in radians per frame");
		ImGui::Text("Positive: Clockwise, Negative: Counter-clockwise");
		ImGui::EndTooltip();
	}

	ImGui::Separator();

	// ========== ブレンドモード設定 ==========
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

		int currentBlendMode = static_cast<int>(p.blendMode);
		if (ImGui::Combo("Blend Mode", &currentBlendMode, blendModes, IM_ARRAYSIZE(blendModes))) {
			p.blendMode = static_cast<BlendMode>(currentBlendMode);
		}

		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Normal: Standard alpha blending");
			ImGui::Text("Add: Bright, glowing effect (recommended for fire, explosions)");
			ImGui::Text("Subtract: Dark, shadow effect");
			ImGui::Text("Multiply: Darker blending");
			ImGui::Text("Screen: Bright, soft blending");
			ImGui::EndTooltip();
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	// ========== 見た目設定 ==========
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

	// ========== アニメーション設定 ==========
	ImGui::Text("=== Animation (Explosion Only) ===");
	ImGui::Checkbox("Use Animation", &p.useAnimation);

	if (p.useAnimation) {
		ImGui::Indent();
		ImGui::SliderInt("Div X", &p.divX, 1, 10);
		ImGui::SliderInt("Div Y", &p.divY, 1, 10);
		ImGui::SliderInt("Total Frames", &p.totalFrames, 1, 100);
		ImGui::SliderFloat("Anim Speed", &p.animSpeed, 0.01f, 0.5f);
		ImGui::Unindent();
	}

	ImGui::Separator();

	// ========== テスト発射 ==========
	ImGui::Text("=== Test ===");
	if (ImGui::Button("Emit at Center (640, 360)", ImVec2(200, 30))) {
		Emit(currentDebugType_, { 640.0f, 360.0f });
	}

	if (ImGui::Button("Reset to Default", ImVec2(200, 30))) {
		LoadParams();
		Novice::ConsolePrintf("Parameters reset to default values\n");
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
		std::string typeName;
		switch (type) {
		case ParticleType::Explosion: typeName = "Explosion"; break;
		case ParticleType::Debris: typeName = "Debris"; break;
		case ParticleType::Hit: typeName = "Hit"; break;
		case ParticleType::Dust: typeName = "Dust"; break;
		case ParticleType::MuzzleFlash: typeName = "MuzzleFlash"; break;
		case ParticleType::Rain: typeName = "Rain"; break;
		case ParticleType::Snow: typeName = "Snow"; break;
		case ParticleType::Orb: typeName = "Orb"; break;
		case ParticleType::Charge: typeName = "Charge"; break;
		case ParticleType::Glow: typeName = "Glow"; break;
		case ParticleType::Shockwave: typeName = "Shockwave"; break;
		case ParticleType::Sparkle: typeName = "Sparkle"; break;
		case ParticleType::Slash: typeName = "Slash"; break;
		case ParticleType::SmokeCloud: typeName = "SmokeCloud"; break;
		default: typeName = "Unknown"; break;
		}

		nlohmann::json paramJson;
		paramJson["count"] = param.count;
		paramJson["textureHandle"] = param.textureHandle;
		paramJson["lifeMin"] = param.lifeMin;
		paramJson["lifeMax"] = param.lifeMax;
		paramJson["speedMin"] = param.speedMin;
		paramJson["speedMax"] = param.speedMax;
		paramJson["angleBase"] = param.angleBase;
		paramJson["angleRange"] = param.angleRange;
		paramJson["gravity"] = {
			{"x", param.gravity.x},
			{"y", param.gravity.y}
		};
		paramJson["acceleration"] = {
			{"x", param.acceleration.x},
			{"y", param.acceleration.y}
		};
		paramJson["emitRange"] = {
			{"x", param.emitRange.x},
			{"y", param.emitRange.y}
		};
		paramJson["sizeMin"] = param.sizeMin;
		paramJson["sizeMax"] = param.sizeMax;
		paramJson["scaleStart"] = param.scaleStart;
		paramJson["scaleEnd"] = param.scaleEnd;
		paramJson["colorStart"] = param.colorStart;
		paramJson["colorEnd"] = param.colorEnd;
		paramJson["rotationSpeedMin"] = param.rotationSpeedMin;
		paramJson["rotationSpeedMax"] = param.rotationSpeedMax;
		paramJson["useAnimation"] = param.useAnimation;
		paramJson["divX"] = param.divX;
		paramJson["divY"] = param.divY;
		paramJson["totalFrames"] = param.totalFrames;
		paramJson["animSpeed"] = param.animSpeed;
		paramJson["blendMode"] = BlendModeToString(param.blendMode);

		// ★環境パーティクル専用パラメータ
		paramJson["bounceDamping"] = param.bounceDamping;
		paramJson["windStrength"] = param.windStrength;
		paramJson["floatAmplitude"] = param.floatAmplitude;
		paramJson["floatFrequency"] = param.floatFrequency;

		root[typeName] = paramJson;
	}

	return root;
}

bool ParticleManager::DeserializeParams(const nlohmann::json& j) {
	try {
		params_.clear();

		std::map<std::string, ParticleType> typeMap = {
			{"Explosion", ParticleType::Explosion},
			{"Debris", ParticleType::Debris},
			{"Hit", ParticleType::Hit},
			{"Dust", ParticleType::Dust},
			{"MuzzleFlash", ParticleType::MuzzleFlash},
			{"Rain", ParticleType::Rain},
			{"Snow", ParticleType::Snow},
			{"Orb", ParticleType::Orb},
			{"Charge", ParticleType::Charge},
			{"Glow", ParticleType::Glow},
			{"Shockwave", ParticleType::Shockwave},
			{"Sparkle", ParticleType::Sparkle},
			{"Slash", ParticleType::Slash},
			{"SmokeCloud", ParticleType::SmokeCloud}
		};

		for (const auto& [typeName, type] : typeMap) {
			if (j.contains(typeName)) {
				const nlohmann::json& paramJson = j[typeName];
				ParticleParam param;

				param.count = JsonUtil::GetValue<int>(paramJson, "count", 1);
				param.textureHandle = JsonUtil::GetValue<int>(paramJson, "textureHandle", -1);
				param.lifeMin = JsonUtil::GetValue<int>(paramJson, "lifeMin", 30);
				param.lifeMax = JsonUtil::GetValue<int>(paramJson, "lifeMax", 60);
				param.speedMin = JsonUtil::GetValue<float>(paramJson, "speedMin", 100.0f);
				param.speedMax = JsonUtil::GetValue<float>(paramJson, "speedMax", 200.0f);
				param.angleBase = JsonUtil::GetValue<float>(paramJson, "angleBase", 0.0f);
				param.angleRange = JsonUtil::GetValue<float>(paramJson, "angleRange", 360.0f);

				// 重力
				if (paramJson.contains("gravity")) {
					param.gravity.x = JsonUtil::GetValue<float>(paramJson["gravity"], "x", 0.0f);
					param.gravity.y = JsonUtil::GetValue<float>(paramJson["gravity"], "y", 0.0f);
				}

				// 加速度
				if (paramJson.contains("acceleration")) {
					param.acceleration.x = JsonUtil::GetValue<float>(paramJson["acceleration"], "x", 0.0f);
					param.acceleration.y = JsonUtil::GetValue<float>(paramJson["acceleration"], "y", 0.0f);
				}

				// 生成範囲
				if (paramJson.contains("emitRange")) {
					param.emitRange.x = JsonUtil::GetValue<float>(paramJson["emitRange"], "x", 0.0f);
					param.emitRange.y = JsonUtil::GetValue<float>(paramJson["emitRange"], "y", 0.0f);
				}

				param.sizeMin = JsonUtil::GetValue<float>(paramJson, "sizeMin", 16.0f);
				param.sizeMax = JsonUtil::GetValue<float>(paramJson, "sizeMax", 32.0f);
				param.scaleStart = JsonUtil::GetValue<float>(paramJson, "scaleStart", 1.0f);
				param.scaleEnd = JsonUtil::GetValue<float>(paramJson, "scaleEnd", 0.0f);
				param.colorStart = JsonUtil::GetValue<unsigned int>(paramJson, "colorStart", 0xFFFFFFFF);
				param.colorEnd = JsonUtil::GetValue<unsigned int>(paramJson, "colorEnd", 0xFFFFFF00);

				// 回転速度
				param.rotationSpeedMin = JsonUtil::GetValue<float>(paramJson, "rotationSpeedMin", 0.0f);
				param.rotationSpeedMax = JsonUtil::GetValue<float>(paramJson, "rotationSpeedMax", 0.0f);

				// アニメーション
				param.useAnimation = JsonUtil::GetValue<bool>(paramJson, "useAnimation", false);
				param.divX = JsonUtil::GetValue<int>(paramJson, "divX", 1);
				param.divY = JsonUtil::GetValue<int>(paramJson, "divY", 1);
				param.totalFrames = JsonUtil::GetValue<int>(paramJson, "totalFrames", 1);
				param.animSpeed = JsonUtil::GetValue<float>(paramJson, "animSpeed", 0.1f);

				// ブレンドモード
				std::string blendModeStr = JsonUtil::GetValue<std::string>(paramJson, "blendMode", "Normal");
				param.blendMode = StringToBlendMode(blendModeStr);

				// Emitter Shape
				if (paramJson.contains("emitterShape")) {
					std::string shapeStr = JsonUtil::GetValue<std::string>(paramJson, "emitterShape", "Point");
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

				// Emitter Size
				if (paramJson.contains("emitterSize")) {
					param.emitterSize.x = JsonUtil::GetValue<float>(paramJson["emitterSize"], "x", 0.0f);
					param.emitterSize.y = JsonUtil::GetValue<float>(paramJson["emitterSize"], "y", 0.0f);
				}

				// Homing
				param.useHoming = JsonUtil::GetValue<bool>(paramJson, "useHoming", false);
				param.homingStrength = JsonUtil::GetValue<float>(paramJson, "homingStrength", 0.0f);

				// 連続発生
				param.isContinuous = JsonUtil::GetValue<bool>(paramJson, "isContinuous", false);
				param.emitInterval = JsonUtil::GetValue<float>(paramJson, "emitInterval", 0.0f);

				// 環境パーティクル専用パラメータ
				param.bounceDamping = JsonUtil::GetValue<float>(paramJson, "bounceDamping", 0.3f);
				param.windStrength = JsonUtil::GetValue<float>(paramJson, "windStrength", 0.0f);
				param.floatAmplitude = JsonUtil::GetValue<float>(paramJson, "floatAmplitude", 0.0f);
				param.floatFrequency = JsonUtil::GetValue<float>(paramJson, "floatFrequency", 1.0f);

				// テクスチャハンドル復元
				if (param.textureHandle == -1) {
					switch (type) {
					case ParticleType::Explosion: param.textureHandle = texExplosion_; break;
					case ParticleType::Debris: param.textureHandle = texDebris_; break;
					case ParticleType::Hit: param.textureHandle = texHit_; break;
					case ParticleType::Dust: param.textureHandle = texDust_; break;
					case ParticleType::MuzzleFlash: param.textureHandle = texExplosion_; break;
					case ParticleType::Rain: param.textureHandle = texRain_; break;
					case ParticleType::Snow: param.textureHandle = texSnow_; break;
					case ParticleType::Orb: param.textureHandle = texOrb_; break;
					case ParticleType::Charge: param.textureHandle = texGlow_; break;
					case ParticleType::Glow: param.textureHandle = texGlow_; break;
					case ParticleType::Shockwave: param.textureHandle = texRing_; break;
					case ParticleType::Sparkle: param.textureHandle = texSparkle_; break;
					case ParticleType::Slash: param.textureHandle = texScratch_; break;
					case ParticleType::SmokeCloud: param.textureHandle = texSmoke_; break;
					}
				}

				params_[type] = param;
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

	// ファイルが存在しない場合はデフォルトパラメータで新規作成
	if (!JsonUtil::LoadFromFile(filepath, j)) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager: JSON file not found. Creating default parameters...\n");
#endif
		// デフォルトパラメータを設定
		LoadParams();
		// 新規作成して保存
		return SaveParamsToJson(filepath);
	}

	// JSONからパラメータを読み込み
	if (DeserializeParams(j)) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager: Parameters loaded from %s\n", filepath.c_str());
#endif
		return true;
	}

#ifdef _DEBUG
	Novice::ConsolePrintf("ParticleManager: Failed to load parameters. Using defaults.\n");
#endif
	LoadParams();
	return false;
}