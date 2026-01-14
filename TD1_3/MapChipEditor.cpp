#include "MapChipEditor.h"
#include "SceneUtilityIncludes.h"
#include <queue>

namespace {
    void DrawSelectionRect_(Camera2D& camera, float startX, float startY, float endX, float endY, unsigned int fillColor, unsigned int wireColor) {
        Vector2 sPos1 = camera.WorldToScreen({ startX, startY });
        Vector2 sPos2 = camera.WorldToScreen({ endX, endY });

        const int x = static_cast<int>(sPos1.x);
        const int y = static_cast<int>(sPos1.y);
        const int w = static_cast<int>(sPos2.x - sPos1.x);
        const int h = static_cast<int>(sPos2.y - sPos1.y);

        Novice::DrawBox(x, y, w, h, 0.0f, fillColor, kFillModeSolid);
        Novice::DrawBox(x, y, w, h, 0.0f, wireColor, kFillModeWireFrame);
    }
}

void MapChipEditor::Initialize() {
    // タイル情報のロード（既にGamePlayScene等で呼ばれていれば不要だが安全のため）
    TileRegistry::Initialize();
    selectedTileId_ = 1;

    undoStack_.clear();
    redoStack_.clear();
    strokeCache_.clear();
    isDragging_ = false;
}

void MapChipEditor::UpdateAndDrawImGui(MapData& mapData, Camera2D& camera) {

    // ショートカットキーの処理 (ImGuiウィンドウ外でも効くように先頭で)
    // Ctrlキーが押されているかチェック
    bool isCtrlPressed = Input().PressKey(DIK_LCONTROL) || Input().PressKey(DIK_RCONTROL);
    bool isShiftPressed = Input().PressKey(DIK_LSHIFT) || Input().PressKey(DIK_RSHIFT);

    // Ctrl + Z : Undo
    if (isCtrlPressed && Input().TriggerKey(DIK_Z)) {
        if (isShiftPressed) {
            ExecuteRedo(mapData); // Ctrl + Shift + Z
        }
        else {
            ExecuteUndo(mapData); // Ctrl + Z
        }
    }
    // Ctrl + Y : Redo (Windows標準)
    if (isCtrlPressed && Input().TriggerKey(DIK_Y)) {
        ExecuteRedo(mapData);
    }

    // --- ImGui ウィンドウ ---
    ImGui::Begin("Map Editor");

    // 保存・ロード
    if (ImGui::Button("Save Map")) {
        mapData.Save("./Resources/data/stage1.json");
        ImGui::OpenPopup("Saved");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Map")) {
        mapData.Load("./Resources/data/stage1.json");
        // ロードしたら履歴はリセットするのが安全
        undoStack_.clear();
        redoStack_.clear();
    }

    if (ImGui::BeginPopup("Saved")) {
        ImGui::Text("Save Complete!");
        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Undo / Redo ボタン
    if (ImGui::Button("Undo (Ctrl+Z)")) {
        ExecuteUndo(mapData);
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo (Ctrl+Y)")) {
        ExecuteRedo(mapData);
    }
    // スタック数を表示（デバッグ用）
    ImGui::Text("History: Undo[%d] Redo[%d]", (int)undoStack_.size(), (int)redoStack_.size());

    // --- ツール切り替え ---
    ImGui::Text("Tools:");
    if (ImGui::RadioButton("Pen (P)", currentMode_ == ToolMode::Pen)) currentMode_ = ToolMode::Pen;
    ImGui::SameLine();
    if (ImGui::RadioButton("Bucket (B)", currentMode_ == ToolMode::Bucket)) currentMode_ = ToolMode::Bucket;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rect (R)", currentMode_ == ToolMode::Rectangle)) currentMode_ = ToolMode::Rectangle;

    // キーボードショートカットでツール切り替え
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (Input().TriggerKey(DIK_P)) currentMode_ = ToolMode::Pen;
        if (Input().TriggerKey(DIK_B)) currentMode_ = ToolMode::Bucket;
        if (Input().TriggerKey(DIK_R)) currentMode_ = ToolMode::Rectangle;
    }

    ImGui::Separator();

    // パレット表示
    ImGui::Text("Select Tile:");
    const auto& tiles = TileRegistry::GetAllTiles();
    int buttonsPerRow = 4;
    int count = 0;

    for (const auto& tile : tiles) {
        std::string label = tile.name + "##" + std::to_string(tile.id);
        if (ImGui::RadioButton(label.c_str(), selectedTileId_ == tile.id)) {
            selectedTileId_ = tile.id;
        }
        count++;
        if (count % buttonsPerRow != 0) ImGui::SameLine();
    }
    if (count % buttonsPerRow != 0) ImGui::NewLine();

    ImGui::Separator();

    // マウス入力処理
    HandleInput(mapData, camera);

    ImGui::End();
}

