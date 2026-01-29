#include "UIElement.h"
#ifdef _DEBUG
#include "imgui.h"
#endif
#include <algorithm>

// =================================================================
// GaugeUIElement 実装
// =================================================================

GaugeUIElement::GaugeUIElement(const std::string& name, TextureId frameId,
    TextureId barId, const Vector2& position, const Vector2& barOffset)
    : UIElement(name, position)
    , frame_(frameId, 1, 1, 1, 0.0f)
    , bar_(barId, 1, 1, 1, 0.0f)
    , ghost_(barId, 1, 1, 1, 0.0f)
    , barOffset_(barOffset) {
    Vector2 anchor = { 0.5f, 0.5f };
    bar_.SetAnchorPoint(anchor);
    ghost_.SetAnchorPoint(anchor);

    bar_.SetCropDirection(CropDirection::Horizontal);
    ghost_.SetCropDirection(CropDirection::Horizontal);

    ghost_.SetBaseColor(0xFF4444FF);
}

void GaugeUIElement::Update(float dt) {
    if (!isVisible_) return;

    // ゴーストゲージの制御
    if (currentRatio_ < ghostRatio_) {
        ghostTimer_ += dt;
        if (ghostTimer_ > 0.5f) {
            float diff = ghostRatio_ - currentRatio_;
            float t = std::min(dt * 4.0f, 1.0f);
            float invT = 1.0f - t;
            float easedT = 1.0f - invT * invT * invT;
            ghostRatio_ -= diff * easedT;

            if (std::abs(ghostRatio_ - currentRatio_) < 0.01f) {
                ghostRatio_ = currentRatio_;
            }
        }
    }
    else {
        ghostRatio_ = currentRatio_;
        ghostTimer_ = 0.0f;
    }

    bar_.SetCropRatio(currentRatio_);
    ghost_.SetCropRatio(ghostRatio_);

    // 瀕死演出
    if (currentRatio_ <= 0.2f && currentRatio_ > 0.0f) {
        if (!bar_.IsFlashBlinking()) {
            bar_.StartFlashBlink(0xFF0000FF, 9999, 0.3f, kBlendModeAdd, 1);
        }
    }
    else {
        bar_.StopFlashBlink();
    }

    frame_.Update(dt);
    bar_.Update(dt);
    ghost_.Update(dt);
}

void GaugeUIElement::Draw() {
    if (!isVisible_) return;

    frame_.SetPosition(position_);
    frame_.SetScale(scale_, scale_);
    frame_.DrawScreen();

    Vector2 barPos = {
        position_.x + barOffset_.x * scale_,
        position_.y + barOffset_.y * scale_
    };

    ghost_.SetPosition(barPos);
    ghost_.SetScale(scale_, scale_);
    ghost_.DrawScreen();

    bar_.SetPosition(barPos);
    bar_.SetScale(scale_, scale_);
    bar_.DrawScreen();
}

void GaugeUIElement::Draw(const Vector2& drawPos) {
    if (!isVisible_) return;
    frame_.SetPosition(drawPos);
    frame_.SetScale(scale_, scale_);
    frame_.DrawScreen();
    Vector2 barPos = {
        drawPos.x + barOffset_.x * scale_,
        drawPos.y + barOffset_.y * scale_
    };
    ghost_.SetPosition(barPos);
    ghost_.SetScale(scale_, scale_);
    ghost_.DrawScreen();
    bar_.SetPosition(barPos);
    bar_.SetScale(scale_, scale_);
    bar_.DrawScreen();
}

void GaugeUIElement::DrawImGuiControls() {
#ifdef _DEBUG
    ImGui::PushID(name_.c_str());

    ImGui::Text("%s", name_.c_str());
    ImGui::Checkbox("Visible", &isVisible_);

    float pos[2] = { position_.x, position_.y };
    if (ImGui::DragFloat2("Position", pos, 1.0f)) {
        position_ = { pos[0], pos[1] };
    }

    ImGui::DragFloat("Scale", &scale_, 0.01f, 0.1f, 5.0f);

    float offset[2] = { barOffset_.x, barOffset_.y };
    if (ImGui::DragFloat2("Bar Offset", offset, 1.0f)) {
        barOffset_ = { offset[0], offset[1] };
    }

    float ratio = currentRatio_;
    if (ImGui::SliderFloat("Test Ratio", &ratio, 0.0f, 1.0f)) {
        SetRatio(ratio);
    }

    ImGui::PopID();
    ImGui::Separator();
#endif
}

void GaugeUIElement::SetRatio(float ratio) {
    float prev = currentRatio_;
    currentRatio_ = std::clamp(ratio, 0.0f, 1.0f);

    if (currentRatio_ < prev) {
        ghostTimer_ = 0.0f;
        if (prev - currentRatio_ > 0.1f) {
            frame_.StartShake(5.0f, 0.3f);
        }
    }
    else if (currentRatio_ > prev) {
        bar_.StartFlashBlink(0x00FF00FF, 1, 0.3f, kBlendModeAdd);
    }
}

void GaugeUIElement::SetColor(uint32_t color) {
    bar_.SetBaseColor(color);
}

// =================================================================
// KeyGuideUIElement 実装
// =================================================================

KeyGuideUIElement::KeyGuideUIElement(const std::string& name, TextureId texId, const Vector2& position, const float scale)
    : UIElement(name, position) {
    component_ = std::make_unique<DrawComponent2D>(texId, 1, 1, 1, 0.0f);
    scale_ = scale;
}

void KeyGuideUIElement::Update(float dt) {
    if (!isVisible_) return;
    component_->Update(dt);
}

void KeyGuideUIElement::Draw() {
    if (!isVisible_) return;
    component_->SetPosition(position_);
    component_->SetScale(scale_, scale_);
    component_->DrawScreen();
}

