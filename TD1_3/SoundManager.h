#pragma once
#include <array>
#include "Novice.h"

// BGMの識別ID
enum class BgmId {
	None = -1,
	Title,
	StageSelect,
	Stage,
	Result,
	Count // 総数
};

// SEの識別ID
enum class SeId {
	// Menu
	Select,
	Decide,
	Back,
	Pause,

	// Game
	PlayerShot,

	// Player
	PlayerJump,
	PlayerDash,
	PlayerDamage,
	PlayerLand,
	PlayerBoomerangThrow,
	PlayerBoomerangFly,
	PlayerBoomerangReturn,
	PlayerStarCollect,

	// Enemy
	EnemyDamage1,
	EnemyDamage2,
	EnemyDamage3,

	EnemyAttack1,
	EnemyAttack2,
	EnemyAttack3,

	EnemyFindPlayer1,
	EnemyFindPlayer2,
	EnemyFindPlayer3,

	// Item
	StarSpawn,






	Count // 総数
};

// 音声リソース構造体
struct AudioResource {
	int handle = -1;        // Noviceのハンドル
	float volume = 1.0f;    // 個別音量（0.0f～1.0f）
	const char* name = "";  // 表示名（デバッグ用）
};

class SoundManager {
public:
	// シングルトンインスタンス取得
	static SoundManager& GetInstance() {
		static SoundManager instance;
		return instance;
	}

	// コピー/ムーブ禁止
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

	void LoadResources();

	// BGM再生・停止
	void PlayBgm(BgmId id, bool loop = true);
	void StopBgm();

	// SE再生
	void PlaySe(SeId id);

	// 音量設定（グローバル）
	void SetBgmVolume(float volume);
	void SetSeVolume(float volume);

	// 個別音量設定
	void SetBgmIndividualVolume(BgmId id, float volume);
	void SetSeIndividualVolume(SeId id, float volume);

	// 個別音量取得
	float GetBgmIndividualVolume(BgmId id) const;
	float GetSeIndividualVolume(SeId id) const;

	// 音量取得（グローバル）
	float GetBgmVolume() const { return bgmVolume_; }
	float GetSeVolume() const { return seVolume_; }

	// 現在の設定を適用
	void ApplyAudioSettings();

	// ImGui用のデバッグウィンドウ
	void ShowDebugWindow();

	// BGM/SEリソースの取得（ImGui用）
	const std::array<AudioResource, static_cast<int>(BgmId::Count)>& GetBgmResources() const { return bgmResources_; }
	const std::array<AudioResource, static_cast<int>(SeId::Count)>& GetSeResources() const { return seResources_; }

private:
	SoundManager();
	~SoundManager() = default;

	// リソース配列
	std::array<AudioResource, static_cast<int>(BgmId::Count)> bgmResources_;
	std::array<AudioResource, static_cast<int>(SeId::Count)> seResources_;

	// グローバル音量
	float bgmVolume_ = 0.3f;
	float seVolume_ = 0.3f;

	// 現在再生中のBGM情報
	int currentBgmPlayHandle_ = -1;
	BgmId currentBgmId_ = BgmId::None;

	// ロード済みフラグ
	bool isLoaded_ = false;

	// ImGui用のプレビュー再生ハンドル
	int previewPlayHandle_ = -1;
};