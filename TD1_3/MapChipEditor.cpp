#include "MapChipEditor.h"
#include "SceneUtilityIncludes.h"
#include "ObjectRegistry.h"
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

#ifdef _DEBUG
void MapChipEditor::Initialize(MapManager* manager) {
	mapManager_ = manager;
	selectedTileId_ = 1;
	selectedObjectTypeId_ = 0;
	selectedObjectIndex_ = -1;
	currentMode_ = ToolMode::Pen;
	currentLayer_ = TileLayer::Block;

	undoStack_.clear();
	redoStack_.clear();
	strokeCache_.clear();
	isDragging_ = false;
	dragStartCol_ = -1;
}

void MapChipEditor::UpdateAndDrawImGui(MapData& mapData, Camera2D& camera) {
	// ショートカットキー処理
	bool isCtrlPressed = Input().PressKey(DIK_LCONTROL) || Input().PressKey(DIK_RCONTROL);
	bool isShiftPressed = Input().PressKey(DIK_LSHIFT) || Input().PressKey(DIK_RSHIFT);

	if (isCtrlPressed && Input().TriggerKey(DIK_Z)) {
		if (isShiftPressed) {
			ExecuteRedo(mapData);
		}
		else {
			ExecuteUndo(mapData);
		}
	}
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
		undoStack_.clear();
		redoStack_.clear();
	}

	if (ImGui::BeginPopup("Saved")) {
		ImGui::Text("Save Complete!");
		ImGui::EndPopup();
	}

	ImGui::Separator();

	// Undo / Redo
	if (ImGui::Button("Undo (Ctrl+Z)")) {
		ExecuteUndo(mapData);
	}
	ImGui::SameLine();
	if (ImGui::Button("Redo (Ctrl+Y)")) {
		ExecuteRedo(mapData);
	}
	ImGui::Text("History: Undo[%d] Redo[%d]", (int)undoStack_.size(), (int)redoStack_.size());

	// マウス位置表示
	int mouseX, mouseY;
	Novice::GetMousePosition(&mouseX, &mouseY);
	Vector2 worldPos = camera.ScreenToWorld({ (float)mouseX, (float)mouseY });
	int col = (int)(worldPos.x / mapData.GetTileSize());
	int row = (int)(worldPos.y / mapData.GetTileSize());
	ImGui::Text("Mouse Position: (%.1f, %.1f)", worldPos.x, worldPos.y);
	ImGui::Text("Tile Position: (%d, %d)", col, row);

	ImGui::Separator();

	// --- モード選択 ---
	ImGui::Text("Mode:");
	if (ImGui::RadioButton("Tile Edit", currentMode_ != ToolMode::Object)) {
		if (currentMode_ == ToolMode::Object) {
			currentMode_ = ToolMode::Pen;
			selectedObjectIndex_ = -1;
		}
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Object (4)", currentMode_ == ToolMode::Object)) {
		currentMode_ = ToolMode::Object;
	}

	// キーボードショートカット
	if (!ImGui::GetIO().WantCaptureKeyboard) {
		if (Input().TriggerKey(DIK_4)) {
			currentMode_ = ToolMode::Object;
		}
	}

	ImGui::Separator();

	// --- タイル編集モード ---
	if (currentMode_ != ToolMode::Object) {

		// 1. 表示用データを用意
		static const char* layerNames[] = { "Tile Block (1)", "Decoration (2)", "Background Deco Block (3)" };

		// 2. Enumとリストの対応付け
		TileLayer layerEnums[] = { TileLayer::Block, TileLayer::Decoration, TileLayer::BackgroundDecoration };

		// 3. 現在のレイヤーが、リストの何番目かを調べる
		int currentItemIndex = 0;
		for (int i = 0; i < IM_ARRAYSIZE(layerEnums); i++) {
			if (currentLayer_ == layerEnums[i]) {
				currentItemIndex = i;
				break;
			}
		}

		// 4. ImGui::Combo の表示
		ImGui::Text("Layer Selection:");
		// 第2引数は int* 型が必要。選択されると currentItemIndex が書き換わる
		if (ImGui::Combo("##layerCombo", &currentItemIndex, layerNames, IM_ARRAYSIZE(layerNames))) {
			// 選択が変更されたら、共通関数を呼ぶ
			ChangeLayer(layerEnums[currentItemIndex]);
		}

		// --- ショートカットキー処理 ---
		if (!ImGui::GetIO().WantCaptureKeyboard) {
			if (Input().TriggerKey(DIK_1)) {
				ChangeLayer(TileLayer::Block);
			}
			if (Input().TriggerKey(DIK_2)) {
				ChangeLayer(TileLayer::Decoration);
			}
			if (Input().TriggerKey(DIK_3)) {
				ChangeLayer(TileLayer::BackgroundDecoration);
			}
		}

		ImGui::Separator();

		// ツール選択
		ImGui::Text("Tools:");
		if (ImGui::RadioButton("Pen (P)", currentMode_ == ToolMode::Pen)) currentMode_ = ToolMode::Pen;
		ImGui::SameLine();
		if (ImGui::RadioButton("Bucket (B)", currentMode_ == ToolMode::Bucket)) currentMode_ = ToolMode::Bucket;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rect (R)", currentMode_ == ToolMode::Rectangle)) currentMode_ = ToolMode::Rectangle;

		if (!ImGui::GetIO().WantCaptureKeyboard) {
			if (Input().TriggerKey(DIK_P)) currentMode_ = ToolMode::Pen;
			if (Input().TriggerKey(DIK_B)) currentMode_ = ToolMode::Bucket;
			if (Input().TriggerKey(DIK_R)) currentMode_ = ToolMode::Rectangle;
		}

		ImGui::Separator();

		// タイルパレット
		ImGui::Text("Select Tile:");
		const auto& tiles = TileRegistry::GetAllTiles();
		int buttonsPerRow = 4;
		int count = 0;

		for (const auto& tile : tiles) {
			if (tile.id == 0 || tile.layer == currentLayer_) {
				std::string label = tile.name + "##" + std::to_string(tile.id);
				if (ImGui::RadioButton(label.c_str(), selectedTileId_ == tile.id)) {
					selectedTileId_ = tile.id;
				}
				count++;
				if (count % buttonsPerRow != 0) ImGui::SameLine();
			}
		}
		if (count % buttonsPerRow != 0) ImGui::NewLine();

		ImGui::Separator();

		// タイル編集の入力処理
		HandleInput(mapData, camera);
	}
	// --- オブジェクト配置モード ---
	else {
		DrawObjectPalette();
		ImGui::Separator();
		DrawObjectList(mapData);
		ImGui::Separator();
		HandleObjectMode(mapData, camera);
	}

	ImGui::End();

}