void MapChipEditor::HandleInput(MapData& mapData, Camera2D& camera) {
    if (ImGui::GetIO().WantCaptureMouse) return;

    // マウス位置の計算
    int mouseX, mouseY;
    Novice::GetMousePosition(&mouseX, &mouseY);
    Vector2 worldPos = camera.ScreenToWorld({ (float)mouseX, (float)mouseY });
    int col = (int)(worldPos.x / mapData.GetTileSize());
    int row = (int)(worldPos.y / mapData.GetTileSize());

    // 範囲外なら何もしない（矩形のドラッグ中は除く）
    bool isInside = (col >= 0 && col < mapData.GetWidth() && row >= 0 && row < mapData.GetHeight());

    // Penツール用：ホバーしている1マスを常にプレビュー表示
    if (currentMode_ == ToolMode::Pen && isInside) {
        const float ts = mapData.GetTileSize();
        const float startX = col * ts;
        const float startY = row * ts;
        const float endX = (col + 1) * ts;
        const float endY = (row + 1) * ts;

        DrawSelectionRect_(camera, startX, startY, endX, endY, 0x00FF0080, 0x00FF00FF); // 緑の半透明＋枠
    }

    // --- Penツール：右クリックで削除（tile=0） ---
    if (currentMode_ == ToolMode::Pen && isInside) {
        // 右クリック開始：ストローク開始
        if (Novice::IsTriggerMouse(1)) {
            isDragging_ = true;
            strokeCache_.clear();
        }

        // 右クリック中：削除
        if (Novice::IsPressMouse(1)) {
            const int currentId = mapData.GetTile(col, row);
            if (currentId != 0) {
                const std::pair<int, int> key = { col, row };
                if (strokeCache_.find(key) == strokeCache_.end()) {
                    strokeCache_[key] = currentId; // 元IDを保存
                }
                mapData.SetTile(col, row, 0); // 削除
            }
        }

        // 右クリック離した：確定
        if (!Novice::IsPressMouse(1) && isDragging_) {
            CommitStroke(mapData);
            isDragging_ = false;
        }
    }

    // 左クリック開始
    if (Novice::IsTriggerMouse(0)) {
        if (isInside) {
            isDragging_ = true;
            strokeCache_.clear();

            if (currentMode_ == ToolMode::Rectangle) {
                dragStartCol_ = col;
                dragStartRow_ = row;
            }
            else if (currentMode_ == ToolMode::Bucket) {
                ToolBucket(mapData, col, row, selectedTileId_);
                CommitStroke(mapData);
                isDragging_ = false;
            }
        }
    }

    // ドラッグ中（ペン）
    if (Novice::IsPressMouse(0) && isDragging_) {
        if (currentMode_ == ToolMode::Pen && isInside) {
            int currentId = mapData.GetTile(col, row);
            if (currentId != selectedTileId_) {
                std::pair<int, int> key = { col, row };
                if (strokeCache_.find(key) == strokeCache_.end()) {
                    strokeCache_[key] = currentId;
                }
                mapData.SetTile(col, row, selectedTileId_);
            }
        }
        else if (currentMode_ == ToolMode::Rectangle) {
            ToolRectangle(mapData, camera, col, row);
        }
    }

    // 左クリック離した（確定）
    if (!Novice::IsPressMouse(0) && isDragging_) {
        isDragging_ = false;

        if (currentMode_ == ToolMode::Rectangle) {
            int minX = (dragStartCol_ < col) ? dragStartCol_ : col;
            int maxX = (dragStartCol_ > col) ? dragStartCol_ : col;
            int minY = (dragStartRow_ < row) ? dragStartRow_ : row;
            int maxY = (dragStartRow_ > row) ? dragStartRow_ : row;

            minX = (minX < 0) ? 0 : minX;
            maxX = (maxX >= mapData.GetWidth()) ? mapData.GetWidth() - 1 : maxX;
            minY = (minY < 0) ? 0 : minY;
            maxY = (maxY >= mapData.GetHeight()) ? mapData.GetHeight() - 1 : maxY;

            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    int currentId = mapData.GetTile(x, y);
                    if (currentId != selectedTileId_) {
                        std::pair<int, int> key = { x, y };
                        if (strokeCache_.find(key) == strokeCache_.end()) {
                            strokeCache_[key] = currentId;
                        }
                        mapData.SetTile(x, y, selectedTileId_);
                    }
                }
            }
        }

        CommitStroke(mapData);
        dragStartCol_ = -1;
    }

    // 右クリック（スポイト）
    if (Novice::IsPressMouse(1) && isInside) {
        int pickedId = mapData.GetTile(col, row);
        if (pickedId != 0) selectedTileId_ = pickedId;
    }
}

