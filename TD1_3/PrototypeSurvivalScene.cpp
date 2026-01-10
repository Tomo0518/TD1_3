#include "PrototypeSurvivalScene.h"
#include "WindowSize.h"
#include "Novice.h"
#include <cmath>
#include <algorithm>

#include "SceneUtilityIncludes.h"

// 便利な定数
const float PI = 3.14159265f;

// 色定義（ABGR順の場合とRGBAの場合があるのでNoviceの仕様に合わせる。ここでは一般的に 0xRRGGBBAA と仮定）
// Noviceは 0xRRGGBBFF かもしれないので適宜調整
const unsigned int COLOR_PLAYER = 0xAAAAFFFF;
const unsigned int COLOR_ENEMY_NORMAL = 0xFF4444FF;
const unsigned int COLOR_ENEMY_TANK = 0x880000FF;
const unsigned int COLOR_DEBRIS_SAFE = 0x00FF00FF; // 緑
const unsigned int COLOR_DEBRIS_EXPAND = 0xFFFF00FF; // 黄
const unsigned int COLOR_DEBRIS_ATTACK = 0xFF0000FF; // 赤
const unsigned int COLOR_DEBRIS_COOLDOWN = 0x555555FF; // グレー

PrototypeSurvivalScene::PrototypeSurvivalScene(SceneManager& mgr) {
    manager_ = &mgr;

    // 初期化
    player_.pos = { kWindowWidth / 2.0f, kWindowHeight / 2.0f };

    debris_.currentRadius = debris_.minRadius;
    debris_.state = DebrisState::Idle;

    // 敵プール確保
    enemies_.resize(100);
}

PrototypeSurvivalScene::~PrototypeSurvivalScene() {}

void PrototypeSurvivalScene::Update(float deltaTime, const char* keys, const char* preKeys) {
	keys; preKeys; // 未使用警告回避
    // InputManagerの更新（引数のkeysは使わず、Manager内部で取得させる）
    Input().Update();

    // ヒットストップ処理（演出のため更新をスキップ）
    if (hitStopTimer_ > 0.0f) {
        hitStopTimer_ -= deltaTime;
        if (hitStopTimer_ <= 0.0f) hitStopTimer_ = 0.0f;
        else return; // 止める
    }

    // シェイク処理
    if (screenShakeTimer_ > 0.0f) {
        screenShakeTimer_ -= deltaTime;
        cameraOffset_.x = (float)(rand() % 100 - 50) / 50.0f * screenShakePower_;
        cameraOffset_.y = (float)(rand() % 100 - 50) / 50.0f * screenShakePower_;
    }
    else {
        cameraOffset_ = { 0,0 };
    }

    UpdatePlayer(deltaTime);
    UpdateDebris(deltaTime);
    UpdateEnemies(deltaTime);
    CheckCollisions();
}

void PrototypeSurvivalScene::UpdatePlayer(float dt) {
    Vector2 moveDir = { 0,0 };

    // キーボード移動
    if (Input().PressKey(DIK_W)) moveDir.y -= 1.0f;
    if (Input().PressKey(DIK_S)) moveDir.y += 1.0f;
    if (Input().PressKey(DIK_A)) moveDir.x -= 1.0f;
    if (Input().PressKey(DIK_D)) moveDir.x += 1.0f;

    // パッド移動（左スティック）
    if (Input().GetInputMode() == InputMode::Gamepad) {
        moveDir = Input().GetPad()->GetLeftStick();
        // Y軸反転対応などはPadクラスの実装によるが、通常上はマイナス
    }

    if (Vector2::Length(moveDir) > 0.0f) {
        moveDir = Vector2::Normalize(moveDir);
        float speed = 300.0f * dt;
        player_.pos += moveDir * speed;
    }

    // 画面外制限
    player_.pos.x = std::clamp(player_.pos.x, player_.radius, kWindowWidth - player_.radius);
    player_.pos.y = std::clamp(player_.pos.y, player_.radius, kWindowHeight - player_.radius);

    // 無敵時間
    if (player_.invincibilityTimer > 0.0f) {
        player_.invincibilityTimer -= dt;
    }
}