void MapChipEditor::DrawObjectPalette() {
	ImGui::Text("Select Object Type:");
	const auto& objectTypes = ObjectRegistry::GetAllObjectTypes();

	for (const auto& objType : objectTypes) {
		std::string label = objType.name + "##obj" + std::to_string(objType.id);
		if (ImGui::RadioButton(label.c_str(), selectedObjectTypeId_ == objType.id)) {
			selectedObjectTypeId_ = objType.id;
		}
	}
}

void MapChipEditor::DrawObjectList(MapData& mapData) {
	mapData; // 未使用警告回避
	ImGui::Text("Placed Objects (%d):", (int)mapData.GetObjectSpawns().size());

	const auto& spawns = mapData.GetObjectSpawns();
	for (size_t i = 0; i < spawns.size(); ++i) {
		const auto& spawn = spawns[i];
		const auto* objType = ObjectRegistry::GetObjectType(spawn.objectTypeId);
		std::string name = objType ? objType->name : "Unknown";

		ImGui::PushID((int)i);

		// 選択状態の表示
		bool isSelected = (selectedObjectIndex_ == (int)i);
		if (isSelected) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
		}

		ImGui::Text("%d: %s (%.1f, %.1f)", (int)i, name.c_str(), spawn.position.x, spawn.position.y);

		if (isSelected) {
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			mapData.RemoveObjectSpawn(i);
			selectedObjectIndex_ = -1;
		}

		ImGui::PopID();
	}
}

