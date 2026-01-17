#pragma once
#include <vector>
#include <string>

/// <summary>
/// オブジェクトタイプの定義（メタデータ）
/// ゲーム内で使用可能なオブジェクトの「種類」を定義する設計図
/// </summary>
struct ObjectTypeInfo {
    int id;                  // オブジェクトタイプID（一意の識別子）
    std::string name;        // エディタ表示用の名前
    std::string category;    // カテゴリ（System, Player, Enemy, Item等）
    unsigned int color;      // エディタでの表示色（RGBA形式）
    std::string tag;         // ゲーム内で使用するデフォルトタグ
};

/// <summary>
/// オブジェクトタイプの定義を一元管理するレジストリ
/// エディタとゲームの両方で使用される
/// </summary>
class ObjectRegistry {
public:
    /// <summary>
    /// レジストリを初期化し、全オブジェクトタイプを登録
    /// ゲーム起動時に1度だけ呼び出す
    /// </summary>
    static void Initialize();

    /// <summary>
    /// 登録されている全オブジェクトタイプのリストを取得
    /// エディタのパレット表示などに使用
    /// </summary>
    static const std::vector<ObjectTypeInfo>& GetAllObjectTypes();

    /// <summary>
    /// 指定されたIDのオブジェクトタイプ情報を取得
    /// </summary>
    /// <param name="objectTypeId">取得したいオブジェクトのID</param>
    /// <returns>該当するObjectTypeInfo（存在しない場合はnullptr）</returns>
    static const ObjectTypeInfo* GetObjectType(int objectTypeId);

private:
    static std::vector<ObjectTypeInfo> objectTypes_;  // 全オブジェクトタイプの定義
};

/// <summary>
/// オブジェクトタイプIDの定数（マジックナンバー回避用）
/// </summary>
namespace ObjectTypeId {
    constexpr int WorldOrigin = 0;      // ワールド座標系の原点（拠点）
    constexpr int PlayerStart = 100;    // プレイヤー開始位置
    // constexpr int EnemyNormal = 101;
    // constexpr int EnemyBoss = 102;
    // constexpr int ItemCoin = 200;
    // constexpr int ItemPowerUp = 201;
}