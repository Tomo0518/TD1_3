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
}

void BackgroundManager::Draw(Camera2D& camera, std::string layerName) {
    for (auto& layer : layers_) {
        if (layerName == layer->GetLayerNameTag()) {
            layer->Draw(camera);
        }
    }
}