void PrototypeSurvivalScene::UpdateDebris(float dt) {
    // 常時回転
    debris_.rotationAngle += debris_.rotationSpeed * dt;

    bool isSpacePressed = Input().PressKey(DIK_SPACE) || Input().GetPad()->Press(Pad::Button::A);

    switch (debris_.state) {
    case DebrisState::Idle:
        debris_.currentRadius = debris_.minRadius;
        if (isSpacePressed) {
            debris_.state = DebrisState::Expanding;
            debris_.isCritical = false;
        }
        break;

    case DebrisState::Expanding:
        debris_.currentRadius += debris_.expandSpeed * dt;
        if (debris_.currentRadius >= debris_.maxRadius) {
            debris_.currentRadius = debris_.maxRadius;
            debris_.state = DebrisState::WaitMax;
            // 最大展開ボーナス音などを鳴らすならここ
            debris_.isCritical = true; // ジャスト最大ならクリティカル扱い
        }
        // ボタン離したら収縮
        if (!isSpacePressed) {
            debris_.state = DebrisState::Contracting;
        }
        break;

    case DebrisState::WaitMax:
        // 維持中は少しノイズを入れて震えさせる演出
        debris_.currentRadius = debris_.maxRadius + (rand() % 5 - 2);
        if (!isSpacePressed) {
            debris_.state = DebrisState::Contracting;
        }
        break;

    case DebrisState::Contracting: {
        // 高速収縮
        float speed = debris_.contractSpeed;
        if (debris_.isCritical) speed *= 1.5f; // クリティカル時はさらに速く

        debris_.currentRadius -= speed * dt;

        if (debris_.currentRadius <= debris_.minRadius) {
            debris_.currentRadius = debris_.minRadius;
            debris_.state = DebrisState::Cooldown;
            debris_.cooldownTimer = 0.5f; // クールダウン時間
            StartShake(0.1f, 5.0f); // 収縮完了の衝撃
        }
        break;
    }

    case DebrisState::Cooldown:
        debris_.cooldownTimer -= dt;
        if (debris_.cooldownTimer <= 0.0f) {
            debris_.state = DebrisState::Idle;
        }
        break;
    }
}

void PrototypeSurvivalScene::UpdateEnemies(float dt) {
    // スポーン処理
    enemySpawnTimer_ += dt;
    float spawnRate = 1.0f; // 1秒に1体
    if (enemySpawnTimer_ > spawnRate) {
        enemySpawnTimer_ = 0.0f;
        SpawnEnemy();
    }

    for (auto& e : enemies_) {
        if (!e.isAlive) continue;

        // ノックバック中は制御不能
        if (e.knockbackDuration > 0.0f) {
            e.knockbackDuration -= dt;
            e.pos += e.knockbackVel * dt;
            // 摩擦で減速
            e.knockbackVel *= 0.9f;
        }
        else {
            // プレイヤーへ向かうAI
            Vector2 toPlayer = player_.pos - e.pos;
            Vector2 dir = Vector2::Normalize(toPlayer);
            float speed = (e.type == EnemyType::Normal) ? 100.0f : 50.0f;
            e.velocity = dir * speed;
            e.pos += e.velocity * dt;
        }
    }
}

