#include "SurvivalGameManager.h"
#include "SurvivalObjects.h" // 各クラスの定義が必要
#include "Vector2.h"
#include <cassert>

SurvivalGameObjectManager::SurvivalGameObjectManager() {}
SurvivalGameObjectManager::~SurvivalGameObjectManager() { Clear(); }

void SurvivalGameObjectManager::AddObject(const std::shared_ptr<GameObject2D>& obj, const std::string& tag) {
    assert(obj && "AddObject: obj is null");
    assert(!tag.empty() && "AddObject: tag must not be empty");

    obj->GetInfo().tag = tag;

    // プロト用：テクスチャ未設定でも描画できるように white を当てる
    obj->SetTexture(TextureId::White1x1);

    objects_.push_back(obj);

    if (auto enemy = std::dynamic_pointer_cast<SurvivalEnemy>(obj)) {
        enemies_.push_back(enemy);
    }
}

void SurvivalGameObjectManager::Clear() {
    objects_.clear();
    enemies_.clear();
    player_.reset();
    debrisController_.reset();
}

void SurvivalGameObjectManager::Update(float deltaTime) {
    // 1. 全オブジェクト更新
    auto it = objects_.begin();
    while (it != objects_.end()) {
        (*it)->Update(deltaTime);

        if (!(*it)->GetInfo().isActive) {
            it = objects_.erase(it);
            continue;
        }

        ++it;
    }

    // 死んだ敵をenemies_リストからも削除
    enemies_.remove_if([](const std::shared_ptr<SurvivalEnemy>& e) {
        if (e->GetInfo().isActive == false) {
            e->Destroy();
            return true;
        }
        return false;
        });

    // 2. 衝突判定
    CheckCollisions();
}

void SurvivalGameObjectManager::Draw(const Camera2D& camera) {
    for (auto& obj : objects_) {
        obj->Draw(camera);
    }
}

void SurvivalGameObjectManager::CheckCollisions() {
    if (!player_ || !debrisController_) return;

    float playerRadius = player_->GetRadius();
    Vector2 playerPos = player_->GetPosition();

    // デブリの状態を取得
    bool isAttacking = debrisController_->IsContracting();
    //bool isDefense = debrisController_->IsExpanding();
    bool isCritical = debrisController_->IsCritical();

    // A. 敵 vs プレイヤー & デブリ
    for (auto& enemy : enemies_) {
        // 死んでる敵は無視（※実装次第）
        // if (!enemy->IsAlive()) continue;

        Vector2 enemyPos = enemy->GetPosition();
        float enemyRadius = enemy->GetRadius();

        // 1. 敵 vs プレイヤー（ゲームオーバー判定）
        float distToPlayer = Vector2::Length(enemyPos - playerPos);
        if (distToPlayer < playerRadius + enemyRadius) {
            player_->OnDamage();
            // プレイヤーを守るために少し弾く
            enemy->OnHit(0, Vector2::Normalize(enemyPos - playerPos), 500.0f);
        }

        // 2. 敵 vs デブリ（総当たりは重いので、距離で簡易フィルタリングすべきだが、今回はプロトなので総当たり）
        // DebrisControllerが持つ pieces_ にアクセスする手段が必要。
        // ※SurvivalObjects.h の DebrisController に GetPieces() を追加するか、
        // Managerが pieces を知っている必要がある。
        // ここでは「SurvivalObjects.h」に追加したと仮定して GetPieces() を呼ぶか、
        // あるいは objects_ の中から DebrisPiece を探す。
        // 一番きれいなのは DebrisController が判定メソッドを持つことだが、今回はここでやる。

        // 暫定：DebrisController はフレンドまたはアクセサ経由で pieces_ を公開してください。
        // ここでは擬似コード的に書きます。
        // const auto& pieces = debrisController_->GetPieces(); 
        // というメソッドを作ってください！

        // ↓ 仕方ないので objects_ から dynamic_cast で探す（重いが確実）
        // 実際は DebrisController::GetPieces() を推奨

        // ★修正案：前のコードで DebrisPiece も AddObject しているので、objects_ に混ざっています。
        // objects_ループだと効率悪いので、Managerに debrisPieces_ リストも持たせるのがベスト。

        // ここではロジックの意図を示すため、「Managerがデブリ片のリストを持っている」と仮定します。
        // 実装時は AddObject で DebrisPiece だった場合に debrisPieces_ にもポインタを入れてください。

        for (auto& piece : debrisController_->GetPieces()) {
            Vector2 debrisPos = piece->GetActualPosition(); // 慣性適用後の座標
            float debrisRadius = 6.0f; // 半径

            if (Vector2::Length(enemyPos - debrisPos) < enemyRadius + debrisRadius) {
                // ヒット！

                if (isAttacking) {
                    // 攻撃モード：ダメージ
                    int dmg = isCritical ? 5 : 1;
                    float power = isCritical ? 1200.0f : 400.0f;
                    Vector2 knockDir = Vector2::Normalize(enemyPos - playerPos);

                    enemy->OnHit(dmg, knockDir, power);

                } else {
                    // 防御モード：押し出し（ダメージなし、あるいは微小）
                    Vector2 pushDir = Vector2::Normalize(enemyPos - debrisPos);
                    enemy->PushBack(pushDir, 5.0f); // グイッと押し出す
                }
            }
        }
        
    }
}