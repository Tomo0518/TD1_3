#pragma once
#include <vector>
#include <string>
#include "TextureManager.h"
#include "ParticleEnum.h"

/// <summary>
/// パーティクルタイプのメタ情報
/// エディタ表示名、カテゴリ、デフォルトテクスチャなどを定義
/// </summary>
struct ParticleTypeInfo {
    ParticleType type;           // 内部識別用の型
    std::string name;            // エディタ表示名（JSON のキー名としても使用）
    std::string category;        // カテゴリ（Combat, Environment, Visual等）
    TextureId defaultTexture;    // デフォルトテクスチャ
    unsigned int editorColor;    // エディタUI表示色（RGBA）
};

/// <summary>
/// パーティクルタイプの定義を一元管理するレジストリ
/// TileRegistry と同様の設計パターン
/// </summary>
class ParticleRegistry {
public:
    /// <summary>
    /// レジストリを初期化し、全パーティクルタイプを登録
    /// ゲーム起動時に1度だけ呼び出す
    /// </summary>
    static void Initialize();

    /// <summary>
    /// 登録されている全パーティクルタイプのリストを取得
    /// エディタのパレット表示などに使用
    /// </summary>
    static const std::vector<ParticleTypeInfo>& GetAllParticleTypes();

    /// <summary>
    /// 指定されたタイプのパーティクル情報を取得
    /// </summary>
    /// <param name="type">取得したいパーティクルタイプ</param>
    /// <returns>該当するParticleTypeInfo（存在しない場合はnullptr）</returns>
    static const ParticleTypeInfo* GetParticleType(ParticleType type);

    /// <summary>
    /// 名前からパーティクルタイプを検索（JSON読み込み用）
    /// </summary>
    /// <param name="name">パーティクル名（例: "Explosion"）</param>
    /// <returns>該当するParticleType（存在しない場合はnulloptを想定）</returns>
    static ParticleType GetParticleTypeByName(const std::string& name);

private:
    static std::vector<ParticleTypeInfo> particleTypes_;
};