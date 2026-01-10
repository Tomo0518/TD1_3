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

	None, // BGMなし指定用
	Count // 総数カウント用
};

// SEの種類（ID）
enum class SeId {
	Select,
	Decide,
	Back,
	Pause,
	PlayerShot,

	Count // 総数カウント用
};

class SoundManager {
public:
	SoundManager();

	static SoundManager& GetInstance() {
		static SoundManager instance;
		return instance;
	}

	// 削除・コピー禁止
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

	// ==========================================
	// 再生制御
	// ==========================================

	void PlayBgm(BgmId id, bool loop = true);
	void StopBgm();
	void PlaySe(SeId id);

	// ==========================================
	// 音量設定（設定画面用）
	// ==========================================
	void SetBgmVolume(float volume);
	float GetBgmVolume() const { return bgmVolume_; }

	void SetSeVolume(float volume);
	float GetSeVolume() const { return seVolume_; }

	// 現在再生中のBGMにボリュームを再適用
	void ApplyAudioSettings();

	// 一度だけリソースをロード(mainの初期化時に一回)
	void LoadResources();
private:
	
	bool isLoaded_ = false;

	float bgmVolume_ = 0.1f;
	float seVolume_ = 0.2f;

	std::array<int, static_cast<int>(BgmId::Count)> bgmResources_;
	std::array<int, static_cast<int>(SeId::Count)> seResources_;

	int currentBgmPlayHandle_ = -1;
	BgmId currentBgmId_ = BgmId::Count;
};