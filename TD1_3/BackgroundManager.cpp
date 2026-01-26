#include "BackgroundManager.h"

void BackgroundManager::AddLayer(TextureId textureId, float scrollSpeed, std::string layerName, float repeatWidth) {
    auto layer = std::make_unique<ParallaxLayer>(textureId, scrollSpeed, layerName, repeatWidth);
    layers_.push_back(std::move(layer));
}

void BackgroundManager::SetInitialCameraPosition(const Vector2& initialPos) {
    for (auto& layer : layers_) {
        layer->SetInitialCameraPosition(initialPos);
    }
}

void BackgroundManager::Draw(Camera2D& camera) {
    // 奥のレイヤーから順に描画
    for (auto& layer : layers_) {
        layer->Draw(camera);
    }

	//Novice::DrawSprite(0, 0, TextureManager::GetInstance().GetTexture(TextureId::Background_Foreground), 1.0f, 1.0f, 0.0f, 0xFFFFFFFF);
	
}

void BackgroundManager::Draw(Camera2D& camera, std::string layerName) {
    for (auto& layer : layers_) {
        if (layerName == layer->GetLayerNameTag()) {
            layer->Draw(camera);
        }
    }
}