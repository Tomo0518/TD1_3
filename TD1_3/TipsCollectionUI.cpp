#include "TipsCollectionUI.h"
#include <algorithm>
#include <cmath>
#include "SceneUtilityIncludes.h"

TipsCollectionUI::TipsCollectionUI()
    : animState_(AnimState::Closed)
    , animTimer_(0.0f)
    , animDuration_(0.3f)
    , isOpen_(false)
    , currentScale_(0.0f)
    , bookPosition_({ 640.0f, 360.0f })
    , bookSize_({ 1000.0f, 600.0f })
#ifdef _DEBUG
    , debugOpenDuration_(0.3f)
    , debugCloseDuration_(0.2f)
#endif
{
}

void TipsCollectionUI::Initialize() {
    // 見開き本の背景を初期化
    bookBackground_ = std::make_unique<DrawComponent2D>(
        TextureId::TipsBookBackground 
    );
    bookBackground_->SetDrawSize(bookSize_);
    bookBackground_->SetAnchorPoint({ 0.5f, 0.5f });
    bookBackground_->SetPosition(bookPosition_);

    // 初期状態は閉じている
    animState_ = AnimState::Closed;
    currentScale_ = 0.0f;
}

void TipsCollectionUI::Update(float deltaTime) {
    // 入力処理
    UpdateInput();

    // アニメーション更新
    UpdateAnimation(deltaTime);

    // DrawComponent の更新
    if (bookBackground_) {
        bookBackground_->Update(deltaTime);
    }
}

void TipsCollectionUI::Draw() {
    // 完全に閉じている時は描画しない
    if (animState_ == AnimState::Closed) {
        return;
    }

    // スケールアニメーションを適用して描画
    if (bookBackground_) {
        // 現在のスケールを設定
        bookBackground_->SetScale(currentScale_, currentScale_);

        // アルファ値も連動させる
        float alpha = currentScale_;
        unsigned int color = 0xFFFFFF00 | static_cast<unsigned int>(alpha * 255.0f);
        bookBackground_->SetBaseColor(color);

        bookBackground_->DrawScreen();
    }
}

void TipsCollectionUI::Open() {
    if (animState_ == AnimState::Open || animState_ == AnimState::Opening) {
        return;
    }

    animState_ = AnimState::Opening;
    animTimer_ = 0.0f;
    isOpen_ = true;

#ifdef _DEBUG
    animDuration_ = debugOpenDuration_;
#else
    animDuration_ = 0.3f;
#endif
}

void TipsCollectionUI::Close() {
    if (animState_ == AnimState::Closed || animState_ == AnimState::Closing) {
        return;
    }

    animState_ = AnimState::Closing;
    animTimer_ = 0.0f;
    isOpen_ = false;

#ifdef _DEBUG
    animDuration_ = debugCloseDuration_;
#else
    animDuration_ = 0.2f;
#endif
}

void TipsCollectionUI::Toggle() {
    if (isOpen_) {
        Close();
    }
    else {
        Open();
    }
}

void TipsCollectionUI::UpdateInput() {
    // M キー or Y ボタンで開閉
    bool togglePressed =
        Input().TriggerKey(DIK_M) ||
        Input().GetPad()->Trigger(Pad::Button::Y);

    if (togglePressed) {
        Toggle();
        // SE再生（必要に応じて）
        // Sound().PlaySe(SeId::Menu_Toggle);
    }

    // B ボタンで閉じる（開いている時のみ）
    if (isOpen_ && Input().GetPad()->Trigger(Pad::Button::B)) {
        Close();
        // Sound().PlaySe(SeId::Cancel);
    }
}

void TipsCollectionUI::UpdateAnimation(float deltaTime) {
    if (animState_ == AnimState::Closed || animState_ == AnimState::Open) {
        return;
    }

    animTimer_ += deltaTime;
    float t = std::min(animTimer_ / animDuration_, 1.0f);

    switch (animState_) {
    case AnimState::Opening: {
        // EaseOutBack を使って弾むような開き方
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float eased = 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
        currentScale_ = std::clamp(eased, 0.0f, 1.0f);

        if (t >= 1.0f) {
            animState_ = AnimState::Open;
            currentScale_ = 1.0f;
        }
        break;
    }

    case AnimState::Closing: {
        // EaseInBack を使ってスムーズに閉じる
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float reversed = 1.0f - t;
        float eased = c3 * std::pow(reversed, 3.0f) - c1 * std::pow(reversed, 2.0f);
        currentScale_ = std::clamp(eased, 0.0f, 1.0f);

        if (t >= 1.0f) {
            animState_ = AnimState::Closed;
            currentScale_ = 0.0f;
        }
        break;
    }

    default:
        break;
    }
}

#ifdef _DEBUG
void TipsCollectionUI::DrawImGui() {
    ImGui::Begin("Tips Collection UI Config");

    ImGui::Text("State: %s",
        animState_ == AnimState::Closed ? "Closed" :
        animState_ == AnimState::Opening ? "Opening" :
        animState_ == AnimState::Open ? "Open" : "Closing");
    ImGui::Text("Current Scale: %.2f", currentScale_);

    ImGui::Separator();
    ImGui::DragFloat2("Book Position", &bookPosition_.x, 1.0f);
    ImGui::DragFloat2("Book Size", &bookSize_.x, 1.0f);

    ImGui::Separator();
    ImGui::DragFloat("Open Duration", &debugOpenDuration_, 0.01f, 0.1f, 2.0f);
    ImGui::DragFloat("Close Duration", &debugCloseDuration_, 0.01f, 0.1f, 2.0f);

    if (ImGui::Button("Toggle")) {
        Toggle();
    }

    // パラメータ適用
    if (bookBackground_) {
        bookBackground_->SetPosition(bookPosition_);
        bookBackground_->SetDrawSize(bookSize_);
    }

    ImGui::End();
}
#endif