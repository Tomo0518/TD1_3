#pragma once
#include "UIElement.h"
#include "InputManager.h"
#include <vector>
#include <memory>
#include <unordered_map>

struct PlayerSkillState {
	bool isDashing = false;
	bool canDash = false;

	enum class BoomerangMode {
		Idle,       // 待機中
		Throwing,   // 投げる準備/投げられる
		Recalling   // 回収できる状態
	};
	BoomerangMode boomerangMode = BoomerangMode::Idle;
	bool canUseBoomerang = false;
};



class UIManager {
public:
    static UIManager& GetInstance() {
        static UIManager instance;
        return instance;
    }

    void Initialize();
    void Update(float dt);
    void Draw();
    void DrawImGui(); // ImGui調整UI

    // UI要素の管理
    void AddElement(std::unique_ptr<UIElement> element);
    void RemoveElement(const std::string& name);
    UIElement* GetElement(const std::string& name);

    // 既存のAPI互換性のため
    void SetPlayerHP(float ratio);
    void SetBossHP(float ratio);
    void UpdateIcons(float dt, const PlayerSkillState& state);
    void ShowResult(bool isClear);
    void TogglePause();

    // 状態管理
    void SetGamePlay(bool flag) { isGamePlay_ = flag; }
    void SetTitle(bool flag) { isTitle_ = flag; }
    bool IsGamePlay() const { return isGamePlay_; }
    bool IsPaused() const { return isPaused_; }

private:
    UIManager() = default;
    ~UIManager() = default;
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    void InitializeGauges();
    void InitializeKeyGuides();
    void InitializeSkillIcons();
    void InitializePauseAndResult();

    void UpdateKeyGuides();
    void UpdateGamepadUI();
    void UpdateKeyboardUI();

    // UI要素コンテナ
    std::vector<std::unique_ptr<UIElement>> uiElements_;
    std::unordered_map<std::string, UIElement*> elementMap_;

    // グループ管理（カテゴリ別に描画順や表示切替を制御）
    std::vector<std::string> gaugeElements_;
    std::vector<std::string> keyGuideElements_;
    std::vector<std::string> skillIconElements_;
    std::vector<std::string> pauseElements_;

    // 状態フラグ
    bool isGamePlay_ = false;
    bool isTitle_ = false;
    bool isPaused_ = false;
    bool isResult_ = false;
    bool isGameClear_ = false;

    // ImGui設定
    bool showImGui_ = true;

    // 設定可能な定数
    Vector2 skillIconPos_ = { 1150.0f, 580.0f };
    Vector2 skillIconOffset_ = { 80.0f, 0.0f };
    Vector2 controllKeyOffset_ = { 0.0f, 70.0f };
};