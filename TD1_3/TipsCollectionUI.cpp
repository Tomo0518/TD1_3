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
    , cardGridStart_({ 320.0f, 180.0f })   
    , cardSize_({ 220.0f, 260.0f })        
    , cardSpacing_({ 240.0f, 280.0f })     
#ifdef _DEBUG
    , debugOpenDuration_(0.3f)
    , debugCloseDuration_(0.2f)
#endif
{
}

// Initialize() でロックアイコンを初期化（既存のコード）
void TipsCollectionUI::Initialize() {
    bookBackground_ = std::make_unique<DrawComponent2D>(
        TextureId::TipsBookBackground
    );
    bookBackground_->SetDrawSize(bookSize_);
    bookBackground_->SetAnchorPoint({ 0.5f, 0.5f });
    bookBackground_->SetPosition(bookPosition_);

    // ロックアイコン（全カード共通）
    lockIcon_ = std::make_unique<DrawComponent2D>(TextureId::TipsLockIcon);
    lockIcon_->SetAnchorPoint({ 0.5f, 0.5f });

    InitializeCards();

    animState_ = AnimState::Closed;
    currentScale_ = 0.0f;
}


void TipsCollectionUI::InitializeCards() {
    cards_.clear();

    // 4枚のカードを 2x2 グリッドで配置
    // Tips ID は 1, 2, 3, 4
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            int tipsId = row * 2 + col + 1;  // 1, 2, 3, 4

            // カードの位置を計算
            Vector2 cardPos = {
                cardGridStart_.x + col * cardSpacing_.x,
                cardGridStart_.y + row * cardSpacing_.y
            };

            auto card = std::make_unique<TipsCardUI>(tipsId, cardPos, cardSize_);
            card->Initialize();
            cards_.push_back(std::move(card));
        }
    }
}

void TipsCollectionUI::Update(float deltaTime) {
    UpdateInput();
    UpdateAnimation(deltaTime);

    if (bookBackground_) {
        bookBackground_->Update(deltaTime);
    }

    // カードを更新（開いている時のみ）
    if (animState_ == AnimState::Open || animState_ == AnimState::Opening) {
        for (auto& card : cards_) {
            card->Update(deltaTime);
        }
    }
}


// Draw() でロックアイコンを渡す
void TipsCollectionUI::Draw() {
    if (animState_ == AnimState::Closed) {
        return;
    }

    // 背景描画
    if (bookBackground_) {
        bookBackground_->SetScale(currentScale_, currentScale_);
        float alpha = currentScale_;
        unsigned int color = 0xFFFFFF00 | static_cast<unsigned int>(alpha * 255.0f);
        bookBackground_->SetBaseColor(color);
        bookBackground_->DrawScreen();
    }

    // カード描画（背景が50%以上開いたら表示）
    if (currentScale_ > 0.5f && lockIcon_) {
        lockIcon_->Update(0.0f);  // ロックアイコンを更新

        for (auto& card : cards_) {
            card->Draw(lockIcon_.get());  // ロックアイコンを渡す
        }
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
    bool togglePressed =
        Input().TriggerKey(DIK_M) ||
        Input().GetPad()->Trigger(Pad::Button::Y);

    if (togglePressed) {
        Toggle();
    }

    if (isOpen_ && Input().GetPad()->Trigger(Pad::Button::B)) {
        Close();
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
    ImGui::Text("Cards Count: %d", static_cast<int>(cards_.size()));

    ImGui::Separator();
    ImGui::DragFloat2("Book Position", &bookPosition_.x, 1.0f);
    ImGui::DragFloat2("Book Size", &bookSize_.x, 1.0f);

    ImGui::Separator();
    ImGui::Text("Card Grid Settings");
    ImGui::DragFloat2("Grid Start", &cardGridStart_.x, 1.0f);
    ImGui::DragFloat2("Card Size", &cardSize_.x, 1.0f);
    ImGui::DragFloat2("Card Spacing", &cardSpacing_.x, 1.0f);

    ImGui::Separator();
    ImGui::DragFloat("Open Duration", &debugOpenDuration_, 0.01f, 0.1f, 2.0f);
    ImGui::DragFloat("Close Duration", &debugCloseDuration_, 0.01f, 0.1f, 2.0f);

    if (ImGui::Button("Toggle")) {
        Toggle();
    }

    if (ImGui::Button("Reinitialize Cards")) {
        InitializeCards();
    }

    // パラメータ適用
    if (bookBackground_) {
        bookBackground_->SetPosition(bookPosition_);
        bookBackground_->SetDrawSize(bookSize_);
    }

    // カード位置の再計算
    int index = 0;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            if (index < cards_.size()) {
                Vector2 cardPos = {
                    cardGridStart_.x + col * cardSpacing_.x,
                    cardGridStart_.y + row * cardSpacing_.y
                };
                cards_[index]->SetPosition(cardPos);
            }
            index++;
        }
    }

    ImGui::End();
}
#endif