#include "OrbitSystem.h"
#include <Novice.h>
#include <cmath>

// 定数定義
static const float kPI = 3.14159265f;

OrbitSystem::OrbitSystem() {
    Initialize();
}

void OrbitSystem::Initialize() {
    // バネのパラメータ設定（少し遅れてついてくる"柔らかい"設定）
    nucleus_.stiffness = 100.0f;
    nucleus_.damping = 12.0f;
    nucleus_.mass = 1.5f;

    // 描画コンポーネントの作成
    starDrawer_ = std::make_unique<DrawComponent2D>(TextureId::Star);
    starDrawer_->SetScale(1.0f,1.0f); // 星のサイズ調整

    nucleus_.position = { 0, 0 };
    nucleus_.velocity = { 0, 0 };
    nucleus_.target = { 0, 0 };
}

void OrbitSystem::Update(float deltaTime, const Vector2& playerPos, int starCount, const std::vector<Boomerang*>& boomerangs) {
    currentDrawCount_ = starCount;
    if (currentDrawCount_ <= 0) return;

	float usedt = deltaTime / 60.0f;

    // 初回更新時、まだ位置が設定されていない場合は即座にプレイヤー位置にワープ
    if (nucleus_.position.x == 0.0f && nucleus_.position.y == 0.0f &&
        nucleus_.velocity.x == 0.0f && nucleus_.velocity.y == 0.0f) {
        nucleus_.position = playerPos;
        nucleus_.velocity = { 0, 0 };
        nucleus_.target = playerPos;
    }

    // 1. 追従ターゲットの決定
    Vector2 targetPos = playerPos;
    bool foundActiveBoomerang = false;

    // 投げている最中のブーメランがあれば、そちらをターゲットにする（視線誘導）
    for (auto* boom : boomerangs) {
        if (boom && !boom->IsIdle() && !boom->IsTemporary()) {
            targetPos = boom->GetTransform().translate;
            foundActiveBoomerang = true;
            break;
        }
    }

    // ターゲットが切り替わった瞬間の挙動調整（面白さポイント）
    if (foundActiveBoomerang && !isTrackingBoomerang_) {
        // プレイヤー → ブーメランへの切り替わり時
        // 勢いよく飛び出すために、少し反動をつけるなどの調整も可能
        nucleus_.stiffness = 200.0f; // ブーメランへは素早く飛んでいく
    }
    else if (!foundActiveBoomerang && isTrackingBoomerang_) {
        // ブーメラン → プレイヤーへの戻り時
        nucleus_.stiffness = 80.0f; // 戻るときは少しふわっと戻る
    }
    isTrackingBoomerang_ = foundActiveBoomerang;

    // 2. 物理バネ更新
    nucleus_.target = targetPos;
    nucleus_.Update(usedt);

    // 3. 回転アニメーション更新
    // 星の数が多いほど速く回すと"エネルギー充填感"が出る
    float speedMultiplier = 1.0f + (currentDrawCount_ * 0.1f);
    rotationAngle_ += rotationSpeed_ * speedMultiplier * usedt;

    if (rotationAngle_ > 2 * kPI) {
        rotationAngle_ -= 2 * kPI;
    }
}

void OrbitSystem::Draw(const Camera2D& camera) {
    if (currentDrawCount_ <= 0) return;

    // 星の数だけ配置を計算して描画
    float angleStep = (2 * kPI) / currentDrawCount_;

    for (int i = 0; i < currentDrawCount_; ++i) {
        float currentAngle = rotationAngle_ + (angleStep * i);

        // 楕円軌道計算 (真円より少し横長にすると遠近感が出る)
        float offsetX = cosf(currentAngle) * orbitRadius_;
        float offsetY = sinf(currentAngle) * (orbitRadius_ * 0.8f); // Y軸を少し潰す

        Vector2 starPos;
        starPos.x = nucleus_.position.x + offsetX;
        starPos.y = nucleus_.position.y + offsetY;

        // 描画実行
        // Usagi本体のDrawManagerを使わずに直接描画する場合
        // カメラ座標はここでは引数として取らず、Noviceの機能かGlobalなカメラ情報を利用する想定
        // ※必要であれば引数にCameraクラスを追加してください

        starDrawer_->SetPosition(starPos);

        // 奥行き表現（後ろにある星は暗く/小さくする）
        bool isBack = sinf(currentAngle) < 0;
        if (isBack) {
            starDrawer_->SetScale(0.8f, 0.8f);
        }
        else {
            starDrawer_->SetScale(1.0f,1.0f);
            starDrawer_->StartFlashBlink(WHITE, 3, 0.4f, BlendMode::kBlendModeAdd, 1);
        }

        starDrawer_->Draw(camera); 
    }
}