void KeyGuideUIElement::Draw(const Vector2& drawPos) {
    if (!isVisible_) return;
    component_->SetPosition(drawPos);
    component_->SetScale(scale_, scale_);
    component_->DrawScreen();
}

void KeyGuideUIElement::DrawImGuiControls() {
#ifdef _DEBUG
    ImGui::PushID(name_.c_str());
    ImGui::Text("%s", name_.c_str());
    ImGui::Checkbox("Visible", &isVisible_);

    float pos[2] = { position_.x, position_.y };
    if (ImGui::DragFloat2("Position", pos, 1.0f)) {
        position_ = { pos[0], pos[1] };
    }

    ImGui::DragFloat("Scale", &scale_, 0.01f, 0.1f, 5.0f);

    if (ImGui::Button("Test Squash")) {
        TriggerSquash();
    }

    ImGui::PopID();
    ImGui::Separator();
#endif
}

void KeyGuideUIElement::TriggerSquash() {
    component_->StartSquash({ 1.2f, 0.8f }, 0.1f);
}

void KeyGuideUIElement::SetTexture(TextureId texId) {
    component_->SetTexture(texId);
}

void KeyGuideUIElement::StartPulse(float from, float to, float duration, bool loop) {
    component_->StartPulse(from, to, duration, loop);
}

void KeyGuideUIElement::StartShake(float power, float duration) {
    component_->StartShake(power, duration);
}

void KeyGuideUIElement::SetBaseColor(uint32_t color) {
    component_->SetBaseColor(color);
}

void KeyGuideUIElement::SetDrawSize(float width, float height) {
    component_->SetDrawSize(width, height);
}

// =================================================================
// SkillIcon 実装
// =================================================================
// =================================================================
// SkillIcon 実装
// =================================================================

SkillIcon::SkillIcon(const Vector2& position)
    : iconPos_(position) {
    icon_.second = 0; // インデックスを初期化
}

void SkillIcon::Update(float dt, bool useSkill, bool canUseSkill, bool isWaiting) {
    // アイコンの更新
    for (auto& iconComp : icon_.first) {
        iconComp->Update(dt);
    }

    // 使用不可状態なら半透明にする
    if (!canUseSkill || isWaiting) {
        for (auto& iconComp : icon_.first) {
            iconComp->SetBaseColor(0x4e76af77); // 半透明
        }
    }
    else {
        for (auto& iconComp : icon_.first) {
            iconComp->SetBaseColor(0xFFFFFFFF); // 不透明
        }
    }

    // 状態変化を検出（false→true または true→false）
    if (useSkill != prevUseSkill_) {
        // アイコンを次に切り替え
        icon_.second = (icon_.second + 1) % std::max(1, (int)icon_.first.size());

        // アイコンにリアクションを与える
        if (icon_.second < icon_.first.size()) {
            icon_.first[icon_.second]->StartSquash({ 1.2f, 0.7f }, 0.2f);
        }
    }

    // 前フレームの状態を保存
    prevUseSkill_ = useSkill;
}

void SkillIcon::Draw() {
    if (!icon_.first.empty()) {
        // 現在のアイコンを描画
        auto& iconComp = icon_.first[icon_.second];
        iconComp->SetPosition(iconPos_);
        iconComp->DrawScreen();
    }
}

void SkillIcon::Draw(const Vector2& drawPos) {
    if (!icon_.first.empty()) {
        // 現在のアイコンを描画
        auto& iconComp = icon_.first[icon_.second];
        iconComp->SetPosition(drawPos);
        iconComp->DrawScreen();
    }
}

void SkillIcon::AddIconTexture(TextureId texId, int iconId) {
    auto iconComp = std::make_unique<DrawComponent2D>(texId, 1, 1, 1, 0.0f);
    icon_.first.push_back(std::move(iconComp));
    icon_.second = iconId;
}

void SkillIcon::SetPosition(const Vector2& pos) {
    iconPos_ = pos;
}

// =================================================================
// SkillIconUIElement 実装
// =================================================================

SkillIconUIElement::SkillIconUIElement(const std::string& name, const Vector2& position)
    : UIElement(name, position) {
    skillIcon_ = std::make_unique<SkillIcon>(position);
}

void SkillIconUIElement::Update(float dt) {
    dt;
    if (!isVisible_) return;
    // UpdateStateで更新されるため、ここでは何もしない
}

void SkillIconUIElement::Draw() {
    if (!isVisible_) return;
    skillIcon_->Draw();
}

void SkillIconUIElement::Draw(const Vector2& drawPos) {
    if (!isVisible_) return;
    skillIcon_->Draw(drawPos);
}

void SkillIconUIElement::DrawImGuiControls() {
#ifdef _DEBUG
    ImGui::PushID(name_.c_str());
    ImGui::Text("%s", name_.c_str());
    ImGui::Checkbox("Visible", &isVisible_);

    float pos[2] = { position_.x, position_.y };
    if (ImGui::DragFloat2("Position", pos, 1.0f)) {
        position_ = { pos[0], pos[1] };
        SetPosition(position_);
    }

    ImGui::PopID();
    ImGui::Separator();
#endif
}

void SkillIconUIElement::SetPosition(const Vector2& pos) {
    position_ = pos;
    skillIcon_->SetPosition(pos);
}

void SkillIconUIElement::AddIconTexture(TextureId texId) {
    skillIcon_->AddIconTexture(texId);
}

void SkillIconUIElement::UpdateState(float dt, bool isActive, bool canUse, bool halfTransparent) {
    if (!isVisible_) return;
    skillIcon_->Update(dt, isActive, canUse, halfTransparent);
}