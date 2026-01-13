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

// ツールの種類
enum class ToolMode {
    Pen,        // 1マス配置（ドラッグで連続）
    Bucket,     // 塗りつぶし
    Rectangle   // 矩形塗りつぶし
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

    // デバッグ用などに現在のツールを取得したい場合
    ToolMode GetCurrentTool() const { return currentMode_; }

private:
    int selectedTileId_ = 1;

    // 現在のツールモード
    ToolMode currentMode_ = ToolMode::Pen;

    // --- Undo / Redo 用変数 ---
    std::vector<EditCommand> undoStack_;
    std::vector<EditCommand> redoStack_;
    std::map<std::pair<int, int>, int> strokeCache_;
    bool isDragging_ = false;

    // --- 矩形ツール用変数 ---
    int dragStartCol_ = -1;
    int dragStartRow_ = -1;

    // --- 内部メソッド ---
    void HandleInput(MapData& mapData, Camera2D& camera);
    void ExecuteUndo(MapData& mapData);
    void ExecuteRedo(MapData& mapData);
    void CommitStroke(MapData& mapData);

    // 塗りつぶし処理
    void ToolBucket(MapData& mapData, int col, int row, int newId);
    // 矩形処理
    void ToolRectangle(MapData& mapData,Camera2D& camera, int col, int row);
};