// バケツツール（塗りつぶしアルゴリズム）
void MapChipEditor::ToolBucket(MapData& mapData, int startCol, int startRow, int newId) {
    int targetId = mapData.GetTile(startCol, startRow);
    if (targetId == newId) return; // 同じ色なら何もしない

    int w = mapData.GetWidth();
    int h = mapData.GetHeight();

    // 幅優先探索(BFS)用のキュー
    std::queue<std::pair<int, int>> q;
    q.push({ startCol, startRow });

    // 既に処理したかどうかのチェック用（無限ループ防止）
    // strokeCache_ を訪問済みリストとして兼用する
    std::pair<int, int> startKey = { startCol, startRow };
    strokeCache_[startKey] = targetId;
    mapData.SetTile(startCol, startRow, newId);

    // 4方向の移動量
    const int dx[] = { 0, 0, -1, 1 };
    const int dy[] = { -1, 1, 0, 0 };

    while (!q.empty()) {
        std::pair<int, int> current = q.front();
        q.pop();

        int cx = current.first;
        int cy = current.second;

        // 4方向をチェック
        for (int i = 0; i < 4; ++i) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            // 範囲内かつ、塗りつぶし対象の色か？
            if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                int nId = mapData.GetTile(nx, ny);
                if (nId == targetId) {
                    // まだキャッシュにない（未訪問）なら処理
                    std::pair<int, int> nKey = { nx, ny };
                    if (strokeCache_.find(nKey) == strokeCache_.end()) {
                        strokeCache_[nKey] = targetId; // 元の色を記録
                        mapData.SetTile(nx, ny, newId); // 塗る
                        q.push({ nx, ny }); // 次の探索へ
                    }
                }
            }
        }
    }
}

// 矩形ツール（プレビュー表示）
void MapChipEditor::ToolRectangle(MapData& mapData, Camera2D& camera, int currentCol, int currentRow) {
    // 始点と現在のマウス位置から矩形を計算
    float ts = mapData.GetTileSize();

    float startX = dragStartCol_ * ts;
    float startY = dragStartRow_ * ts;
    float endX = (currentCol + 1) * ts; // マウスがあるマスの右下まで含める
    float endY = (currentRow + 1) * ts;

    // 逆方向ドラッグ対応
    if (dragStartCol_ > currentCol) {
        startX = (dragStartCol_ + 1) * ts;
        endX = currentCol * ts;
    }
    if (dragStartRow_ > currentRow) {
        startY = (dragStartRow_ + 1) * ts;
        endY = currentRow * ts;
    }

    // ワールド座標からスクリーン座標へ変換
    Vector2 sPos1 = camera.WorldToScreen({ startX, startY });
    Vector2 sPos2 = camera.WorldToScreen({ endX, endY });

    // 半透明の矩形を描画（プレビュー）
    Novice::DrawBox(
        (int)sPos1.x, (int)sPos1.y,
        (int)(sPos2.x - sPos1.x), (int)(sPos2.y - sPos1.y),
        0.0f, 0xFF000080, kFillModeSolid // 赤色の半透明
    );
    // 枠線
    Novice::DrawBox(
        (int)sPos1.x, (int)sPos1.y,
        (int)(sPos2.x - sPos1.x), (int)(sPos2.y - sPos1.y),
        0.0f, 0xFF0000FF, kFillModeWireFrame
    );
}

void MapChipEditor::CommitStroke(MapData& mapData) {
    if (strokeCache_.empty()) return;

    EditCommand cmd;
    // キャッシュの内容をコマンドに変換
    for (auto const& [pos, prevId] : strokeCache_) {
        int col = pos.first;
        int row = pos.second;
        int currentId = mapData.GetTile(col, row); // 今（書き換え後）のID

        // 実際に値が変わったものだけ記録
        if (prevId != currentId) {
            cmd.logs.push_back({ col, row, prevId, currentId });
        }
    }

    if (!cmd.logs.empty()) {
        undoStack_.push_back(cmd);

        // 新しい操作をしたのでRedoスタックは無効になる
        redoStack_.clear();
    }

    strokeCache_.clear();
}

void MapChipEditor::ExecuteUndo(MapData& mapData) {
    if (undoStack_.empty()) return;

    // 最新のコマンドを取り出す
    EditCommand cmd = undoStack_.back();
    undoStack_.pop_back();

    // 変更を「元に戻す（prevIdにする）」
    for (const auto& log : cmd.logs) {
        mapData.SetTile(log.col, log.row, log.prevId);
    }

    // Redoスタックに積む
    redoStack_.push_back(cmd);
}

void MapChipEditor::ExecuteRedo(MapData& mapData) {
    if (redoStack_.empty()) return;

    // 最新のRedoコマンドを取り出す
    EditCommand cmd = redoStack_.back();
    redoStack_.pop_back();

    // 変更を「やり直す（newIdにする）」
    for (const auto& log : cmd.logs) {
        mapData.SetTile(log.col, log.row, log.newId);
    }

    // Undoスタックに戻す
    undoStack_.push_back(cmd);
}