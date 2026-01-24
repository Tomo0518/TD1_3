#include <Novice.h>

#ifdef _DEBUG
#include <imgui.h>
#endif
#include "SceneManager.h"
#include "SoundManager.h"
#include "TextureManager.h"

#include "Camera2D.h"

const char kWindowTitle[] = "==============ゲームタイトル==============";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, (int)kWindowWidth, (int)kWindowHeight);

	const float kDeltaTime = 1.0f;
	SceneManager sceneManager;

	//Novice::SetWindowMode(kFullscreen);

	SoundManager::GetInstance().LoadResources();
	Camera2D::GetInstance().SetIsWorldYUp(true);
	ParticleManager::GetInstance().Load();
	TextureManager::GetInstance().LoadResources();

	// キー入力結果を受け取る箱 
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		sceneManager.Update(kDeltaTime, keys, preKeys);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		sceneManager.Draw();

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		/*if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}*/

		if (sceneManager.ShouldQuit()) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}