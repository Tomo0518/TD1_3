#include "DrawComponent2D.h"
#include <memory>
#include "TileRegistry.h"
#include <memory>
#include "TextureManager.h"

class TileInstance {
public:
    TileInstance(const TileDefinition& def, const Vector2& worldPos)
        : id_(def.id), worldPos_(worldPos), isActive_(false) {

        if (def.renderMode == RenderMode::Component) {
            // 個別に実体を生成
            drawComp_ = std::make_unique<DrawComponent2D>(
                def.textureId,
                def.animConfig.divX,
                def.animConfig.divY,
                def.animConfig.totalFrames,
                def.animConfig.speed,
                def.animConfig.isLoop
            );
			drawComp_->SetAnchorPoint(def.animConfig.anchorPoint);

            // アニメーションの開始時間をバラつかせて「自然さ」を出す
            if (def.animConfig.isAnimated) {
                float randomOffset = static_cast<float>(rand()) / RAND_MAX;
                drawComp_->Update(randomOffset);
            }

			drawLayer_ = def.drawLayer;
        }
    }

    void Update(float dt, bool inActiveRange) {
        if (!drawComp_) return;

        if (inActiveRange) {
            isActive_ = true;
            drawComp_->SetPosition(worldPos_);
            drawComp_->Update(dt);
        }
        else if (isActive_) {
            // 画面外に出た瞬間にリセットして
            drawComp_->StopAllEffects();
            isActive_ = false;
        }
    }

    void Draw(const Camera2D& camera) {
        if (drawComp_) {
            drawComp_->Draw(camera);
        }
    }

    // 外界（攻撃等）からの干渉用
    void OnHit() {
        if (drawComp_) {
            drawComp_->StartShake(3.0f, 0.2f); // 叩いた手応え！
        }
    }

	Vector2 GetWorldPos() const { return worldPos_; }
    DrawLayer GetDrawLayer() const { return drawLayer_; }

private:
    int id_;
    Vector2 worldPos_;
    bool isActive_;
    std::unique_ptr<DrawComponent2D> drawComp_;
	DrawLayer drawLayer_;
};