void MapChipEditor::HandleObjectMode(MapData& mapData, Camera2D& camera) {
	// 配置済みオブジェクトの描画（マップ上にアイコン表示）
	// この部分はマウスキャプチャに関係なく常に描画
	const auto& spawns = mapData.GetObjectSpawns();
	for (size_t i = 0; i < spawns.size(); ++i) {
		const auto& spawn = spawns[i];

		// ObjectRegistryから色を取得
		const auto* objType = ObjectRegistry::GetObjectType(spawn.objectTypeId);
		unsigned int baseColor = objType ? objType->color : 0x0000FFFF;

		// アイコンサイズ（32x32ピクセル）
		const float iconSize = 32.0f;
		Vector2 iconWorldMin = { spawn.position.x - iconSize / 2.0f, spawn.position.y - iconSize / 2.0f };
		Vector2 iconWorldMax = { spawn.position.x + iconSize / 2.0f, spawn.position.y + iconSize / 2.0f };

		Vector2 screenMin = camera.WorldToScreen(iconWorldMin);
		Vector2 screenMax = camera.WorldToScreen(iconWorldMax);

		// 選択中は黄色、それ以外はオブジェクトタイプの色
		unsigned int color = (selectedObjectIndex_ == (int)i) ? 0xFFFF00FF : baseColor;

		Novice::DrawBox(
			(int)screenMin.x, (int)screenMin.y,
			(int)(screenMax.x - screenMin.x), (int)(screenMax.y - screenMin.y),
			0.0f, color, kFillModeWireFrame
		);

		// 中心に小さい点
		Novice::DrawBox(
			(int)camera.WorldToScreen(spawn.position).x - 2,
			(int)camera.WorldToScreen(spawn.position).y - 2,
			4, 4, 0.0f, color, kFillModeSolid
		);
	}

	// --- ここから入力処理 ---
	// マウスがImGuiウィンドウ上にある場合は入力処理をスキップ
	if (ImGui::GetIO().WantCaptureMouse) return;

	// マウス位置取得
	int mouseX, mouseY;
	Novice::GetMousePosition(&mouseX, &mouseY);
	Vector2 worldPos = camera.ScreenToWorld({ (float)mouseX, (float)mouseY });

	// 左クリック：オブジェクト配置
	if (Novice::IsTriggerMouse(0)) {
		// 既存のオブジェクトをクリックしたか判定
		bool clickedExisting = false;
		const float clickRadius = 16.0f;

		for (size_t i = 0; i < spawns.size(); ++i) {
			float dx = worldPos.x - spawns[i].position.x;
			float dy = worldPos.y - spawns[i].position.y;
			float distSq = dx * dx + dy * dy;

			if (distSq < clickRadius * clickRadius) {
				selectedObjectIndex_ = (int)i;
				clickedExisting = true;
				break;
			}
		}

		// 既存をクリックしていなければ新規配置
		if (!clickedExisting) {
			const auto* objType = ObjectRegistry::GetObjectType(selectedObjectTypeId_);
			std::string tag = objType ? objType->tag : "";

			// WorldOrigin (0) と PlayerStart (100) は1つまで
			if (selectedObjectTypeId_ == 0 || selectedObjectTypeId_ == 100) {
				// 既存の同じタイプを探す
				bool alreadyExists = false;
				for (size_t i = 0; i < spawns.size(); ++i) {
					if (spawns[i].objectTypeId == selectedObjectTypeId_) {
						// 既に存在する場合は位置を更新（移動）
						mapData.UpdateObjectSpawnPosition(i, worldPos);
						selectedObjectIndex_ = (int)i;
						alreadyExists = true;
						Novice::ConsolePrintf("[MapEditor] Moved %s to (%.1f, %.1f)\n",
							objType->name.c_str(), worldPos.x, worldPos.y);
						break;
					}
				}

				// 存在しない場合は新規配置
				if (!alreadyExists) {
					mapData.AddObjectSpawn(selectedObjectTypeId_, worldPos, tag);
					selectedObjectIndex_ = -1;
					Novice::ConsolePrintf("[MapEditor] Placed %s at (%.1f, %.1f)\n",
						objType->name.c_str(), worldPos.x, worldPos.y);
				}
			}
			// それ以外のオブジェクトは複数配置可能
			else {
				mapData.AddObjectSpawn(selectedObjectTypeId_, worldPos, tag);
				selectedObjectIndex_ = -1;
				Novice::ConsolePrintf("[MapEditor] Placed object type %d at (%.1f, %.1f)\n",
					selectedObjectTypeId_, worldPos.x, worldPos.y);
			}
		}
	}

	// 右クリック：選択解除
	if (Novice::IsTriggerMouse(1)) {
		selectedObjectIndex_ = -1;
	}

	// Deleteキー：選択中のオブジェクト削除
	if (selectedObjectIndex_ >= 0 && Input().TriggerKey(DIK_DELETE)) {
		mapData.RemoveObjectSpawn(selectedObjectIndex_);
		selectedObjectIndex_ = -1;
	}
}

