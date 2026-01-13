#pragma once
#include "MapData.h"
#include "Camera2D.h"
#include "TileRegistry.h"
#include <Novice.h>
#include <imgui.h>
#include <vector>
#include <map>

// 1マスの変更履歴
struct TileChangeLog {
    int col;
    int row;
    int prevId; // 変更前のID
    int newId;  // 変更後のID
};

// 1回のアクション（一筆書き）をまとめたコマンド
struct EditCommand {
    std::vector<TileChangeLog> logs;
};

class MapChipEditor {
public:
    // 初期化
    void Initialize();

    // 更新＆描画
    void UpdateAndDrawImGui(MapData& mapData, Camera2D& camera);

private:
    int selectedTileId_ = 1;

    // --- Undo / Redo 用変数 ---
    std::vector<EditCommand> undoStack_;
    std::vector<EditCommand> redoStack_;

    // ドラッグ中の一時キャッシュ (座標{col, row} -> 変更前のID)
    // これにより、同じ場所をグリグリしても「最初の状態」を覚えておける
    std::map<std::pair<int, int>, int> strokeCache_;
    bool isDragging_ = false;

    // --- 内部メソッド ---
    void HandleInput(MapData& mapData, Camera2D& camera);
    void ExecuteUndo(MapData& mapData);
    void ExecuteRedo(MapData& mapData);
    void CommitStroke(MapData& mapData); // ドラッグ終了時に履歴を確定させる
};