#pragma once
#include "Button.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include "Pad.h"
#include <vector>
#include <memory>

/// <summary>
/// 複数のボタンを管理するクラス
/// </summary>
class ButtonManager {
public:
	ButtonManager();

	/// <summary>
	/// ボタンを追加
	/// </summary>
	/// <param name="position">ボタンの位置</param>
	/// <param name="size">ボタンのサイズ</param>
	/// <param name="label">ボタンのラベル</param>
	/// <param name="callback">ボタンが押された時のコールバック</param>
	void AddButton(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback);

	/// <summary>
	/// ボタンを追加（Buttonオブジェクトを直接渡す）
	/// </summary>
	void AddButton(const Button& button);

	/// <summary>
	/// すべてのボタンをクリア
	/// </summary>
	void ClearButtons();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	/// <param name="keys">キーボードの入力状態</param>
	/// <param name="preKeys">前フレームのキーボード入力状態</param>
	/// <param name="pad">パッドの参照</param>
	void Update(float deltaTime, const char* keys, const char* preKeys, Pad& pad);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="textureHandle">ボタンのテクスチャハンドル</param>
	/// <param name="font">フォントアトラス</param>
	/// <param name="textRenderer">テキストレンダラー</param>
	void Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer);

	// ゲッター
	int GetSelectedIndex() const { return selectedIndex_; }
	size_t GetButtonCount() const { return buttons_.size(); }

	// セッター
	void SetSelectedIndex(int index);

	void SetButtonTexture(int textureHandle);

	// ループ設定
	void SetLoopNavigation(bool loop) { loopNavigation_ = loop; }
	bool IsLoopNavigation() const { return loopNavigation_; }

	// SE再生用のコールバック設定
	void SetOnSelectSound(std::function<void()> callback) { onSelectSound_ = callback; }
	void SetOnDecideSound(std::function<void()> callback) { onDecideSound_ = callback; }

	// 初期フレームフラグ（パッド入力の誤検出防止）
	void SetFirstFrame(bool flag) { firstFrame_ = flag; }

private:
	std::vector<Button> buttons_;   // ボタンのリスト
	int selectedIndex_ = 0;         // 現在選択中のボタンのインデックス
	bool loopNavigation_ = true;    // ループナビゲーションを有効にするか

	// パッド入力の前フレーム値（誤検出防止用）
	float prevLY_ = 0.0f;
	bool firstFrame_ = true;

	// SE再生用のコールバック
	std::function<void()> onSelectSound_;  // 選択時のSE
	std::function<void()> onDecideSound_;  // 決定時のSE

	// 入力処理
	void HandleKeyboardInput(const char* keys, const char* preKeys);
	void HandlePadInput(Pad& pad);
};