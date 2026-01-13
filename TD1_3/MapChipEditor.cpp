#include "MapChipEditor.h"
#include "SceneUtilityIncludes.h"

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
    // ImGui操作中はゲーム画面を触らせない
    if (ImGui::GetIO().WantCaptureMouse) return;

    // 左クリック開始
    if (Novice::IsTriggerMouse(0)) {
        isDragging_ = true;
        strokeCache_.clear(); // 新しい一筆書きの開始
    }

    // ドラッグ中（またはクリック中）
    if (Novice::IsPressMouse(0) && isDragging_) {
        // マウス位置の取得
        int mouseX, mouseY;
        Novice::GetMousePosition(&mouseX, &mouseY);
        Vector2 mousePos = { (float)mouseX, (float)mouseY };
        Vector2 worldPos = camera.ScreenToWorld(mousePos);

        int col = (int)(worldPos.x / mapData.GetTileSize());
        int row = (int)(worldPos.y / mapData.GetTileSize());

        // 範囲チェック
        if (col >= 0 && col < mapData.GetWidth() && row >= 0 && row < mapData.GetHeight()) {
            int currentId = mapData.GetTile(col, row);

            // 「違うブロック」を置こうとしている時だけ処理する
            if (currentId != selectedTileId_) {
                // まだこのストロークで触っていない場所なら、変更前のIDを保存しておく
                std::pair<int, int> key = { col, row };
                if (strokeCache_.find(key) == strokeCache_.end()) {
                    strokeCache_[key] = currentId;
                }

                // マップデータを即時書き換え（見た目に反映）
                mapData.SetTile(col, row, selectedTileId_);
            }
        }
    }

    // 左クリック離した（一筆書き終了）
    if (!Novice::IsPressMouse(0) && isDragging_) {
        isDragging_ = false;
        CommitStroke(mapData); // 履歴に確定
    }

    // 右クリック（スポイト機能）
    if (Novice::IsPressMouse(1)) {
        int mouseX, mouseY;
        Novice::GetMousePosition(&mouseX, &mouseY);
        Vector2 worldPos = camera.ScreenToWorld({ (float)mouseX, (float)mouseY });
        int col = (int)(worldPos.x / mapData.GetTileSize());
        int row = (int)(worldPos.y / mapData.GetTileSize());

        int pickedId = mapData.GetTile(col, row);
        if (pickedId != 0) selectedTileId_ = pickedId;
    }
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