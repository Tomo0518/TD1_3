#include "SurvivalObjects.h"
#include "SurvivalGameManager.h" 
#include "Novice.h"
#include "WindowSize.h"
#include <cmath>
#include <algorithm>

// 便利な定数
const float PI = 3.14159265f;

// 汎用ヘルパー：角度正規化
float NormalizeAngleObj(float angle) {
    while (angle > PI) angle -= 2.0f * PI;
    while (angle < -PI) angle += 2.0f * PI;
    return angle;
}

// ==========================================
// SurvivalPlayer (Core)
// ==========================================
SurvivalPlayer::SurvivalPlayer(InputManager* input) : input_(input) {
    // 描画コンポーネント設定
    //drawComp_ = AddComponent<DrawComponent2D>();
    // 画像未設定でも動くように白い矩形(white1x1)や塗りつぶしを想定

    drawComp_->SetDrawSize(radius_ * 2.0f, radius_ * 2.0f);
    drawComp_->SetAnchorPoint({ 0.5f, 0.5f });
    drawComp_->SetBaseColor(0xAAAAFFFF); // 青白く光るコア

    // 初期位置
    transform_.translate = { kWindowWidth / 2.0f, kWindowHeight / 2.0f };
}

void SurvivalPlayer::Update(float dt) {
    Vector2 moveDir = { 0,0 };

    // キー入力
    if (input_->PressKey(DIK_W)) moveDir.y -= 1.0f;
    if (input_->PressKey(DIK_S)) moveDir.y += 1.0f;
    if (input_->PressKey(DIK_A)) moveDir.x -= 1.0f;
    if (input_->PressKey(DIK_D)) moveDir.x += 1.0f;

    // パッド入力
    if (input_->GetInputMode() == InputMode::Gamepad) {
        moveDir = input_->GetPad()->GetLeftStick();
    }

    // 移動処理
    if (Vector2::Length(moveDir) > 0.0f) {
        moveDir = Vector2::Normalize(moveDir);
        transform_.translate += moveDir * speed_ * dt;
    }

    // 画面外に出ないようにクランプ
    transform_.translate.x = std::clamp(transform_.translate.x, radius_, kWindowWidth - radius_);
    transform_.translate.y = std::clamp(transform_.translate.y, radius_, kWindowHeight - radius_);

    // 無敵時間の点滅処理
    if (invincibilityTimer_ > 0.0f) {
        invincibilityTimer_ -= dt;
        if ((int)(invincibilityTimer_ * 20) % 2 == 0) {
            drawComp_->SetBaseColor(0x8888FFFF);
        }
        else {
            drawComp_->SetBaseColor(0xAAAAFFFF);
        }
    }
    else {
        drawComp_->SetBaseColor(0xAAAAFFFF);
    }

    // 親クラス更新（コンポーネント更新）
    GameObject2D::Update(dt);
}

void SurvivalPlayer::OnDamage() {
    if (invincibilityTimer_ > 0.0f) return;
    hp_--;
    invincibilityTimer_ = 1.0f;

    // 被弾演出：激しくシェイク＆赤フラッシュ
    drawComp_->StartShake(5.0f, 0.5f);
    //drawComp_->StartFlash({ 1.0f, 0.0f, 0.0f, 1.0f }, 0.2f);
}

// ==========================================
// SurvivalEnemy
// ==========================================
SurvivalEnemy::SurvivalEnemy(Vector2 startPos, EnemyType type, std::shared_ptr<SurvivalPlayer> target)
    : type_(type), target_(target) {

    transform_.translate = startPos;

    // タイプ別のパラメータ設定
    if (type_ == EnemyType::Tank) {
        hp_ = 15;
        radius_ = 24.0f;
       // drawComp_ = AddComponent<DrawComponent2D>();
        drawComp_->SetBaseColor(0x882222FF); // 濃い赤
    }
    else {
        hp_ = 2;
        radius_ = 12.0f;
       // drawComp_ = AddComponent<DrawComponent2D>();
        drawComp_->SetBaseColor(0xFF4444FF); // 明るい赤
    }

    drawComp_->SetDrawSize(radius_ * 2, radius_ * 2);
    drawComp_->SetAnchorPoint({ 0.5f, 0.5f });
}

