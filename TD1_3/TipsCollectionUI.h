#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "TextureManager.h"
#include "InputManager.h"
#include "TipsManager.h"
#include "TipsCardUI.h"  // 追加
#include <memory>
#include <vector>  // 追加

#ifdef _DEBUG
#include "imgui.h"
#endif

class TipsCollectionUI {
public:
    TipsCollectionUI();
    ~TipsCollectionUI() = default;

    void Initialize();
    void Update(float deltaTime);
    void Draw();

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
    void InitializeCards();  // 追加

    enum class AnimState {
        Closed,
        Opening,
        Open,
        Closing
    };

    AnimState animState_;
    float animTimer_;
    float animDuration_;

    bool isOpen_;
    float currentScale_;

    std::unique_ptr<DrawComponent2D> bookBackground_;
    std::unique_ptr<DrawComponent2D> lockIcon_;

    Vector2 bookPosition_;
    Vector2 bookSize_;

    // カード管理（追加）

    std::vector<std::unique_ptr<TipsCardUI>> cards_;
    Vector2 cardGridStart_;   // グリッドの開始位置
    Vector2 cardSize_;        // 各カードのサイズ
    Vector2 cardSpacing_;     // カード間の間隔

#ifdef _DEBUG
    float debugOpenDuration_;
    float debugCloseDuration_;
#endif
};