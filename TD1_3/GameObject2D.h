#pragma once
#include <string>
#include <memory>

#include "Vector2.h"
#include "Matrix3x3.h"
#include "Transform2D.h"
#include "Camera2D.h"

#include "Rigidbody2D.hpp"

// 描画コンポーネント
#include "DrawComponent2D.h" 

#include "TextureManager.h"

// 必要な構造体定義
struct GameObjectInfo {
    int id = -1;
    std::string tag = "Untagged";
    bool isActive = true;
    bool isVisible = true;
};

struct Collider {
    Vector2 offset = { 0.0f, 0.0f };
    Vector2 size = { 1.0f, 1.0f };
    bool canCollide = true;
    bool isTrigger = false;
};

struct Status {
    int maxHP = 100;
    int currentHP = 100;
    int stateFlag = 0;
};

class GameObject {
protected:
    GameObject* owner_ = nullptr; // 所有者オブジェクト

    // 基本情報
    GameObjectInfo info_;

    // 座標・姿勢（SRT）
    Transform2D transform_;

    // 物理挙動（速度・加速度）
    Rigidbody2D rigidbody_;

    // 描画機能
    std::unique_ptr<DrawComponent2D> drawComponent_;

    // 当たり判定情報
    Collider collider_;

    // ゲームパラメータ
    Status status_;

public:

    GameObject() {
        // デフォルト初期化
        transform_.position = { 0.0f, 0.0f };
        transform_.scale = { 1.0f, 1.0f };
        transform_.rotation = 0.0f;

        // DrawComponent2Dの生成
		drawComponent_ = std::make_unique<DrawComponent2D>();
    }

    virtual ~GameObject() = default;

    // IDとタグを指定して初期化
    GameObject(int id, const std::string& tag) : GameObject() {
        info_.id = id;
        info_.tag = tag;
    }

    // 生成時や初期化時にセットする
    void SetOwner(GameObject* owner) { owner_ = owner; }
    GameObject* GetOwner() const { return owner_; }

	// オーナーかどうか判定
    bool IsOwnedBy(GameObject* potentialOwner) const {
        return owner_ == potentialOwner;
    }

    virtual void Initialize() {
        rigidbody_.Initialize();

        // 描画コンポーネントの初期化があれば呼ぶ
        if (drawComponent_) {
            drawComponent_->Initialize();
        }
    }

    // テクスチャの設定（DrawComponent2Dへ流す）
    void SetTexture(TextureId id) {
        if (drawComponent_) {
			drawComponent_->SetTexture(id);
        }
    }

    virtual void Update(float deltaTime) {
        if (!info_.isActive) return;

        // 1. 物理挙動の更新（速度計算）
        rigidbody_.Update(deltaTime);

        // 2. 計算された速度を座標に反映
        transform_.position += rigidbody_.GetMoveDelta(deltaTime);
        transform_.rotation += rigidbody_.GetRotationDelta(deltaTime);

        // 3. ワールド行列の更新
        transform_.CalculateWorldMatrix();

       
        // 4. 描画コンポーネント等の更新があれば呼ぶ
        if (drawComponent_) { 
			drawComponent_->SetTransform(transform_);
            drawComponent_->Update(deltaTime);}
    }

    virtual void Draw(const Camera2D& camera) {
        if (!info_.isActive || !info_.isVisible) return;

        // DrawComponent2Dを使って描画
        if (drawComponent_) {
            drawComponent_->Draw(camera);
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
    DrawComponent2D* GetDrawComponent() { return drawComponent_.get(); }

    void SetPosition(const Vector2& pos) { transform_.position = pos; }
    Vector2 GetPosition() const { return transform_.position; }
};