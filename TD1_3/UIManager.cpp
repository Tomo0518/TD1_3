#include "UIManager.h"
#include "imgui.h"
#include <algorithm>

// =================================================================
// UIManager 実装のみ
// =================================================================

void UIManager::Initialize() {
    uiElements_.clear();
    elementMap_.clear();

    InitializeGauges();
    InitializeKeyGuides();
    InitializeSkillIcons();
    InitializePauseAndResult();
}

void UIManager::InitializeGauges() {
    // プレイヤーHP
    auto playerHP = std::make_unique<GaugeUIElement>(
        "PlayerHP", TextureId::PlayerHPFrame, TextureId::PlayerHPBar,
        Vector2(250.0f, 650.0f), Vector2(0.0f, 0.0f)
    );
    playerHP->SetColor(0xFFFFFFFF);
    AddElement(std::move(playerHP));
    gaugeElements_.push_back("PlayerHP");

    // ボスHP
    auto bossHP = std::make_unique<GaugeUIElement>(
        "BossHP", TextureId::BossHPFrame, TextureId::BossHPBar,
        Vector2(640.0f, 50.0f), Vector2(-150.0f, 0.0f)
    );
    bossHP->SetColor(0xFF0000FF);
    bossHP->SetScale(0.8f);
    AddElement(std::move(bossHP));
    gaugeElements_.push_back("BossHP");
}

void UIManager::InitializeKeyGuides() {
    Vector2 basePos = { 981.f,672.f};
    float keyScale = 0.72f;
    float offset = 50.0f;

    // キーボード用
    AddElement(std::make_unique<KeyGuideUIElement>("KeyW", TextureId::KeyW, Vector2(basePos.x, basePos.y - offset), keyScale));
    AddElement(std::make_unique<KeyGuideUIElement>("KeyA", TextureId::KeyA, Vector2(basePos.x - offset, basePos.y), keyScale));
    AddElement(std::make_unique<KeyGuideUIElement>("KeyS", TextureId::KeyS, basePos, keyScale));
    AddElement(std::make_unique<KeyGuideUIElement>("KeyD", TextureId::KeyD, Vector2(basePos.x + offset, basePos.y), keyScale));
    AddElement(std::make_unique<KeyGuideUIElement>("KeyJ", TextureId::KeyJ, Vector2(1100.f,665.f)));
    AddElement(std::make_unique<KeyGuideUIElement>("KeyK", TextureId::KeyK, Vector2(1205.f, 665.f)));

    keyGuideElements_ = { "KeyW", "KeyA", "KeyS", "KeyD", "KeyJ", "KeyK" };

    // ゲームパッド用
    AddElement(std::make_unique<KeyGuideUIElement>("PadJump", TextureId::PadJump_A, Vector2(983.f, 584.f),0.79f));
    AddElement(std::make_unique<KeyGuideUIElement>("PadStick", TextureId::PadStickAndArrow,Vector2( 981.f,652.f),0.82f));
    AddElement(std::make_unique<KeyGuideUIElement>("PadButtonX", TextureId::PadButtonX, Vector2(1100.f ,663.f),0.85f));
    AddElement(std::make_unique<KeyGuideUIElement>("PadButtonB", TextureId::PadButtonB, Vector2(1205.f, 663.f), 0.85f));
}

void UIManager::InitializeSkillIcons() {
    auto dashIcon = std::make_unique<SkillIconUIElement>("SkillIconDash", Vector2(1206.f ,580.f));
    dashIcon->AddIconTexture(TextureId::Icon_Dash);
    AddElement(std::move(dashIcon));
    skillIconElements_.push_back("SkillIconDash");

    auto boomerangIcon = std::make_unique<SkillIconUIElement>("SkillIconBoomerang", Vector2(1100.0f,580.0f));
    boomerangIcon->AddIconTexture(TextureId::Icon_BoomerangThrow);
    boomerangIcon->AddIconTexture(TextureId::Icon_BoomerangReturn);
    AddElement(std::move(boomerangIcon));
    skillIconElements_.push_back("SkillIconBoomerang");
}