void MapChipEditor::HandleInput(MapData& mapData, Camera2D& camera) {
	// マウス位置の計算（描画に必要なので常に実行）
	int mouseX, mouseY;
	Novice::GetMousePosition(&mouseX, &mouseY);
	Vector2 worldPos = camera.ScreenToWorld({ (float)mouseX, (float)mouseY });
	int col = (int)(worldPos.x / mapData.GetTileSize());
	int row = (int)(worldPos.y / mapData.GetTileSize());

	// 範囲外なら何もしない（矩形のドラッグ中は除く）
	bool isInside = (col >= 0 && col < mapData.GetWidth() && row >= 0 && row < mapData.GetHeight());

	// --- 描画処理（マウスキャプチャに関係なく実行）---

	// Penツール用：ホバーしている1マスを常にプレビュー表示
	if (currentMode_ == ToolMode::Pen && isInside) {
		const float ts = mapData.GetTileSize();
		const float startX = col * ts;
		const float startY = row * ts;
		const float endX = (col + 1) * ts;
		const float endY = (row + 1) * ts;

		DrawSelectionRect_(camera, startX, startY, endX, endY, 0x00FF0080, 0x00FF00FF); // 緑の半透明＋枠
	}

	// 矩形ツールのプレビュー（ドラッグ中）
	if (currentMode_ == ToolMode::Rectangle && isDragging_ && Novice::IsPressMouse(0)) {
		ToolRectanglePreview(mapData, camera, col, row);
	}

	// --- ここから入力処理 ---
	// マウスがImGuiウィンドウ上にある場合は入力処理をスキップ
	if (ImGui::GetIO().WantCaptureMouse) return;

	// --- Penツール：右クリックで削除（tile=0） ---
	if (currentMode_ == ToolMode::Pen && isInside) {
		// 右クリック開始：ストローク開始
		if (Novice::IsTriggerMouse(1)) {
			isDragging_ = true;
			strokeCache_.clear();
		}

		// 右クリック中：削除
		if (Novice::IsPressMouse(1)) {
			const int currentId = mapData.GetTile(col, row, currentLayer_);
			if (currentId != 0) {
				const std::pair<int, int> key = { col, row };
				if (strokeCache_.find(key) == strokeCache_.end()) {
					strokeCache_[key] = currentId; // 元IDを保存
				}
				mapData.SetTile(col, row, 0, currentLayer_); // 削除
				// MapManagerへの通知
				if (mapManager_) {
					mapManager_->OnTileChanged(col, row, currentLayer_);
				}
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


	// ドラッグ中（ペンのみ、矩形プレビューは上で描画済み）
	if (Novice::IsPressMouse(0) && isDragging_) {
		if (currentMode_ == ToolMode::Pen && isInside) {
			int currentId = mapData.GetTile(col, row, currentLayer_);
			if (currentId != selectedTileId_) {
				std::pair<int, int> key = { col, row };
				if (strokeCache_.find(key) == strokeCache_.end()) {
					strokeCache_[key] = currentId;
				}
				mapData.SetTile(col, row, selectedTileId_, currentLayer_);

				if (mapManager_) {
					mapManager_->OnTileChanged(col, row, currentLayer_);
				}

			}
		}
	}

	// 左クリック離した（確定）
	if (!Novice::IsPressMouse(0) && isDragging_) {
		isDragging_ = false;

		if (currentMode_ == ToolMode::Rectangle) {
			// 矩形を適用
			ToolRectangleApply(mapData, col, row);
			// MapManagerへの通知
			if (mapManager_) {
				mapManager_->OnTileChanged(col, row, currentLayer_);
			}
		}

		CommitStroke(mapData);
		dragStartCol_ = -1;
	}

	// 右クリック（スポイト）- 削除機能と競合しないように、ドラッグ中でない場合のみ
	if (Novice::IsTriggerMouse(1) && isInside && currentMode_ != ToolMode::Pen) {
		int pickedId = mapData.GetTile(col, row, currentLayer_);
		if (pickedId != 0) selectedTileId_ = pickedId;
	}
}

// バケツツール（塗りつぶしアルゴリズム）
void MapChipEditor::ToolBucket(MapData& mapData, int startCol, int startRow, int newId) {
	int targetId = mapData.GetTile(startCol, startRow, currentLayer_);
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
	mapData.SetTile(startCol, startRow, newId, currentLayer_);

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
				int nId = mapData.GetTile(nx, ny, currentLayer_);
				if (nId == targetId) {
					// まだキャッシュにない（未訪問）なら処理
					std::pair<int, int> nKey = { nx, ny };
					if (strokeCache_.find(nKey) == strokeCache_.end()) {
						strokeCache_[nKey] = targetId; // 元の色を記録
						mapData.SetTile(nx, ny, newId, currentLayer_); // 塗る

						// MapManagerへの通知
						if (mapManager_) {
							mapManager_->OnTileChanged(nx, ny, currentLayer_);
						}

						q.push({ nx, ny }); // 次の探索へ
					}
				}
			}
		}
	}
}

