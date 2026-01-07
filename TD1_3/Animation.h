#pragma once
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


class Animation {
private:
	// グラフィックハンドル
	int grHandle_ = -1;

	// スプレッドシート情報
	int frameWidth_ = 32;			// 1フレームの幅（ピクセル）
	int frameHeight_ = 32;			// 1フレームの高さ（ピクセル）
	int framesPerRow_ = 1;			// 1行あたりのフレーム数
	int totalFrames_ = 1;			// 総フレーム数

	// アニメーション制御
	int currentFrame_ = 0;			// 現在のフレーム番号（0から開始）
	bool isPlaying_ = false;
	float frameDuration_ = 0.1f;	// 1フレームの表示時間（秒）
	float elapsedTime_ = 0.0f;		// 現在フレームの経過時間
	bool isLooping_ = true;

	// 描画用ソース矩形（計算結果）
	int srcX_ = 0;
	int srcY_ = 0;
	int srcW_ = 0;
	int srcH_ = 0;

public:
	// コンストラクタ
	Animation(int grHandle, int frameWidth, int frameHeight, int totalFrames,
		int framesPerRow, float frameSpeed, bool isLoop)
		: grHandle_(grHandle)
		, frameWidth_(std::max(1, frameWidth))        // ←最小値1を保証
		, frameHeight_(std::max(1, frameHeight))      // ←最小値1を保証
		, totalFrames_(std::max(1, totalFrames))      // ←最小値1を保証
		, framesPerRow_(std::max(1, framesPerRow))    // ←最小値1を保証（ゼロ除算回避）
		, frameDuration_(std::max(0.001f, frameSpeed)) // ←最小値0.001秒を保証
		, isLooping_(isLoop) {

		isPlaying_ = true;
		// 初期フレーム設定
		UpdateSourceRect();
	}

	~Animation() = default;

	// コピーコンストラクタを追加
	Animation(const Animation& other)
		: grHandle_(other.grHandle_)
		, frameWidth_(other.frameWidth_)
		, frameHeight_(other.frameHeight_)
		, totalFrames_(other.totalFrames_)
		, framesPerRow_(other.framesPerRow_)
		, frameDuration_(other.frameDuration_)
		, isLooping_(other.isLooping_)
		, currentFrame_(other.currentFrame_)
		, isPlaying_(other.isPlaying_)
	{
	}

	// アニメーション制御
	void Play() {
		isPlaying_ = true;
		currentFrame_ = 0;
		elapsedTime_ = 0.0f;
		UpdateSourceRect();
	}

	void Stop() {
		isPlaying_ = false;
		currentFrame_ = 0;
		elapsedTime_ = 0.0f;
		UpdateSourceRect();
	}

	void Pause() {
		isPlaying_ = false;
	}

	void Resume() {
		isPlaying_ = true;
	}


	void Initialize() {
		currentFrame_ = 0;
		elapsedTime_ = 0.0f;
		UpdateSourceRect();
	}

	// フレーム更新
	void Update(float deltaTime) {
		if (!isPlaying_) return;

		elapsedTime_ += deltaTime;

		// フレーム切り替えのタイミングをチェック
		while (elapsedTime_ >= frameDuration_) {
			elapsedTime_ -= frameDuration_;
			currentFrame_++;

			// フレーム範囲チェック
			if (currentFrame_ >= totalFrames_) {
				if (isLooping_) {
					currentFrame_ = 0;
				}
				else {
					currentFrame_ = totalFrames_ - 1;
					isPlaying_ = false;
				}
			}

			UpdateSourceRect();
		}
	}

	// ソース矩形計算
	void UpdateSourceRect() {
		// 現在フレームの行と列を計算
		int row = currentFrame_ / framesPerRow_;
		int col = currentFrame_ % framesPerRow_;

		// ソース矩形を設定
		srcX_ = col * frameWidth_;
		srcY_ = row * frameHeight_;
		srcW_ = frameWidth_;
		srcH_ = frameHeight_;
	}

	// 特定フレームに移動
	void SetFrame(int frameIndex) {
		if (frameIndex >= 0 && frameIndex < totalFrames_) {
			currentFrame_ = frameIndex;
			elapsedTime_ = 0.0f;
			UpdateSourceRect();
		}
	}

	// Getter
	int GetGraphHandle() const { return grHandle_; }
	int GetSrcX() const { return srcX_; }
	int GetSrcY() const { return srcY_; }
	int GetSrcW() const { return srcW_; }
	int GetSrcH() const { return srcH_; }
	int GetCurrentFrame() const { return currentFrame_; }
	int GetTotalFrames() const { return totalFrames_; }
	int GetFrameWidth() const { return frameWidth_; }
	int GetFrameHeight() const { return frameHeight_; }
	bool IsPlaying() const { return isPlaying_; }
	bool IsLooping() const { return isLooping_; }
	float GetFrameDuration() const { return frameDuration_; }
	float GetElapsedTime() const { return elapsedTime_; }

	// Setter
	void SetFrameDuration(float duration) {
		if (duration > 0.0f) {
			frameDuration_ = duration;
		}
	}

	void SetLooping(bool loop) { isLooping_ = loop; }

	void SetTotalFrames(int total) {
		if (total > 0) {
			totalFrames_ = total;
			if (currentFrame_ >= totalFrames_) {
				currentFrame_ = totalFrames_ - 1;
				UpdateSourceRect();
			}
		}
	}

	// デバッグ用
	void PrintDebugInfo() const {
		// デバッグ出力（必要に応じて実装）
	}
};