#pragma once
#include "ParallaxLayer.h"
#include <vector>
#include <memory>
#include "TextureManager.h"
#include <string>

/// <summary>
/// 複数の背景レイヤーを管理
/// </summary>
class BackgroundManager {
public:
    BackgroundManager() = default;

    /// <summary>
    /// 背景レイヤーを追加
    /// </summary>
    /// <param name="textureHandle">テクスチャハンドル</param>
    /// <param name="scrollSpeed">スクロール速度（0.0〜1.0）</param>
    /// <param name="repeatWidth">繰り返し幅</param>
    void AddLayer(TextureId textureId, float scrollSpeed, std::string layerName, float repeatWidth = 1280.0f);

    /// <summary>
    /// 初期カメラ位置を設定（スポーン地点）
    /// </summary>
    /// <param name="initialPos">初期カメラ位置</param>
    void SetInitialCameraPosition(const Vector2& initialPos);

    /// <summary>
    /// 全レイヤーを描画（奥から順に）
    /// </summary>
    void Draw(Camera2D& camera);


    void Draw(Camera2D& camera, std::string layerName);

    /// <summary>
    /// 全レイヤーをクリア
    /// </summary>
    void Clear() { layers_.clear(); }

private:
    std::vector<std::unique_ptr<ParallaxLayer>> layers_;
};