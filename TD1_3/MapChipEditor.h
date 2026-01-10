#pragma once
#include "MapData.h"
#include "Camera2D.h"
#include "TileRegistry.h"
#include <Novice.h>
#include <imgui.h>

#include "SceneUtilityIncludes.h" // Inputクラスなど共通ユーティリティ

class MapEditor {
public:
    // 初期化
    void Initialize() {
        // タイル情報のロード（ゲーム起動時に1回呼ぶ）
        TileRegistry::Initialize();
        selectedTileId_ = 1; // デフォルトで何か選択しておく
    }

    // 更新＆描画（ImGuiの処理）
    void UpdateAndDrawImGui(MapData& mapData, Camera2D& camera) {
        ImGui::Begin("Map Editor");

        // --- 1. 保存・ロード機能 ---
        if (ImGui::Button("Save Map")) {
            mapData.Save("./Resources/data/stage1.json");
            ImGui::OpenPopup("Saved");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load Map")) {
            mapData.Load("./Resources/data/stage1.json");
        }

        // 保存完了メッセージ
        if (ImGui::BeginPopup("Saved")) {
            ImGui::Text("Save Complete!");
            ImGui::EndPopup();
        }

        ImGui::Separator();

        // --- 2. パレット自動生成 ---
        ImGui::Text("Select Tile:");

        // TileRegistryにある全ブロックをボタンとして表示
        const auto& tiles = TileRegistry::GetAllTiles();

        // グリッド状に並べるための設定
        int buttonsPerRow = 4;
        int count = 0;

        for (const auto& tile : tiles) {
            // 選択中のものは色を変えるなどの強調も可能
            std::string label = tile.name + "##" + std::to_string(tile.id);

            // 選択状態のラジオボタンのように振る舞う
            if (ImGui::RadioButton(label.c_str(), selectedTileId_ == tile.id)) {
                selectedTileId_ = tile.id;
            }

            // 横並び処理
            count++;
            if (count % buttonsPerRow != 0) {
                ImGui::SameLine();
            }
        }
        // 改行リセット
        if (count % buttonsPerRow != 0) ImGui::NewLine();

        ImGui::Separator();

        // --- 3. マウスによる配置処理 ---
        ImGui::Text("Click on screen to place blocks.");
        HandleMouseInput(mapData, camera);

        ImGui::End();
    }

private:
    int selectedTileId_ = 0;

    // マウス入力を処理してマップを書き換える
    void HandleMouseInput(MapData& mapData,  Camera2D& camera) {
        // ImGuiのウィンドウ上にある時はゲーム画面への操作を無効化する
        if (ImGui::GetIO().WantCaptureMouse) return;

        // 左クリックで配置
        if (Novice::IsPressMouse(0)) {
            Vector2 mousePos = { (float)Input().GetMousePosition().x, (float)Input().GetMousePosition().y}; // Inputクラス等は環境に合わせて

            // スクリーン座標 → ワールド座標
            Vector2 worldPos = camera.ScreenToWorld(mousePos);

            // ワールド座標 → グリッド座標 (MapDataのメソッドがあればそれを使うのも良し)
            int col = (int)(worldPos.x / mapData.GetTileSize());
            int row = (int)(worldPos.y / mapData.GetTileSize());

            // 範囲内なら書き換え
            if (col >= 0 && col < mapData.GetWidth() && row >= 0 && row < mapData.GetHeight()) {
                mapData.SetTile(col, row, selectedTileId_);
            }
        }

        // 右クリックでスポイト（その場所のブロックを選択）機能があると便利
        if (Novice::IsPressMouse(1)) {
            Vector2 mousePos = { (float)Input().GetMousePosition().x, (float)Input().GetMousePosition().y };
            Vector2 worldPos = camera.ScreenToWorld(mousePos);
            int col = (int)(worldPos.x / mapData.GetTileSize());
            int row = (int)(worldPos.y / mapData.GetTileSize());

            int pickedId = mapData.GetTile(col, row);
            if (pickedId != 0) selectedTileId_ = pickedId; // 0(空気)は吸わない方が使いやすいかも
        }
    }
};