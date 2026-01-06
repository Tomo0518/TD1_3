#pragma once
#include <Novice.h>
#include <array>
#include <string>

// BGMの種類（ID）
enum class BgmId {
	Title,
	StageSelect,
	Stage,
	Result,
	Count // 総数カウント用
};

// SEの種類（ID）
enum class SeId {
	Select,
	Decide,
	Back,
	Pause,
	PlayerShot,



	// 必要な分だけここに追加
	Count // 総数カウント用
};

class AudioManager {
public:
	AudioManager();

	// ==========================================
	// 再生制御
	// ==========================================

	/// <summary>
	/// 指定したBGMを再生（既に再生中の場合は何もしない、別の曲なら切り替える）
	/// </summary>
	/// <param name="id">再生したいBGMのID</param>
	/// <param name="loop">ループするか</param>
	void PlayBgm(BgmId id, bool loop = true);

	/// <summary>
	/// BGMを停止
	/// </summary>
	void StopBgm();

	/// <summary>
	/// SEを再生
	/// </summary>
	void PlaySe(SeId id);

	// ==========================================
	// 音量設定（設定画面用）
	// ==========================================

	void SetBgmVolume(float volume);
	float GetBgmVolume() const { return bgmVolume_; }

	void SetSeVolume(float volume);
	float GetSeVolume() const { return seVolume_; }

private:
	// リソース読み込み用ヘルパー
	void LoadResources();

private:
	// 音量 (0.0f ~ 1.0f)
	float bgmVolume_ = 0.1f;
	float seVolume_ = 0.2f;

	// リソースハンドル（読み込んだデータ）
	std::array<int, static_cast<int>(BgmId::Count)> bgmResources_;
	std::array<int, static_cast<int>(SeId::Count)> seResources_;

	// 再生状態管理
	int currentBgmPlayHandle_ = -1; // 現在再生中のBGMの再生ハンドル
	BgmId currentBgmId_ = BgmId::Count; // 現在再生中のBGMの種類
};