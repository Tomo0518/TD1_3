#pragma once
#include "Vector2.h"
#include <string>
#include <unordered_map>

/// <summary>
/// マップ上に配置された個別オブジェクトのデータ（インスタンス情報）
/// JSONに保存され、ゲーム実行時に実際のGameObjectを生成するために使用される
/// </summary>
struct ObjectSpawnInfo {
    int objectTypeId = 0;                               // どの種類のオブジェクトか（ObjectRegistryのIDを参照）
    Vector2 position{ 0.0f, 0.0f };                     // ワールド座標での配置位置
    std::string tag = "";                                // オブジェクトのタグ（任意の識別子）
    std::unordered_map<std::string, float> customData;  // 追加パラメータ（HP、速度など）
};