void SurvivalEnemy::Update(float dt) {
    if (isDead_) return;

    if (hitInvincibility_ > 0.0f) hitInvincibility_ -= dt;

    if (knockbackDuration_ > 0.0f) {
        // --- ノックバック中 ---
        knockbackDuration_ -= dt;
        transform_.translate += knockbackVel_ * dt;
        knockbackVel_ *= 0.9f; // 摩擦で減速
        drawComp_->SetBaseColor(0xFFFFFFFF); // 白飛び演出
    }
    else {
        // --- 通常AI（追尾） ---
        drawComp_->SetBaseColor((type_ == EnemyType::Tank) ? 0x882222FF : 0xFF4444FF);

        if (target_) {
            Vector2 toPlayer = target_->GetPosition() - transform_.translate;
            if (Vector2::Length(toPlayer) > 1.0f) {
                Vector2 dir = Vector2::Normalize(toPlayer);
                float speed = (type_ == EnemyType::Tank) ? 40.0f : 100.0f;
                transform_.translate += dir * speed * dt;
            }
        }
    }

    GameObject2D::Update(dt);
}

void SurvivalEnemy::OnHit(int damage, Vector2 knockbackDir, float knockbackPower) {
    // 無敵時間中はスキップ（ただし多段ヒット防止用なので極短）
    if (hitInvincibility_ > 0.0f) return;

    hp_ -= damage;

    // ノックバック適用
    knockbackVel_ = knockbackDir * knockbackPower;
    knockbackDuration_ = 0.2f + (knockbackPower / 2000.0f); // 強い攻撃ほど長く飛ぶ

    // ヒット演出：つぶれるアニメーション
    drawComp_->StartSquash({ 1.3f, 0.7f }, 0.1f);

    if (hp_ <= 0) {
        isDead_ = true;
        GetInfo().isActive = false;
        // 死亡エフェクトがあればここで再生（ParticleManagerなどに依頼）
    }
}

void SurvivalEnemy::PushBack(Vector2 dir, float dist) {
    // 物理的に押し出される処理（ダメージなし）
    transform_.translate += dir * dist;
}

// ==========================================
// DebrisPiece (慣性を持つがれき)
// ==========================================
DebrisPiece::DebrisPiece(int index, int totalCount, std::shared_ptr<SurvivalPlayer> anchor)
    : index_(index), totalCount_(totalCount), anchor_(anchor) {

   // drawComp_ = AddComponent<DrawComponent2D>();
    drawComp_->SetDrawSize(12.0f, 12.0f); // デフォルトサイズ
    drawComp_->SetAnchorPoint({ 0.5f, 0.5f });

    // 個体差の生成（ノイズ）
    angleOffset_ = (float)index / (float)totalCount * 2.0f * PI;
    distNoise_ = (float)(rand() % 40 - 20);
    selfRotSpeed_ = (float)(rand() % 100 - 50) / 10.0f;
}

void DebrisPiece::SetStateInfo(float currentRadius, float rotationAngle, float expandSpeedScale) {
	expandSpeedScale; // 未使用
    // Controllerから毎フレーム呼ばれる「目標状態」の設定

    // 目標とする位置（TargetPos）を計算
    // 発散中ならspeedScale（速度ムラ）を適用して形を崩す
    float r = currentRadius + distNoise_;

    // 本来あるべき角度
    float a = angleOffset_ + rotationAngle;

    // アンカー（プレイヤー）中心からのオフセット
    Vector2 offset = { cosf(a) * r, sinf(a) * r };

    // 目標座標更新
    targetPos_ = anchor_->GetPosition() + offset;

    // 自転更新
    currentSelfRot_ += selfRotSpeed_ * 0.016f; // dt簡易
    drawComp_->SetRotation(a + currentSelfRot_);

    // 色の更新
    // ※実際はStateを見て変えたいが、ここでは簡易的に緑固定
    // コントローラー側で一括設定しても良い
}