void UIManager::InitializePauseAndResult() {
    auto pauseBg = std::make_unique<KeyGuideUIElement>("PauseBg", TextureId::White1x1, Vector2(640.0f, 360.0f));
    pauseBg->SetBaseColor(0x00000080);
    pauseBg->SetDrawSize(1280.0f, 720.0f);
    AddElement(std::move(pauseBg));

    auto pauseText = std::make_unique<KeyGuideUIElement>("PauseText", TextureId::PauseText, Vector2(640.0f, 360.0f));
    AddElement(std::move(pauseText));

    auto resultImage = std::make_unique<KeyGuideUIElement>("ResultImage", TextureId::ResultClear, Vector2(640.0f, 360.0f));
    AddElement(std::move(resultImage));

    pauseElements_ = { "PauseBg", "PauseText", "ResultImage" };
}

void UIManager::AddElement(std::unique_ptr<UIElement> element) {
    std::string name = element->GetName();
    elementMap_[name] = element.get();
    uiElements_.push_back(std::move(element));
}

void UIManager::RemoveElement(const std::string& name) {
    elementMap_.erase(name);
    uiElements_.erase(
        std::remove_if(uiElements_.begin(), uiElements_.end(),
            [&name](const std::unique_ptr<UIElement>& elem) {
                return elem->GetName() == name;
            }),
        uiElements_.end()
    );
}

UIElement* UIManager::GetElement(const std::string& name) {
    auto it = elementMap_.find(name);
    return (it != elementMap_.end()) ? it->second : nullptr;
}

void UIManager::Update(float dt) {
    if (isTitle_) return;

    // F1でImGui表示切替（デバッグ用）
    if (InputManager::GetInstance().TriggerKey(DIK_F1)) {
        showImGui_ = !showImGui_;
    }

    if (isPaused_) {
        if (auto* elem = GetElement("PauseText")) {
            elem->Update(dt);
        }
        return;
    }

    if (isResult_) {
        if (auto* elem = GetElement("ResultImage")) {
            elem->Update(dt);
        }
        return;
    }

    if (isGamePlay_) {
        // ゲージ更新
        for (const auto& name : gaugeElements_) {
            if (auto* elem = GetElement(name)) {
                elem->Update(dt);
            }
        }

        // キーガイド更新
        UpdateKeyGuides();

        // 入力モードに応じて表示切替
        bool isGamepad = InputManager::GetInstance().GetInputMode() == InputMode::Gamepad;
        for (const auto& name : keyGuideElements_) {
            if (auto* elem = GetElement(name)) {
                elem->SetVisible(!isGamepad);
                elem->Update(dt);
            }
        }

        // パッドUI更新
        if (auto* elem = GetElement("PadJump")) {
            elem->SetVisible(isGamepad);
            elem->Update(dt);
        }
        if (auto* elem = GetElement("PadStick")) {
            elem->SetVisible(isGamepad);
            elem->Update(dt);
        }
        if (auto* elem = GetElement("PadButtonX")) {
            elem->SetVisible(isGamepad);
            elem->Update(dt);
        }
        if (auto* elem = GetElement("PadButtonB")) {
            elem->SetVisible(isGamepad);
            elem->Update(dt);
        }
    }
}

void UIManager::UpdateKeyGuides() {
    InputManager& input = InputManager::GetInstance();

    if (input.GetInputMode() == InputMode::Gamepad) {
        UpdateGamepadUI();
    }
    else {
        UpdateKeyboardUI();
    }
}

void UIManager::UpdateGamepadUI() {
    InputManager& input = InputManager::GetInstance();

    if (input.GetPad()->Trigger(Pad::Button::A)) {
        if (auto* elem = dynamic_cast<KeyGuideUIElement*>(GetElement("PadJump"))) {
            elem->TriggerSquash();
        }
    }
    if (input.GetPad()->Trigger(Pad::Button::B)) {
        if (auto* elem = dynamic_cast<KeyGuideUIElement*>(GetElement("PadButtonB"))) {
            elem->TriggerSquash();
        }
    }
    if (input.GetPad()->Trigger(Pad::Button::X)) {
        if (auto* elem = dynamic_cast<KeyGuideUIElement*>(GetElement("PadButtonX"))) {
            elem->TriggerSquash();
        }
    }
}

void UIManager::UpdateKeyboardUI() {
    InputManager& input = InputManager::GetInstance();

    const std::vector<std::pair<int, std::string>> keyMappings = {
        {DIK_W, "KeyW"}, {DIK_A, "KeyA"}, {DIK_S, "KeyS"},
        {DIK_D, "KeyD"}, {DIK_J, "KeyJ"}, {DIK_K, "KeyK"}
    };

    for (const auto& [key, name] : keyMappings) {
        if (input.TriggerKey(key)) {
            if (auto* elem = dynamic_cast<KeyGuideUIElement*>(GetElement(name))) {
                elem->TriggerSquash();
            }
        }
    }
}

