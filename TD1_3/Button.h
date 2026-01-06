#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include <string>
#include <functional>

class Button {
public:
	Button(const Vector2& position, const Vector2& size,
		const std::string& label, std::function<void()> callback);

	~Button(); // デストラクタを追加

	// コピーコンストラクタ
	Button(const Button& other);

	// ムーブコンストラクタ
	Button(Button&& other) noexcept;

	// テクスチャを設定
	void SetTexture(int textureHandle);

	// 更新処理
	void Update(float deltaTime, bool isSelected);

	// 描画処理
	void Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer) const;

	// コールバック実行
	void Execute();

	// 位置・サイズの設定
	void SetPosition(const Vector2& position);
	void SetSize(const Vector2& size);
	Vector2 GetPosition() const { return position_; }
	Vector2 GetSize() const { return size_; }

	// 見た目のカスタマイズ
	void SetColorNormal(uint32_t color) { colorNormal_ = color; }
	void SetColorSelected(uint32_t color) { colorSelected_ = color; }
	void SetTextScale(float scale) { textScale_ = scale; }

private:
	// 描画コンポーネント
	DrawComponent2D* drawComp_ = nullptr;

	// 位置とサイズ
	Vector2 position_ = { 0.0f, 0.0f };
	Vector2 size_ = { 100.0f, 50.0f };
	Vector2 anchor_ = { 0.5f, 0.5f };

	// ラベルとコールバック
	std::string label_;
	std::function<void()> callback_;

	// 色設定
	uint32_t colorNormal_ = 0x2A2A2AFF;         // 通常時の背景色
	uint32_t colorSelected_ = 0xFFFFBBFF;       // 選択時の背景色
	uint32_t colorFrame_ = 0xFFFFFFFF;          // 通常時の枠線色
	uint32_t colorFrameSelected_ = 0xFFFFBBFF;  // 選択時の枠線色
	uint32_t colorText_ = 0xCCCCCCFF;           // 通常時のテキスト色
	uint32_t colorTextSelected_ = 0xFFFF55FF;   // 選択時のテキスト色

	// テキストスケール
	float textScale_ = 1.5f;
	float textScaleSelected_ = 1.53f;

	// スケールエフェクト
	float scaleMin_ = 0.95f;
	float scaleMax_ = 1.05f;

	// イージング用
	float easeSpeed_ = 8.0f;
	float easeT_ = 0.0f;

	// 選択状態
	bool isSelected_ = false;

	// 内部処理
	void InitializeDrawComponent();
	void UpdateDrawComponent();
};