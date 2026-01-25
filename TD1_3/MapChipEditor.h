#pragma once
#include "MapData.h"
#include "Camera2D.h"
#include "TileRegistry.h"
#include <Novice.h>
#include <vector>
#include <map>
#include "MapManager.h"

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG


// 1マスの変更履歴
struct TileChangeLog {
    int col;
    int row;
    int prevId;
    int newId;
    TileLayer layer;
};

// ツールの種類
enum class ToolMode {
    Pen,
    Eraser,
    Bucket,
    Rectangle,
    Object
};

// 1回のアクション（一筆書き）をまとめたコマンド
struct EditCommand {
    std::vector<TileChangeLog> logs;
};

class MapChipEditor {
public:

#ifdef _DEBUG
    void Initialize(MapManager* manager);
    void UpdateAndDrawImGui(MapData& mapData, Camera2D& camera);
    ToolMode GetCurrentTool() const { return currentMode_; }

	void SetMapManager(MapManager* manager) { mapManager_ = manager; }

private:
    int selectedTileId_ = 1;
	int preEraserTileId_ = 1;
    ToolMode currentMode_ = ToolMode::Pen;
    TileLayer currentLayer_ = TileLayer::Block;

	MapManager* mapManager_ = nullptr;

    // ファイルパス指定用
    char currentFilePath_[256] = "./Resources/data/stage1.json";

    // オブジェクトモード用
    int selectedObjectTypeId_ = 100;  // デフォルト: PlayerStart
    int selectedObjectIndex_ = -1;    // 選択中のオブジェクトインデックス（-1=未選択）

    // Undo / Redo用
    std::vector<EditCommand> undoStack_;
    std::vector<EditCommand> redoStack_;
    std::map<std::pair<int, int>, int> strokeCache_;
    bool isDragging_ = false;

    // 矩形ツール用
    int dragStartCol_ = -1;
    int dragStartRow_ = -1;

    // 内部メソッド
    void HandleInput(MapData& mapData, Camera2D& camera);
    void HandleObjectMode(MapData& mapData, Camera2D& camera);
    void DrawObjectPalette();
    void DrawObjectList(MapData& mapData);

    void ExecuteUndo(MapData& mapData);
    void ExecuteRedo(MapData& mapData);
    void CommitStroke(MapData& mapData);

    // レイヤーを変更する関数、モードチェンジに呼び出す
    void ChangeLayer(TileLayer newLayer);

    // ツール処理
    void ToolBucket(MapData& mapData, int col, int row, int newId);
    void ToolRectanglePreview(MapData& mapData, Camera2D& camera, int col, int row);
    void ToolRectangleApply(MapData& mapData, int endCol, int endRow, int applyTileId);
#endif // _DEBUG
};