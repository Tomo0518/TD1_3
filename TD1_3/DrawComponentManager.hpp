#pragma once
#include "DrawComponent2D.h"
#include <unordered_map>
#include <string>

class DrawComponentManager {
public:
    DrawComponentManager() = default;
    ~DrawComponentManager() {
        // 全てのコンポーネントを自動削除
        for (auto& pair : components_) {
            delete pair.second;
        }
        components_.clear();
    }

    // コピー禁止
    DrawComponentManager(const DrawComponentManager&) = delete;
    DrawComponentManager& operator=(const DrawComponentManager&) = delete;

    /// <summary>
    /// コンポーネントを登録
    /// </summary>
    void RegisterComponent(const std::string& name, DrawComponent2D* component) {
        if (component) {
            components_[name] = component;
            if (!activeComponent_) {
                activeComponent_ = component;
                activeComponentName_ = name;
            }
        }
    }

    /// <summary>
    /// アクティブなコンポーネントを変更（エフェクト状態を自動引き継ぎ）
    /// </summary>
    bool ChangeComponent(const std::string& name) {
        auto it = components_.find(name);
        if (it != components_.end() && activeComponent_ != it->second) {
            // エフェクト状態を引き継ぐ
            TransferEffectState(activeComponent_, it->second);
            
            activeComponent_ = it->second;
            activeComponentName_ = name;
            activeComponent_->PlayAnimation();
            return true;
        }
        return false;
    }

    /// <summary>
    /// 現在のアクティブコンポーネントを取得
    /// </summary>
    DrawComponent2D* GetActiveComponent() const {
        return activeComponent_;
    }

    /// <summary>
    /// 名前でコンポーネントを取得
    /// </summary>
    DrawComponent2D* GetComponent(const std::string& name) const {
        auto it = components_.find(name);
        return (it != components_.end()) ? it->second : nullptr;
    }

    /// <summary>
    /// 全コンポーネントに共通設定を適用
    /// </summary>
    void SetFlipX(bool flipX) {
        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->SetFlipX(flipX);
            }
        }
    }

    void SetTransform(const Transform2D& transform) {
        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->SetTransform(transform);
            }
        }
    }

    void SetPosition(const Vector2& position) {
        if (activeComponent_) {
            activeComponent_->SetPosition(position);
        }
    }

    /// <summary>
    /// エフェクト開始（全コンポーネントに適用）
    /// </summary>
    void StartHitEffect() {
        effectState_.isHitEffectActive = true;
        effectState_.hitEffectTimer = 0.0f;
        
        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->StartHitEffect();
            }
        }
    }

    void StartShake(float intensity, float duration) {
        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->StartShake(intensity, duration);
            }
        }
	}

    void StartPulse(float minScale, float maxScale, float duration) {
        effectState_.isPulseActive = true;
        effectState_.pulseMinScale = minScale;
        effectState_.pulseMaxScale = maxScale;
        effectState_.pulseDuration = duration;
        
        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->StartPulse(minScale, maxScale, duration);
            }
        }
    }

    void StartSquash(const Vector2& targetScale, float duration) {
        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->StartSquash(targetScale, duration);
            }
        }
	}
    void StartFlashBlink(unsigned int color, int count, float duration,BlendMode blend, unsigned int layer = 1) {
        effectState_.isFlashBlinkActive = true;
        effectState_.flashColor = color;
        effectState_.flashCount = count;
        effectState_.flashDuration = duration;
        effectState_.flashLayer = layer;
        effectState_.flashTimer = 0.0f;
        effectState_.isFlashOn = true;
        effectState_.blendMode = blend;

        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->StartFlashBlink(color, count, duration, blend, layer);
            }
        }
    }

    void StopFlashBlink() {
        effectState_.isFlashBlinkActive = false;

        for (auto& pair : components_) {
            if (pair.second) {
                pair.second->StopFlashBlink();
            }
        }
    }

    /// <summary>
    /// アクティブコンポーネントの更新
    /// </summary>


    void Update(float deltaTime) {
        if (activeComponent_) {
            activeComponent_->Update(deltaTime);

            // アクティブコンポーネントからフラッシュの進行状況を取得して同期
            if (effectState_.isFlashBlinkActive) {
                if (activeComponent_->IsFlashBlinking()) {
                    effectState_.flashCount = activeComponent_->GetFlashRemainingCount();
                    effectState_.flashTimer = activeComponent_->GetFlashTimer();
                    effectState_.isFlashOn = activeComponent_->IsFlashCurrentlyOn();
                }
                else {
                    // アクティブコンポーネントのフラッシュが終了したら全て停止
                    effectState_.isFlashBlinkActive = false;
                }
            }
        }
    }

    /// <summary>
    /// アクティブコンポーネントの描画
    /// </summary>
    void Draw(const Camera2D& camera) {
        if (activeComponent_) {
            activeComponent_->Draw(camera);
        }
    }

    /// <summary>
    /// アクティブなコンポーネント名を取得
    /// </summary>
    const std::string& GetActiveComponentName() const {
        return activeComponentName_;
    }

private:
    /// <summary>
    /// エフェクト状態を別のコンポーネントに転送
    /// </summary>
    void TransferEffectState(DrawComponent2D* from, DrawComponent2D* to) {
        if (!from || !to) return;

        if (effectState_.isHitEffectActive) {
            to->StartHitEffect();
        }

        if (effectState_.isPulseActive) {
            to->StartPulse(
                effectState_.pulseMinScale,
                effectState_.pulseMaxScale,
                effectState_.pulseDuration
            );
        }

       // フラッシュ点滅の転送
        if (effectState_.isFlashBlinkActive && effectState_.flashCount > 0) {
            // タイマーと状態を含めて完全に引き継ぐ
            to->SetFlashBlinkState(
                effectState_.flashColor,
                effectState_.flashCount,
                effectState_.flashDuration,
                effectState_.flashLayer,
                effectState_.flashTimer,
                effectState_.isFlashOn
            );
        }
    }

    struct EffectState {
        bool isHitEffectActive = false;
        float hitEffectTimer = 0.0f;

        bool isPulseActive = false;
        float pulseMinScale = 1.0f;
        float pulseMaxScale = 1.0f;
        float pulseDuration = 0.0f;

        // フラッシュ点滅エフェクト
        bool isFlashBlinkActive = false;
        unsigned int flashColor = 0xFFFFFFFF;
        int flashCount = 0;          // 残り回数（動的に更新される）
        float flashDuration = 0.0f;
        unsigned int flashLayer = 1;
        float flashTimer = 0.0f;
        bool isFlashOn = false;
        BlendMode blendMode;
    };

    std::unordered_map<std::string, DrawComponent2D*> components_;
    DrawComponent2D* activeComponent_ = nullptr;
    std::string activeComponentName_;
    EffectState effectState_;
};