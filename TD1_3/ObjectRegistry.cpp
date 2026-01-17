#include "ObjectRegistry.h"

// 静的メンバの定義
std::vector<ObjectTypeInfo> ObjectRegistry::objectTypes_;

void ObjectRegistry::Initialize() {
    objectTypes_.clear();

    // ===================================================================
    // システムオブジェクト
    // ===================================================================
    objectTypes_.push_back({
        0,                  // id
        "WorldOrigin",      // name
        "System",           // category
        0xFF0000FF,         // color（赤）
        "WorldOrigin"       // tag
        });

    // ===================================================================
    // プレイヤー関連
    // ===================================================================
    objectTypes_.push_back({
        100,                // id
        "PlayerStart",      // name
        "Player",           // category
        0x00FF00FF,         // color（緑）
        "Player"            // tag
        });

    // ===================================================================
    // 敵キャラクター（今後追加）
    // ===================================================================
    // objectTypes_.push_back({
    //     101,
    //     "Enemy_Normal",
    //     "Enemy",
    //     0xFF00FFFF,     // color（マゼンタ）
    //     "Enemy"
    // });

    // objectTypes_.push_back({
    //     102,
    //     "Enemy_Boss",
    //     "Enemy",
    //     0xFF0088FF,     // color（濃いマゼンタ）
    //     "Boss"
    // });

    // ===================================================================
    // アイテム（今後追加）
    // ===================================================================
    // objectTypes_.push_back({
    //     200,
    //     "Item_Coin",
    //     "Item",
    //     0xFFFF00FF,     // color（黄色）
    //     "Coin"
    // });

    // objectTypes_.push_back({
    //     201,
    //     "Item_PowerUp",
    //     "Item",
    //     0xFF8800FF,     // color（オレンジ）
    //     "PowerUp"
    // });
}

const std::vector<ObjectTypeInfo>& ObjectRegistry::GetAllObjectTypes() {
    return objectTypes_;
}

const ObjectTypeInfo* ObjectRegistry::GetObjectType(int objectTypeId) {
    for (const auto& objType : objectTypes_) {
        if (objType.id == objectTypeId) {
            return &objType;
        }
    }
    return nullptr;  // 該当するIDが見つからない
}