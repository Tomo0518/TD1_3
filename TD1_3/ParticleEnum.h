#pragma once

/// <summary>
/// パーティクルの「挙動」の種類
/// </summary>
enum class ParticleBehavior {
	Physics,    // 物理挙動（速度・重力・回転あり：デブリなど）
	Stationary, // その場に留まる（アニメーションのみ：爆発など）
	Ghost,      // 残像（動きなし、その場でフェードアウト：ダッシュ残像）
	Homing      // ★追加：ターゲットに向かって収束（チャージなど）
};

/// <summary>
/// 発生源の形状
/// </summary>
enum class EmitterShape {
	Point,      // 点（デフォルト）
	Line,       // 線（雨・雪）
	Rectangle   // 矩形（エリア全体）
};

/// <summary>
/// エフェクトの種類
/// </summary>
enum class ParticleType {
	Explosion,
	Debris,
	Hit,
	Dust,
	MuzzleFlash,
	Rain,       // 雨
	Snow,       // 雪
	Orb,        // ふわふわオーブ
	Charge,      // チャージエフェクト

	Glow,        // 汎用光（particle_glow.png）
	Shockwave,   // 衝撃波（particle_ring.png）
	Sparkle,     // キラキラ（particle_sparkle.png）
	Slash,       // 斬撃・軌跡（particle_scratch.png）
	SmokeCloud,  // 雲・煙（particle_smoke.png）
	DigitalSpark
};