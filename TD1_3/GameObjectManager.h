#pragma once
#include "GameObject2D.h"
#include <vector>
#include <memory>
#include <algorithm>
#include "MapData.h"
#include <utility>

enum class ObjectType {
    Player,
    Enemy,
    Bullet,
    Item,
    Effect,
    // 必要に応じて追加
};

class GameObjectManager {
private:
	int idCounter_ = 0; // オブジェクトIDカウンター

    // 全オブジェクトを所有権付きで管理
    std::vector<std::unique_ptr<GameObject2D>> objects_;

    // 追加待ちキュー（Update中の追加によるイテレータ無効化を防ぐ）
    std::vector<std::unique_ptr<GameObject2D>> pendingObjects_;


    template <typename T, typename... Args>
    T* CreateObject(GameObject2D* owner, const std::string& tag, Args&&... args) {
        // 1. メモリ確保 (T型で作成)
        auto newObj = std::make_unique<T>(std::forward<Args>(args)...);

        // 2. 基本情報のセットアップ
        newObj->SetOwner(owner);      // オーナー登録
        newObj->SetManager(this);     // マネージャー登録
        newObj->GetInfo().tag = tag;  // タグ設定
		newObj->GetInfo().id = idCounter_++; // 一意のID割り当て
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
    template <typename T, typename... Args>
    T* Spawn(GameObject2D* owner, const char* tag, Args&&... args) {
        return CreateObject<T>(owner, std::string(tag), std::forward<Args>(args)...);
    }

    // 2. Match for std::string Tag
    template <typename T, typename... Args>
    T* Spawn(GameObject2D* owner, const std::string& tag, Args&&... args) {
        return CreateObject<T>(owner, tag, std::forward<Args>(args)...);
    }

    // 3. No Tag Provided (Defaults to "Untagged")
    // This catches everything else.
    template <typename T, typename... Args>
    T* Spawn(GameObject2D* owner, Args&&... args) {
        return CreateObject<T>(owner, std::string("Untagged"), std::forward<Args>(args)...);
    }

	// MapDataから
	//void SpawnFromMapData() {
	//	const MapData& mapData = MapData::GetInstance();
	//	int width = mapData.GetWidth();
	//	int height = mapData.GetHeight();
 //       for (int y = 0; y < height; ++y) {
 //           for (int x = 0; x < width; ++x) {
 //               int tileID = mapData.GetTile(x, y, TileLayer::Object);
 //           }
 //       }
 //   }

    // ==========================================
    //  基本ループ
    // ==========================================
    void Update(float deltaTime) {
		Novice::ScreenPrintf(0, 20, "Object Count: %d", static_cast<int>(objects_.size()));

        // 1. 新規追加オブジェクトをメインリストへ統合
        for (auto& obj : pendingObjects_) {
            obj->SetManager(this);
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

    GameObject2D* GetObjectById(int id) {
        for (auto& obj : objects_) {
            if (obj->GetInfo().id == id) {
                return obj.get();
            }
        }
        return nullptr; // 見つからなかった場合
	}

    GameObject2D* GetPlayerObject() {
        for (auto& obj : objects_) {
            if (obj->GetInfo().tag == "Player") {
                return obj.get();
            }
        }
        return nullptr; // 見つからなかった場合
    }

    std::vector<GameObject2D*> GetObjectsByTag(const std::string& tag) {
        std::vector<GameObject2D*> result;
        for (auto& obj : objects_) {
            if (obj->GetInfo().tag == tag) {
                result.push_back(obj.get());
            }
        }
        return result;
    }

    std::vector<GameObject2D*> GetAllObjects() {
        std::vector<GameObject2D*> result;
        for (auto& obj : objects_) {
            result.push_back(obj.get());
        }
        return result;
	}
};