void DebrisPiece::Update(float dt) {
    // === 慣性変形のキモ ===
    // targetPos_（本来あるべき円周上の点）に対して、
    // transform_.translate（実際のがれき）を遅れて追従させる。

    // 追従係数 (Lerp Factor)
    // 1.0f なら遅れなし。小さいほどネバネバする。
    // 発散・収縮中はキビキビ動いてほしいので係数を変えるなどの工夫も可
    float lerpFactor = 10.0f * dt;

    // 簡易Lerp
    Vector2 diff = targetPos_ - transform_.translate;
    transform_.translate += diff * lerpFactor;

    // あまりに離れすぎたら強制ワープ（テレポート対策）
    if (Vector2::Length(diff) > 500.0f) {
        transform_.translate = targetPos_;
    }

    GameObject2D::Update(dt);
}

// ==========================================
// DebrisController (司令塔)
// ==========================================
DebrisController::DebrisController(SurvivalGameObjectManager* manager, InputManager* input, std::shared_ptr<SurvivalPlayer> anchor)
    : manager_(manager), input_(input), anchor_(anchor) {

    currentRadius_ = minRadius_;

    // 64個のがれきを生成してマネージャーに登録
    int count = 64;
    for (int i = 0; i < count; i++) {
        auto piece = std::make_shared<DebrisPiece>(i, count, anchor);
        pieces_.push_back(piece);
        manager_->AddObject(piece, "DebrisPiece"); // 描画・更新のためにマネージャーへ渡す
    }
}

void DebrisController::Update(float dt) {
    // 回転
    // 半径に応じて回転速度を変える（小さい＝速い、大きい＝遅い）
    float t = (currentRadius_ - minRadius_) / (maxRadius_ - minRadius_);
    t = std::clamp(t, 0.0f, 1.0f);
    currentRotationSpeed_ = 8.0f * (1.0f - t) + 1.0f * t;

    if (state_ == State::Contracting) currentRotationSpeed_ = 15.0f; // 攻撃中は超高速

    rotationAngle_ += currentRotationSpeed_ * dt;

    // 入力
    bool isSpace = input_->PressKey(DIK_SPACE) || input_->GetPad()->Press(Pad::Button::A);

    // ステートマシン
    switch (state_) {
    case State::Idle:
        currentRadius_ = minRadius_;
        if (isSpace) {
            state_ = State::Expanding;
            isCritical_ = false;
        }
        break;

    case State::Expanding:
        currentRadius_ += 400.0f * dt; // 展開速度
        if (currentRadius_ >= maxRadius_) {
            currentRadius_ = maxRadius_;
            state_ = State::WaitMax;
            isCritical_ = true; // チャージ完了
        }
        if (!isSpace) state_ = State::Contracting;
        break;

    case State::WaitMax:
        currentRadius_ = maxRadius_;
        if (!isSpace) state_ = State::Contracting;
        break;

    case State::Contracting:
    {
        float speed = 1500.0f; // 収縮速度
        if (isCritical_) speed *= 1.2f;

        currentRadius_ -= speed * dt;
        if (currentRadius_ <= minRadius_) {
            currentRadius_ = minRadius_;
            state_ = State::Cooldown;
            cooldownTimer_ = isCritical_ ? 0.8f : 0.15f;
        }
    }
    break;

    case State::Cooldown:
        cooldownTimer_ -= dt;
        if (cooldownTimer_ <= 0.0f) state_ = State::Idle;
        break;
    }

    // 子機（Pieces）に指令を出す
    // 色の計算
    unsigned int color = 0x00FF00FF; // Safe
    if (state_ == State::Expanding) color = 0xFFFF00FF;
    if (state_ == State::WaitMax) color = isCritical_ ? 0xFFFFFFFF : 0xFFFF00FF;
    if (state_ == State::Contracting) color = 0xFF0000FF; // Attack

    for (auto& p : pieces_) {
        // 各ピースに目標位置を計算させる
        // Expanding中は少しバラつきを持たせるためのScaleを渡すなどの拡張もここ
        p->SetStateInfo(currentRadius_, rotationAngle_, 1.0f);
      //  p->GetComponent<DrawComponent2D>()->SetBaseColor(color);
    }
}

bool DebrisController::IsExpanding() const {
    return state_ == State::Expanding || state_ == State::WaitMax;
}

bool DebrisController::IsContracting() const {
    return state_ == State::Contracting;
}