void UIManager::Draw() {
    if (isTitle_) return;

    // ゲームプレイUI
    if (isGamePlay_) {
        for (const auto& name : gaugeElements_) {
            if (auto* elem = GetElement(name)) {
                elem->Draw();
            }
        }

        for (const auto& name : keyGuideElements_) {
            if (auto* elem = GetElement(name)) {
                elem->Draw();
            }
        }

        // パッドUI
        if (auto* elem = GetElement("PadJump")) elem->Draw();
        if (auto* elem = GetElement("PadStick")) elem->Draw();
        if (auto* elem = GetElement("PadButtonX")) elem->Draw();
        if (auto* elem = GetElement("PadButtonB")) elem->Draw();

        // スキルアイコン
        for (const auto& name : skillIconElements_) {
            if (auto* elem = GetElement(name)) {
                elem->Draw();
            }
        }
    }

    // ポーズ
    if (isPaused_) {
        if (auto* elem = GetElement("PauseBg")) elem->Draw();
        if (auto* elem = GetElement("PauseText")) elem->Draw();
    }

    // リザルト
    if (isResult_) {
        if (auto* elem = GetElement("PauseBg")) elem->Draw();
        if (auto* elem = GetElement("ResultImage")) elem->Draw();
    }
}

void UIManager::DrawImGui() {
    if (!showImGui_) return;

    ImGui::Begin("UI Manager", &showImGui_);

    if (ImGui::CollapsingHeader("Global Settings")) {
        float iconPos[2] = { skillIconPos_.x, skillIconPos_.y };
        if (ImGui::DragFloat2("Skill Icon Base Pos", iconPos, 1.0f)) {
            skillIconPos_ = { iconPos[0], iconPos[1] };
        }

        float iconOffset[2] = { skillIconOffset_.x, skillIconOffset_.y };
        if (ImGui::DragFloat2("Skill Icon Offset", iconOffset, 1.0f)) {
            skillIconOffset_ = { iconOffset[0], iconOffset[1] };
        }

        float keyOffset[2] = { controllKeyOffset_.x, controllKeyOffset_.y };
        if (ImGui::DragFloat2("Control Key Offset", keyOffset, 1.0f)) {
            controllKeyOffset_ = { keyOffset[0], keyOffset[1] };
        }
    }

    if (ImGui::CollapsingHeader("UI Elements")) {
        for (auto& elem : uiElements_) {
            elem->DrawImGuiControls();
        }
    }

    ImGui::End();
}

// 既存API互換性
void UIManager::SetPlayerHP(float ratio) {
    if (auto* gauge = dynamic_cast<GaugeUIElement*>(GetElement("PlayerHP"))) {
        gauge->SetRatio(ratio);
    }
}

void UIManager::SetBossHP(float ratio) {
    if (auto* gauge = dynamic_cast<GaugeUIElement*>(GetElement("BossHP"))) {
        gauge->SetRatio(ratio);
    }
}

void UIManager::UpdateIcons(float dt, const PlayerSkillState& state) {
    if (!isGamePlay_) return;

    // ダッシュアイコン更新
    if (auto* dashIcon = dynamic_cast<SkillIconUIElement*>(GetElement("SkillIconDash"))) {
        dashIcon->UpdateState(dt, state.isDashing, state.canDash, false);
    }

    // ブーメランアイコン更新
    if (auto* boomIcon = dynamic_cast<SkillIconUIElement*>(GetElement("SkillIconBoomerang"))) {
        bool isRecalling = (state.boomerangMode == PlayerSkillState::BoomerangMode::Recalling);
        boomIcon->UpdateState(dt, isRecalling, state.canUseBoomerang, !state.canUseBoomerang);
    }
}

void UIManager::ShowResult(bool isClear) {
    isResult_ = true;
    isGamePlay_ = false;
    isGameClear_ = isClear;

    if (auto* resultImage = dynamic_cast<KeyGuideUIElement*>(GetElement("ResultImage"))) {
        if (isClear) {
            resultImage->SetTexture(TextureId::ResultClear);
            resultImage->StartPulse(1.0f, 1.2f, 0.5f, true);
        }
        else {
            resultImage->SetTexture(TextureId::ResultOver);
            resultImage->StartShake(10.0f, 0.5f);
        }
    }
}

void UIManager::TogglePause() {
    isPaused_ = !isPaused_;
}