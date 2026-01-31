#pragma once
#include <string>
#include <vector>
#include "TextureManager.h" // TextureId定義のためインクルード

enum class TileLayer {
	Background, //
	BackgroundDecoration, // 装飾タイル（当たり判定なし、背景の一つ上のレイヤー）
	Block,      // 地形（当たり判定あり、描画順は中）
	Decoration, // 装飾（当たり判定なし、描画順は奥 or 手前）
	Object,      // ゲームオブジェクト配置用（実行時は消える）
};

enum class DrawLayer {
	Background,
	Foreground
};

// タイルの種類
enum class TileType {
	None,       // 空気
	Solid,      // 通常ブロック
	AutoTile    // オートタイル
};

enum TileID{
	Air = 0,
	Block_Ground = 1,
	Block_Iron = 2,

	Deco_Grass = 10,
	Deco_Sign = 11,
	Deco_Sign2 = 14,
	Deco_Sign3 = 15,
	Deco_Rock1 = 12,
	Deco_Bush1 = 13,

	// Tutorial Images
	JumpTutorial = 50,
	DashTutorial = 51,
	ThrowTutorial = 52,
	BoomerangTutorial = 53,
	ChargeTutorial = 57,
	ReturnTutorial = 58,
	StunTutorial = 59,
	Deco_Rock1_F = 54,
	Deco_Bush1_F = 55,
	Deco_BushDark_F = 56,

	BoomerangJumpTutorial = 60,
	BoomerangThrowToJump = 61,
	BoomerangFocusPlayer = 62,

	//PlayerStart = 100,

	Deco_RockBlock = 200,
	Deco_IceBlock = 201,
};

// 描画の仕組みを定義
enum class RenderMode {
	Simple,      // MapChipでの描画（高速・軽量）
	Component    // DrawComponentを使用（アニメーション・エフェクト・揺れ）
};

// アニメーションの詳細設定用
struct TileAnimConfig {
	bool isAnimated = false; // アニメーションするか
	int divX = 1;            // 横分割
	int divY = 1;            // 縦分割
	int totalFrames = 1;     // 総フレーム数
	float speed = 0.0f;      // 再生速度
	bool isLoop = true;      // ループするか
	Vector2 anchorPoint = { 0.5f, 0.5f };
};

struct TileDefinition {
	int id;                 // ID
	std::string name;       // エディタ表示名
	TextureId textureId;    // TextureManagerで使うID
	TileType type;          // 種類
	bool isSolid;           // 当たり判定
	TileLayer layer;     // 所属するレイヤー
	Vector2 drawOffset; // 描画オフセット

	RenderMode renderMode; // 描画モード(MapChipかComponentか)
	DrawLayer drawLayer = DrawLayer::Background; // 描画レイヤー(背景か前景か)
	TileAnimConfig animConfig = { false, 1, 1, 1, 0.0f }; // アニメーション設定
};

class TileRegistry {
public:
	static const std::vector<TileDefinition>& GetAllTiles() { return tiles_; }

	static const TileDefinition* GetTile(int id) {
		for (const auto& tile : tiles_) {
			if (tile.id == id) return &tile;
		}
		return nullptr;
	}