void PrototypeSurvivalScene::CheckCollisions() {
    float debrisRingThickness = 15.0f; // ドーナツの厚み

    for (auto& e : enemies_) {
        if (!e.isAlive) continue;

        // 1. プレイヤーとの衝突（ゲームオーバー条件）
        float distToCore = Vector2::Length(player_.pos - e.pos);
        if (distToCore < player_.radius + e.radius) {
            if (player_.invincibilityTimer <= 0.0f) {
                player_.hp--;
                player_.invincibilityTimer = 1.0f;
                StartShake(0.3f, 15.0f);
                // プレイヤーを守るため敵を少し弾く
                e.knockbackVel = Vector2::Normalize(e.pos - player_.pos) * 500.0f;
                e.knockbackDuration = 0.2f;
            }
        }

        // 2. デブリ壁との衝突ロジック
        // リングの範囲内に敵がいるか？
        // (内側判定は少し緩くして、めり込み許容)
        float distDiff = distToCore - debris_.currentRadius;

        // 敵が「リングの厚み」に触れているか
        if (fabs(distDiff) < debrisRingThickness + e.radius) {

            // 重要：スカスカ具合（密度）の計算
            // コアから見た敵の角度
            Vector2 toEnemy = e.pos - player_.pos;
            float enemyAngle = atan2f(toEnemy.y, toEnemy.x); // -PI ~ PI

            // 簡易的に「デブリがある角度かどうか」を判定
            // 全周 2PI を debrisCount で割った角度ごとにデブリがある
            // 現在の半径におけるデブリ1つの「角度幅」
            // 半径が大きいほど、デブリ間の隙間（角度的な空白）は変わらないが、距離的な隙間が開く。
            // 物理的に通り抜けられるかは「円弧の隙間長 > 敵の直径」で決まる。

            float circumference = 2.0f * PI * debris_.currentRadius;
            float gapArcLength = circumference / debris_.debrisCount; // デブリ中心間の距離
            float debrisPhysicalWidth = debris_.debrisSize; // デブリの物理サイズ

            // 隙間（Gap）の長さ = (中心間距離 - デブリサイズ)
            float physicalGap = gapArcLength - debrisPhysicalWidth;

            // 敵のサイズより隙間が大きいなら、確率、あるいは角度判定ですり抜ける
           // bool canPassThrough = false;

            // 厳密な角度判定をする場合：
            // 敵の角度をデブリの回転に合わせて正規化
            float relativeAngle = enemyAngle - debris_.rotationAngle;
            // 0~2PIに収める
            while (relativeAngle < 0) relativeAngle += 2 * PI;
            float step = 2 * PI / debris_.debrisCount;
            float modAngle = fmodf(relativeAngle, step);

            // modAngle が 中心(step/2) に近ければデブリがある、端なら隙間
            // デブリの角度幅
            float debrisAngularHalfWidth = (debris_.debrisSize / 2.0f) / debris_.currentRadius;
            float distFromDebrisCenterAngle = fabs(modAngle - (step / 2.0f)); // その区画のデブリ中心からの角度差

            // デブリに当たっているか？
            bool hitDebris = (distFromDebrisCenterAngle < debrisAngularHalfWidth + (e.radius / debris_.currentRadius));

            // 半径が小さい時は隙間がないので常にヒット扱いにする補正
            if (physicalGap < e.radius * 2.0f) {
                hitDebris = true;
            }

            if (hitDebris) {
                // --- 衝突発生 ---

                if (debris_.state == DebrisState::Contracting) {
                    // 【攻撃】 収縮時はダメージ＋強烈ノックバック
                    e.hp--;
                    StartShake(0.05f, 2.0f); // 軽いシェイク
                    hitStopTimer_ = 0.02f;   // ヒットストップ

                    // 外側へ弾く（絶対ルール）
                    Vector2 knockDir = Vector2::Normalize(e.pos - player_.pos);
                    float power = debris_.isCritical ? 1000.0f : 600.0f;
                    if (debris_.isCritical) e.hp -= 2; // クリティカル追加ダメージ

                    e.knockbackVel = knockDir * power;
                    e.knockbackDuration = 0.3f;

                    if (e.hp <= 0) e.isAlive = false;

                }
                else {
                    // 【防御】 発散・待機時は壁として押し返す
                    // 敵が内側に入ろうとしていたら押し戻す
                    if (distDiff < 0) {
                        // コア中心から敵へのベクトル方向に位置を修正（半径位置まで押し出す）
                        Vector2 pushDir = Vector2::Normalize(e.pos - player_.pos);
                        e.pos = player_.pos + pushDir * (debris_.currentRadius + debrisRingThickness / 2 + e.radius);
                    }
                }
            }
            else {
                // すり抜け発生！（リスク）
                // 特に処理なし、敵はそのまま進行する
            }
        }
    }
}

void PrototypeSurvivalScene::SpawnEnemy() {
    // 未使用の敵を探す
    for (auto& e : enemies_) {
        if (!e.isAlive) {
            e.isAlive = true;
            e.type = (rand() % 5 == 0) ? EnemyType::Tank : EnemyType::Normal; // 20%でタンク
            e.hp = (e.type == EnemyType::Tank) ? 5 : 1;
            e.radius = (e.type == EnemyType::Tank) ? 20.0f : 12.0f;
            e.knockbackDuration = 0.0f;
            e.knockbackVel = { 0,0 };

            // 画面外からスポーン
            float angle = (float)(rand() % 360) * PI / 180.0f;
            float spawnDist = sqrt(kWindowWidth * kWindowWidth + kWindowHeight * kWindowHeight) / 2.0f + 50.0f;
            e.pos = player_.pos + Vector2{ cosf(angle), sinf(angle) } * spawnDist;
            break;
        }
    }
}

