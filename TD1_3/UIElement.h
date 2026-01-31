#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include <string>
#include <memory>
#include <vector>

// UI要素の基底クラス
class UIElement {
public:
    UIElement(const std::string& name, const Vector2& position)
        : name_(name), position_(position), scale_(1.0f), isVisible_(true) {
    }

    virtual ~UIElement() = default;

    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;
    virtual void Draw(const Vector2& drawPos) = 0;
    virtual void DrawImGuiControls() = 0;

    const std::string& GetName() const { return name_; }
    Vector2 GetPosition() const { return position_; }
    virtual void SetPosition(const Vector2& pos) { position_ = pos; }
    float GetScale() const { return scale_; }
    void SetScale(float scale) { scale_ = scale; }
    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool visible) { isVisible_ = visible; }

protected:
    std::string name_;
    Vector2 position_;
    float scale_;
    bool isVisible_;
};

// =================================================================
// ゲージUI要素
// =================================================================
class GaugeUIElement : public UIElement {
public:
    GaugeUIElement(const std::string& name, TextureId frameId, TextureId barId,
        const Vector2& position, const Vector2& barOffset);

    void Update(float dt) override;
    void Draw() override;
    void Draw(const Vector2& drawPos) override;
    void DrawImGuiControls() override;

    void SetRatio(float ratio);
    void SetColor(uint32_t color);
    float GetRatio() const { return currentRatio_; }

private:
    DrawComponent2D frame_;
    DrawComponent2D bar_;
    DrawComponent2D ghost_;
    Vector2 barOffset_;

    float currentRatio_ = 1.0f;
    float ghostRatio_ = 1.0f;
    float ghostTimer_ = 0.0f;
};

// =================================================================
// キーガイドUI要素
// =================================================================
class KeyGuideUIElement : public UIElement {
public:
    KeyGuideUIElement(const std::string& name, TextureId texId, const Vector2& position, const float scale = 1.0f);

    void Update(float dt) override;
    void Draw() override;
    void Draw(const Vector2& drawPos) override;
    void DrawImGuiControls() override;

    void TriggerSquash();
    void SetTexture(TextureId texId);
    void StartPulse(float from, float to, float duration, bool loop);
    void StartShake(float power, float duration);
    void SetBaseColor(uint32_t color);
    void SetDrawSize(float width, float height);

private:
    std::unique_ptr<DrawComponent2D> component_;
};

// =================================================================
// スキルアイコン（内部実装クラス）
// =================================================================
class SkillIcon {
public:
    SkillIcon(const Vector2& position);

    void Update(float dt, bool useSkill, bool canUseSkill, bool isWaiting = false);
    void Draw();
	void Draw(const Vector2& drawPos);
	void Draw(const Vector2& drawPos,Vector2& drawScale);

    void AddIconTexture(TextureId texId, int iconId = 0);
    void SetPosition(const Vector2& pos);
    Vector2 GetPosition() const { return iconPos_; }

private:
    Vector2 iconPos_;
    std::pair<std::vector<std::unique_ptr<DrawComponent2D>>, int> icon_; // アイコンリストと現在のインデックス
    bool prevUseSkill_ = false; // 前フレームの使用状態
};

// =================================================================
// スキルアイコンUI要素
// =================================================================
class SkillIconUIElement : public UIElement {
public:
    SkillIconUIElement(const std::string& name, const Vector2& position);

    void Update(float dt) override;
    void Draw() override;
	void Draw(const Vector2& drawPos) override;
    void DrawImGuiControls() override;
    void SetPosition(const Vector2& pos) override;

    void AddIconTexture(TextureId texId);
    void UpdateState(float dt, bool useSkill, bool canUseSkill, bool isWaiting = false);

private:
    std::unique_ptr<SkillIcon> skillIcon_;
};