	static void Initialize() {
		tiles_.clear();

		// ==================================
		// タイル定義登録方法
		// ==================================
		//struct TileDefinition {
		//	int id;                 // ID
		//	std::string name;       // エディタ表示名
		//	TextureId textureId;    // TextureManagerで使うID
		//	TileType type;          // 種類
		//	bool isSolid;           // 当たり判定
		//	TileLayer layer;		// 所属するレイヤー
		//	Vector2 drawOffset;		// 描画オフセット
		//	RenderMode renderMode;	// 描画モード(MapChipかComponentか)
		//	TileAnimConfig animConfig{
		//		true(アニメーションあるかどうか), 8(スプライトシートの横), 1(縦), 8(総フレーム数),
		//		0.15f(アニメーション速度),true(ループするかどうか),{0.5f, 0.8f}(アンカーポイントの位置)};
		//};

		// ID:0 空気 (テクスチャなし)
		tiles_.push_back({
			TileID::Air, "Air",
			TextureId::Count,
			TileType::None,
			false,
			TileLayer::Block,
			{0.0f, 0.0f},
			RenderMode::Simple,{}
			});

		// ID:1 地面 (オートタイル)
		tiles_.push_back({
			TileID::Block_Ground, "Ground",
			TextureId::GroundAuto,
			TileType::AutoTile,
			true,
			TileLayer::Block,
			{0.0f, 0.0f},
			RenderMode::Simple,{}
			});

		// ID:2 鉄ブロック
		tiles_.push_back({
			TileID::Block_Iron, "Iron",
			TextureId::GroundAuto,
			TileType::Solid,
			true,
			TileLayer::Block,
			{0.0f, 0.0f},
			RenderMode::Simple,{}
			});

		// ==================================
		// --- Decoration Layer (装飾) (Decoration)
		// ==================================
		// ID:10 草 (Decoration)
		tiles_.push_back({
			TileID::Deco_Grass, "Grass", TextureId::Deco_GrassAnim, TileType::Solid, false, // 当たり判定なし
			TileLayer::Decoration, {0.0f, 32.0f}, // オフセットで位置微調整
			RenderMode::Component,DrawLayer::Background,
			{ true, 8, 1, 8, 12.f,true,{0.5f, 1.0f} }// アニメーション設定
			});

		// ID:11 看板 (Decoration)
		tiles_.push_back({
			TileID::Deco_Sign, "Sign", TextureId::Deco_Sign, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Background,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});
		// ID:11 看板 (Decoration)
		tiles_.push_back({
			TileID::Deco_Sign2, "Sign2", TextureId::Deco_Sign2, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Background,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});
		// ID:11 看板 (Decoration)
		tiles_.push_back({
			TileID::Deco_Sign3, "Sign3", TextureId::Deco_Sign3, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Background,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		// ID:12 rock (Decoration)
		tiles_.push_back({
			TileID::Deco_Rock1, "Rock1", TextureId::Deco_Rock1, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Background,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		// ID:13 bush (Decoration)
		tiles_.push_back({
			TileID::Deco_Bush1, "Bush1", TextureId::Deco_Bush1, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Background,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		// **************************************
		// tutorial image (decoration)
		// **************************************
		tiles_.push_back({
			TileID::JumpTutorial, "JumpTutorial", TextureId::JumpTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::DashTutorial, "DashTutorial", TextureId::DashTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::ThrowTutorial, "ThrowTutorial", TextureId::ThrowTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::BoomerangTutorial, "BoomerangTutorial", TextureId::BoomerangTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});
		tiles_.push_back({
			TileID::ChargeTutorial, "ChargeTutorial", TextureId::ChargeTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});
		tiles_.push_back({
			TileID::ReturnTutorial, "ReturnTutorial", TextureId::ReturnTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::StunTutorial, "StunTutorial", TextureId::StunTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::BoomerangJumpTutorial, "BoomerangJumpTutorial", TextureId::BoomerangJumpTutorial, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::BoomerangThrowToJump, "BoomerangThrowToJump", TextureId::BoomerangThrowToJump, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::BoomerangFocusPlayer, "BoomerangFocusPlayer", TextureId::BoomerangFocusPlayer, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		// **************************************
		// foreground (decoration)
		// **************************************
		tiles_.push_back({
			TileID::Deco_Rock1_F, "Rock1", TextureId::Deco_Rock1, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

		tiles_.push_back({
			TileID::Deco_Bush1_F, "Bush1", TextureId::Deco_Bush1, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});
		tiles_.push_back({
			TileID::Deco_BushDark_F, "Bush1", TextureId::Deco_BushDark, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,DrawLayer::Foreground,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});
		

		// **************************************
		// --- Object Layer (配置物) ---
		// ID:100 プレイヤースタート位置
		tiles_.push_back({
			100, "PlayerStart", TextureId::None, TileType::Solid, false,
			TileLayer::Object, {0.0f, 0.0f}
			});

		// ==================================
		// 背景装飾タイル (BackgroundDecoration)
		// ==================================
		// ID:200 背景岩ブロック
		tiles_.push_back({
			TileID::Deco_RockBlock, "BackgroundRockBlock", TextureId::Deco_Background_RockBlock, TileType::AutoTile , false,
			TileLayer::BackgroundDecoration, {0.0f, 0.0f},
			RenderMode::Simple,{}
			});
		// ID:201 背景アイスブロック 
		tiles_.push_back({
			TileID::Deco_IceBlock, "BackgroundIceBlock", TextureId::Deco_Background_IceBlock, TileType::None , false,
			TileLayer::BackgroundDecoration, {0.0f, 0.0f},
			RenderMode::Simple,{}
			});
	}

private:
	static std::vector<TileDefinition> tiles_;
};

inline std::vector<TileDefinition> TileRegistry::tiles_;