void PrototypeSurvivalScene::StartShake(float duration, float power) {
    screenShakeTimer_ = duration;
    screenShakePower_ = power;
}

void PrototypeSurvivalScene::Draw() {
    // シェイク適用のため、描画座標にcameraOffsetを加算して描画する

    // 背景（簡易）
    Novice::DrawBox(0, 0, (int)kWindowWidth, (int)kWindowHeight,0.0f, 0x222233FF, kFillModeSolid);

    // 1. 敵描画
    for (const auto& e : enemies_) {
        if (!e.isAlive) continue;
        unsigned int color = (e.type == EnemyType::Tank) ? COLOR_ENEMY_TANK : COLOR_ENEMY_NORMAL;
        // ノックバック中は白っぽく
        if (e.knockbackDuration > 0) color = 0xFFFFFFFF;

        Novice::DrawBox(
            (int)(e.pos.x - e.radius + cameraOffset_.x),
            (int)(e.pos.y - e.radius + cameraOffset_.y),
            (int)(e.radius * 2), (int)(e.radius * 2),
            0.0f, color, kFillModeSolid
        );
    }

    // 2. プレイヤー描画
    unsigned int pColor = player_.color;
    if (player_.invincibilityTimer > 0 && (int)(player_.invincibilityTimer * 20) % 2 == 0) {
        pColor = 0x8888FFFF; // 点滅
    }
    Novice::DrawBox(
        (int)(player_.pos.x - player_.radius + cameraOffset_.x),
        (int)(player_.pos.y - player_.radius + cameraOffset_.y),
        (int)(player_.radius * 2), (int)(player_.radius * 2),
        0.0f, pColor, kFillModeSolid
    );

    // 3. デブリ（がれき）描画
    unsigned int debrisColor = COLOR_DEBRIS_SAFE;
    if (debris_.state == DebrisState::Expanding) debrisColor = COLOR_DEBRIS_EXPAND;
    if (debris_.state == DebrisState::WaitMax) debrisColor = debris_.isCritical ? 0xFFFFFFFF : COLOR_DEBRIS_EXPAND; // クリティカル待ちで白発光
    if (debris_.state == DebrisState::Contracting) debrisColor = COLOR_DEBRIS_ATTACK;
    if (debris_.state == DebrisState::Cooldown) debrisColor = COLOR_DEBRIS_COOLDOWN;

    float angleStep = 2.0f * PI / debris_.debrisCount;
    for (int i = 0; i < debris_.debrisCount; i++) {
        float angle = i * angleStep + debris_.rotationAngle;

        // 配置座標
        Vector2 debrisLocalPos = {
            cosf(angle) * debris_.currentRadius,
            sinf(angle) * debris_.currentRadius
        };
        Vector2 debrisWorldPos = player_.pos + debrisLocalPos;

        // 回転ノコギリのように各個体も回す
        float selfRot = angle + (debris_.state == DebrisState::Contracting ? debris_.rotationAngle * 2.0f : 0.0f);

        Novice::DrawBox(
            (int)(debrisWorldPos.x - debris_.debrisSize / 2 + cameraOffset_.x),
            (int)(debrisWorldPos.y - debris_.debrisSize / 2 + cameraOffset_.y),
            (int)debris_.debrisSize, (int)debris_.debrisSize,
            selfRot, debrisColor, kFillModeSolid
        );
    }

    // UIデバッグ
  /*  Novice::ScreenPrintf(10, 10, "HP: %d", player_.hp);
    Novice::ScreenPrintf(10, 30, "Radius: %.1f", debris_.currentRadius);
    Novice::ScreenPrintf(10, 50, "State: %d", (int)debris_.state);
    if (debris_.isCritical) Novice::ScreenPrintf(10, 70, "CRITICAL READY!");*/
}