// 矩形ツール（プレビュー表示のみ）
void MapChipEditor::ToolRectanglePreview(MapData& mapData, Camera2D& camera, int currentCol, int currentRow) {
	// 始点が無効な場合は何もしない
	if (dragStartCol_ < 0 || dragStartRow_ < 0) return;

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

// 矩形ツール（適用処理）
void MapChipEditor::ToolRectangleApply(MapData& mapData, int endCol, int endRow) {
	// 始点が無効な場合は何もしない
	if (dragStartCol_ < 0 || dragStartRow_ < 0) return;

	// 矩形の範囲を計算
	int minX = (dragStartCol_ < endCol) ? dragStartCol_ : endCol;
	int maxX = (dragStartCol_ > endCol) ? dragStartCol_ : endCol;
	int minY = (dragStartRow_ < endRow) ? dragStartRow_ : endRow;
	int maxY = (dragStartRow_ > endRow) ? dragStartRow_ : endRow;

	// マップの範囲内にクランプ
	minX = (minX < 0) ? 0 : minX;
	maxX = (maxX >= mapData.GetWidth()) ? mapData.GetWidth() - 1 : maxX;
	minY = (minY < 0) ? 0 : minY;
	maxY = (maxY >= mapData.GetHeight()) ? mapData.GetHeight() - 1 : maxY;

	// 矩形範囲内の全てのタイルを変更
	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			int currentId = mapData.GetTile(x, y, currentLayer_);
			if (currentId != selectedTileId_) {
				std::pair<int, int> key = { x, y };
				// 同じ操作で複数回変更された場合でも、最初の状態を保持
				if (strokeCache_.find(key) == strokeCache_.end()) {
					strokeCache_[key] = currentId;
				}
				mapData.SetTile(x, y, selectedTileId_, currentLayer_);

				// MapManagerへの通知
				if (mapManager_) {
					mapManager_->OnTileChanged(x, y, currentLayer_);
				}
			}
		}
	}
}

void MapChipEditor::CommitStroke(MapData& mapData) {
	if (strokeCache_.empty()) return;

	EditCommand cmd;
	// キャッシュの内容をコマンドに変換
	for (auto const& [pos, prevId] : strokeCache_) {
		int col = pos.first;
		int row = pos.second;
		int currentId = mapData.GetTile(col, row, currentLayer_); // 今（書き換え後）のID

		// 実際に値が変わったものだけ記録
		if (prevId != currentId) {
			cmd.logs.push_back({ col, row, prevId, currentId, currentLayer_ });
		}
	}

	if (!cmd.logs.empty()) {
		undoStack_.push_back(cmd);

		// 新しい操作をしたのでRedoスタックは無効になる
		redoStack_.clear();
	}

	strokeCache_.clear();
}

// ヘルパー関数の実装（クラス内など適当な場所に）
void MapChipEditor::ChangeLayer(TileLayer newLayer) {
	if (currentLayer_ == newLayer) return; // 同じなら何もしない

	currentLayer_ = newLayer;

	// レイヤー変更時に、そのレイヤーに合ったタイルIDにリセットする
	switch (currentLayer_) {
	case TileLayer::Block:
		selectedTileId_ = TileID::Block_Ground; // 1
		break;
	case TileLayer::Decoration:
		selectedTileId_ = TileID::Deco_Grass;  // 10
		break;
	case TileLayer::BackgroundDecoration:
		selectedTileId_ = TileID::Deco_RockBlock; // 200
		break;
	default:
		break;
	}
}

void MapChipEditor::ExecuteUndo(MapData& mapData) {
	if (undoStack_.empty()) return;

	// 最新のコマンドを取り出す
	EditCommand cmd = undoStack_.back();
	undoStack_.pop_back();

	// 変更を「元に戻す（prevIdにする）」
	for (const auto& log : cmd.logs) {
		mapData.SetTile(log.col, log.row, log.prevId, log.layer);
		mapManager_->OnTileChanged(log.col, log.row, log.layer);
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
		mapData.SetTile(log.col, log.row, log.newId, log.layer);
		mapManager_->OnTileChanged(log.col, log.row, log.layer);
	}

	// Undoスタックに戻す
	undoStack_.push_back(cmd);
}
#endif // _DEBUG