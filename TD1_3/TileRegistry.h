#pragma once
#include <string>
#include <vector>
#include "TextureManager.h" // TextureId定義のためインクルード

enum class TileLayer {
	Background, //
	BackgroundDecoration, // 装飾タイル（当たり判定なし、背景の一つ上のレイヤー）
	Block,      // 地形（当たり判定あり、描画順は中）
	Decoration, // 装飾（当たり判定なし、描画順は奥 or 手前）
	Object      // ゲームオブジェクト配置用（実行時は消える）
};

// タイルの種類
enum class TileType {
	None,       // 空気
	Solid,      // 通常ブロック
	AutoTile    // オートタイル
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
	TileAnimConfig animConfig; // アニメーション設定
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
			0, "Air",
			TextureId::Count,
			TileType::None,
			false,
			TileLayer::Block,
			{0.0f, 0.0f},
			RenderMode::Simple,{}
			});

		// ID:1 地面 (オートタイル)
		tiles_.push_back({
			1, "Ground",
			TextureId::GroundAuto,
			TileType::AutoTile,
			true,
			TileLayer::Block,
			{0.0f, 0.0f},
			RenderMode::Simple,{}
			});

		// ID:2 鉄ブロック
		tiles_.push_back({
			2, "Iron",
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
			10, "Grass", TextureId::Deco_GrassAnim, TileType::Solid, false, // 当たり判定なし
			TileLayer::Decoration, {0.0f, 8.0f}, // オフセットで位置微調整
			RenderMode::Component,
			{ true, 8, 1, 8, 0.15f,true,{0.5f, 0.8f} }// アニメーション設定
			});

		// ID:11 看板 (Decoration)
		tiles_.push_back({
			11, "Sign", TextureId::Deco_Scrap, TileType::Solid, false,
			TileLayer::Decoration, {0.0f, 0.0f},
			RenderMode::Component,
			{ false, 1, 1, 1, 0.0f } // アニメーションなし
			});

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
			200, "BackgroundRockBlock", TextureId::Deco_Background_RockBlock, TileType::Solid , false,
			TileLayer::BackgroundDecoration, {0.0f, 0.0f},
			RenderMode::Simple,{}
			});
		// ID:201 背景アイスブロック 
		tiles_.push_back({
			201, "BackgroundIceBlock", TextureId::Deco_Background_IceBlock, TileType::None , false,
			TileLayer::BackgroundDecoration, {0.0f, 0.0f},
			RenderMode::Simple,{}
			});
	}

private:
	static std::vector<TileDefinition> tiles_;
};

inline std::vector<TileDefinition> TileRegistry::tiles_;