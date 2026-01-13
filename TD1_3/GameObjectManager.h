#pragma once
#include "GameObject2D.h"
#include <vector>
#include <memory>
#include <algorithm>

class GameObjectManager {
private:
    // 全オブジェクトを所有権付きで管理
    std::vector<std::unique_ptr<GameObject2D>> objects_;

    // 追加待ちキュー（Update中の追加によるイテレータ無効化を防ぐ）
    std::vector<std::unique_ptr<GameObject2D>> pendingObjects_;

public:
    // ==========================================
    //  生成メソッド (Spawn)
    // ==========================================
    /// <summary>
    /// 任意のGameObject派生クラスを生成し、オーナー設定まで行う
    /// </summary>
    /// <typeparam name="T">生成したいクラス (例: Bullet)</typeparam>
    /// <param name="owner">生成主 (例: this)</param>
    /// <param name="tag">タグ (任意)</param>
    /// <returns>生成されたオブジェクトのポインタ</returns>
    template <typename T>
    T* Spawn(GameObject2D* owner, const std::string& tag = "Untagged") {
        // 1. メモリ確保 (T型で作成)
        auto newObj = std::make_unique<T>();

        // 2. 基本情報のセットアップ
        newObj->SetOwner(owner);      // オーナー登録
        newObj->SetManager(this);     // マネージャー登録
        newObj->GetInfo().tag = tag;  // タグ設定
        newObj->Initialize();         // 初期化呼び出し

        // 3. 呼び出し元に返すための生ポインタを取得
        T* rawPtr = newObj.get();

        // 4. リストへ追加（Update中はpendingに入れ、後でマージする等の処理推奨）
        // ここでは簡易的に直接追加か、次フレーム追加用リストに入れる
        pendingObjects_.push_back(std::move(newObj));

        // 5. ポインタを返す！
        // これにより「Boss」は「今産まれたEnemy」を即座に知ることができる
        return rawPtr;
    }

    // ==========================================
    //  基本ループ
    // ==========================================
    void Update(float deltaTime) {
        // 1. 新規追加オブジェクトをメインリストへ統合
        for (auto& obj : pendingObjects_) {
            objects_.push_back(std::move(obj));
        }
        pendingObjects_.clear();

        // 2. 全オブジェクト更新
        for (auto& obj : objects_) {
            obj->Update(deltaTime);
        }

        // 3. 死亡フラグが立ったオブジェクトを削除
        objects_.erase(
            std::remove_if(objects_.begin(), objects_.end(),
                [](const std::unique_ptr<GameObject2D>& obj) {
                    return obj->IsDead();
                }
            ),
            objects_.end()
        );
    }

    void Draw(const Camera2D& camera) {
        for (auto& obj : objects_) {
            obj->Draw(camera);
        }
    }

    // 全削除（シーン切り替え時など）
    void Clear() {
        objects_.clear();
        pendingObjects_.clear();
    }
};