#pragma once
#include <string>
#include <memory>

#include "Vector2.h"
#include "Matrix3x3.h"
#include "Transform2D.h"
#include "Camera2D.h"

#include "Rigidbody2D.hpp"
#include "DrawComponent2D.h" 
#include "TextureManager.h"
#include "SoundManager.h"

class GameObjectManager; // 前方宣言

// 必要な構造体定義
struct GameObjectInfo {
    int id = -1;
    std::string tag = "Untagged";
    bool isActive = true;
    bool isVisible = true;
};

struct Collider {
    Vector2 offset = { 0.0f, 0.0f };
    Vector2 size = { 64.0f, 64.0f };
    bool canCollide = true;
    bool isTrigger = false;
};

struct Status {
    int maxHP = 100;
    int currentHP = 100;
    int stateFlag = 0;
};

class GameObject2D {
protected:
    GameObject2D* owner_ = nullptr; // 所有者オブジェクト

    // 基本情報
    GameObjectInfo info_;

    // 座標・姿勢（SRT）
    Transform2D transform_;

    // 物理挙動（速度・加速度）
    Rigidbody2D rigidbody_;

    // 描画機能
    DrawComponent2D* drawComp_;

    // 当たり判定情報
    Collider collider_;

    // ゲームパラメータ
    Status status_;

    // 生成・削除を依頼するマネージャー
    GameObjectManager* manager_ = nullptr;

    // 死亡フラグ（trueになるとマネージャーが削除する）
    bool isDead_ = false;

public:

    GameObject2D() {
        // デフォルト初期化
        transform_.translate = { 0.0f, 0.0f };
        transform_.scale = { 1.0f, 1.0f };
        transform_.rotation = 0.0f;

        // DrawComponent2Dの生成
		drawComp_ = new DrawComponent2D();
    }

    virtual ~GameObject2D() = default;

    // IDとタグを指定して初期化
    GameObject2D(int id, const std::string& tag) : GameObject2D() {
        info_.id = id;
        info_.tag = tag;
    }

    // 生成時や初期化時にセットする
    void SetOwner(GameObject2D* owner) { owner_ = owner; }
    GameObject2D* GetOwner() const { return owner_; }

	// オーナーかどうか判定
    bool IsOwnedBy(GameObject2D* potentialOwner) const {
        return owner_ == potentialOwner;
    }

	// 初期化処理
    virtual void Initialize() {
        rigidbody_.Initialize();

        // 描画コンポーネントの初期化があれば呼ぶ
        if (drawComp_) {
            drawComp_->Initialize();
        }
    }

    // テクスチャの設定（DrawComponent2Dへ流す）
    void SetTexture(TextureId id) {
        if (drawComp_) {
			drawComp_->SetTexture(id);
        }
    }

    virtual void Update(float deltaTime = 60.0f) {
        if (!info_.isActive) return;

        // 1. 物理挙動の更新（速度計算）
        rigidbody_.Update(deltaTime);

        // 2. 計算された速度を座標に反映
        transform_.translate += rigidbody_.GetMoveDelta(deltaTime);
        transform_.rotation += rigidbody_.GetRotationDelta(deltaTime);

        // 3. ワールド行列の更新
        transform_.CalculateWorldMatrix();

       
        // 4. 描画コンポーネント等の更新があれば呼ぶ
        if (drawComp_) { 
			drawComp_->SetTransform(transform_);
            drawComp_->Update(deltaTime);
        }
    }

    virtual void Draw(const Camera2D& camera) {
        if (!info_.isActive || !info_.isVisible) return;

        // DrawComponent2Dを使って描画
        if (drawComp_) {
            drawComp_->Draw(camera);
        }
    }

    // 力を加える
    void AddForce(const Vector2& force) { rigidbody_.AddForce(force); }

    // --- Getters / Setters ---

    GameObjectInfo& GetInfo() { return info_; }
    Transform2D& GetTransform() { return transform_; }
    Rigidbody2D& GetRigidbody() { return rigidbody_; }
    Collider& GetCollider() { return collider_; }
    Status& GetStatus() { return status_; }
	

    // 描画コンポーネントへのアクセス（細かい設定用）
    DrawComponent2D* GetDrawComponent() { return drawComp_; }

    void SetPosition(const Vector2& pos) { transform_.translate = pos; }
    Vector2 GetPosition() const { return transform_.translate; }

    // --- マネージャー連携用セッター ---
    void SetManager(GameObjectManager* manager) { manager_ = manager; }
    GameObjectManager* GetGameObjectManager() { return manager_; }

    // 自分を殺す（リストから削除依頼）
    void Destroy() { isDead_ = true; }
    bool IsDead() const { return isDead_; }

	// collsion detection
    virtual int OnCollision(GameObject2D* other) {
        // デフォルトでは何もしない
        other;
        return 0;
    }

    // damaged
    virtual void OnDamaged(int damage) {
		if (damage < 0) return; // 負のダメージは無効
        status_.currentHP -= damage;
        if (status_.currentHP <= 0) {
            status_.currentHP = 0;
            Destroy();
        }
	}

	// healed
    virtual void OnHealed(int heal) {
		if (heal < 0) return; // 負の回復は無効
        status_.currentHP += heal;
        if (status_.currentHP > status_.maxHP) {
            status_.currentHP = status_.maxHP;
        }
	}

    void ShowDebugGuiWindow() {
        if (!this) return;

#ifdef _DEBUG
        // デバッグ用GUIウィンドウ表示
        if (ImGui::TreeNode(("GameObject ID: " + std::to_string(info_.id)).c_str())) {
            ImGui::Text(("Tag: " + info_.tag).c_str());
            ImGui::Text("Active: %s", info_.isActive ? "Yes" : "No");
            ImGui::Text("Visible: %s", info_.isVisible ? "Yes" : "No");
            ImGui::Separator();
            ImGui::Text("Transform:");
            ImGui::Text("Position: (%.1f, %.1f)", transform_.translate.x, transform_.translate.y);
            ImGui::Text("Scale: (%.1f, %.1f)", transform_.scale.x, transform_.scale.y);
            ImGui::Text("Rotation (radians): %.2f", transform_.rotation);
            ImGui::Separator();
            ImGui::Text("Status:");
            ImGui::Text("Max HP: %d", status_.maxHP);
            ImGui::Text("Current HP: %d", status_.currentHP);
            ImGui::Text("State Flag: %d", status_.stateFlag);

            // Rigidbody
            ImGui::Separator();
            ImGui::Text("Rigidbody:");
            ImGui::Text("Velocity: (%.1f, %.1f)", rigidbody_.velocity.x, rigidbody_.velocity.y);
            ImGui::Text("Acceleration: (%.1f, %.1f)", rigidbody_.acceleration.x, rigidbody_.acceleration.y);
            ImGui::Text("Deceleration: (%.1f, %.1f)", rigidbody_.deceleration.x, rigidbody_.deceleration.y);

			ImGui::TreePop();
        }
#endif
    }
};