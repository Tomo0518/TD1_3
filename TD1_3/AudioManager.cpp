#include "AudioManager.h"

AudioManager::AudioManager() {
	// 全てのリソースを初期化（-1は無効値）
	bgmResources_.fill(-1);
	seResources_.fill(-1);

	LoadResources();
}

void AudioManager::LoadResources() {
	// ========================================
	// BGMのロード
	// ========================================
	bgmResources_[static_cast<int>(BgmId::Title)] = Novice::LoadAudio("./Resources/sounds/BGM/title.mp3");
	bgmResources_[static_cast<int>(BgmId::StageSelect)] = bgmResources_[static_cast<int>(BgmId::Title)]; // 同じ曲を使う場合
	bgmResources_[static_cast<int>(BgmId::Stage)] = Novice::LoadAudio("./Resources/sounds/BGM/stage.mp3");
	bgmResources_[static_cast<int>(BgmId::Result)] = Novice::LoadAudio("./Resources/sounds/BGM/result.mp3");

	// ========================================
	// SEのロード
	// ========================================
	seResources_[static_cast<int>(SeId::Select)] = Novice::LoadAudio("./Resources/sounds/SE/moveSelect.mp3");
	seResources_[static_cast<int>(SeId::Decide)] = Novice::LoadAudio("./Resources/sounds/SE/decide.mp3");
	seResources_[static_cast<int>(SeId::Back)] = Novice::LoadAudio("./Resources/sounds/SE/cancel.mp3");
	seResources_[static_cast<int>(SeId::Pause)] = Novice::LoadAudio("./Resources/sounds/SE/cancel.mp3");
	seResources_[static_cast<int>(SeId::PlayerShot)] = Novice::LoadAudio("./Resources/sounds/SE/playerShot.mp3");
}

void AudioManager::PlayBgm(BgmId id, bool loop) {
	// 既に同じ曲が流れているなら何もしない（音量更新だけ念の為行う）
	if (currentBgmId_ == id && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		Novice::SetAudioVolume(currentBgmPlayHandle_, bgmVolume_);
		return;
	}

	// 別の曲が流れている、または停止中なら
	StopBgm(); // 前の曲を止める

	// 新しい曲を再生
	int resourceHandle = bgmResources_[static_cast<int>(id)];
	if (resourceHandle != -1) {
		currentBgmPlayHandle_ = Novice::PlayAudio(resourceHandle, loop, bgmVolume_);
		currentBgmId_ = id;
	}
}

void AudioManager::StopBgm() {
	if (currentBgmPlayHandle_ != -1 && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		Novice::StopAudio(currentBgmPlayHandle_);
	}
	currentBgmPlayHandle_ = -1;
	currentBgmId_ = BgmId::Count; // 「何も再生していない」状態
}

void AudioManager::PlaySe(SeId id) {
	int resourceHandle = seResources_[static_cast<int>(id)];
	if (resourceHandle != -1) {
		Novice::PlayAudio(resourceHandle, false, seVolume_);
	}
}

void AudioManager::SetBgmVolume(float volume) {
	bgmVolume_ = volume;

	// 現在再生中のBGMがあれば、即座に音量を反映させる
	if (currentBgmPlayHandle_ != -1 && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		Novice::SetAudioVolume(currentBgmPlayHandle_, bgmVolume_);
	}
}

void AudioManager::SetSeVolume(float volume) {
	seVolume_ = volume;
	// SEは鳴りっぱなしのものは少ないので、次回の再生から反映でOK
	// もしループ再生する環境音SEなどがある場合は、個別のハンドル管理が必要
}