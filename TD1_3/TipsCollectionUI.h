#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "TextureManager.h"
#include "InputManager.h"
#include "TipsManager.h"
#include <memory>

#ifdef _DEBUG
#include "imgui.h"
#endif

/// <summary>
/// Tips一覧UI（見開き本）の管理クラス
/// </summary>
class TipsCollectionUI {
public:
    TipsCollectionUI();
    ~TipsCollectionUI() = default;

    void Initialize();
    void Update(float deltaTime);
    void Draw();

    // 開閉状態
    bool IsOpen() const { return isOpen_; }
    void Open();
    void Close();
    void Toggle();

#ifdef _DEBUG
    void DrawImGui();
#endif

private:
    void UpdateInput();
    void UpdateAnimation(float deltaTime);

    // 開閉アニメーション
    enum class AnimState {
        Closed,   // 完全に閉じている
        Opening,  // 開いている途中
        Open,     // 完全に開いている
        Closing   // 閉じている途中
    };

    AnimState animState_;
    float animTimer_;
    float animDuration_;

    // 開閉状態
    bool isOpen_;
    float currentScale_;  // アニメーション用スケール（0.0 ~ 1.0）

    // 見開き本の背景
    std::unique_ptr<DrawComponent2D> bookBackground_;

    // 配置設定
    Vector2 bookPosition_;  // 画面中央
    Vector2 bookSize_;      // 見開き本のサイズ

    // デバッグ用調整パラメータ
#ifdef _DEBUG
    float debugOpenDuration_;
    float debugCloseDuration_;
#endif
};