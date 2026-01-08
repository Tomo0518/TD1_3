#pragma once
#include <string>
#include <vector>
#include <map>

// 1つのタイルの情報をまとめた構造体
struct TileDefinition {
    int id;             // マップデータに保存される数値 (例: 1)
    std::string name;   // エディタ表示名 (例: "Ground")
    int textureIndex;   // タイルセット画像内の何番目の絵か (例: 0)
    bool isSolid;       // 当たり判定があるか（Physics用、今はデータとして持つだけ）
};

// 全タイルのカタログを管理するクラス（どこからでも使えるようにStaticにする）
class TileRegistry {
public:
    // 定義リストを取得
    static const std::vector<TileDefinition>& GetAllTiles() {
        return tiles_;
    }

    // IDから定義を取得（描画や判定で使う）
    static const TileDefinition* GetTile(int id) {
        for (const auto& tile : tiles_) {
            if (tile.id == id) return &tile;
        }
        return nullptr;
    }

    // 初期化：ここでブロックの種類を登録する！
    // 新しいブロックを追加したい時はここに行を追加する
    static void Initialize() {
        tiles_.clear();
        // ID, 名前, 画像Index, 当たり判定
        tiles_.push_back({ 0, "Air",         0, false }); // 空気（消しゴム用）
        tiles_.push_back({ 1, "Ground",      1, true });
        tiles_.push_back({ 2, "Brick",       2, true });
        tiles_.push_back({ 3, "Spike",       3, true });
        tiles_.push_back({ 4, "Ice",         4, true });
        tiles_.push_back({ 10, "Goal",      10, false });
        // どんどん追加できる
    }

private:
    static std::vector<TileDefinition> tiles_;
};

// 実体定義（cppファイルに書くべきだが、簡単のためここに書くならinlineが必要。
// 正しくは TileRegistry.cpp を作ってそこに書く）
inline std::vector<TileDefinition> TileRegistry::tiles_;