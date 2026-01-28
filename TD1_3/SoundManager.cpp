#include "SoundManager.h"
#ifdef _DEBUG
#include <imgui.h>
#endif
SoundManager::SoundManager() {
	LoadResources();
}

void SoundManager::LoadResources() {
	if (isLoaded_) {
		return;
	}

	// ========================================
	// BGMのロード（ハンドル、個別音量、表示名）
	// ========================================
	bgmResources_[static_cast<int>(BgmId::Title)] = {
		Novice::LoadAudio("./Resources/sounds/BGM/title.mp3"),
		0.7f,
		"Title BGM"
	};

	bgmResources_[static_cast<int>(BgmId::StageSelect)] = {
		bgmResources_[static_cast<int>(BgmId::Title)].handle, // 同じ曲を使う
		0.7f,
		"Stage Select BGM"
	};

	bgmResources_[static_cast<int>(BgmId::Stage)] = {
		Novice::LoadAudio("./Resources/sounds/BGM/stage.mp3"),
		0.8f,
		"Stage BGM"
	};

	bgmResources_[static_cast<int>(BgmId::Result)] = {
		Novice::LoadAudio("./Resources/sounds/BGM/result.mp3"),
		0.75f,
		"Result BGM"
	};

	// ========================================
	// SEのロード（ハンドル、個別音量、表示名）
	// ========================================

	// ===== Menu SE =====
	seResources_[static_cast<int>(SeId::Select)] = {
		Novice::LoadAudio("./Resources/sounds/SE/menu/moveSelect.mp3"),
		0.4f,
		"Menu Select"
	};

	seResources_[static_cast<int>(SeId::Decide)] = {
		Novice::LoadAudio("./Resources/sounds/SE/menu/decide.mp3"),
		0.5f,
		"Menu Decide"
	};

	seResources_[static_cast<int>(SeId::Back)] = {
		Novice::LoadAudio("./Resources/sounds/SE/menu/cancel.mp3"),
		0.4f,
		"Menu Back"
	};

	seResources_[static_cast<int>(SeId::Pause)] = {
		Novice::LoadAudio("./Resources/sounds/SE/menu/cancel.mp3"),
		0.4f,
		"Pause"
	};

	// ===== Game SE =====

	//----- Player SE -----

	seResources_[static_cast<int>(SeId::PlayerShot)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/shot.mp3"),
		0.6f,
		"Player Shot"
	};

	seResources_[static_cast<int>(SeId::PlayerJump)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/jump.mp3"),
		0.5f,
		"Player Jump"
	};
	seResources_[static_cast<int>(SeId::PlayerDash)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/dash.mp3"),
		0.7f,
		"Player Dash"
	};
	seResources_[static_cast<int>(SeId::PlayerDamage)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/damage.mp3"),
		0.7f,
		"Player Damage"
	};
	seResources_[static_cast<int>(SeId::PlayerLand)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/land.mp3"),
		0.2f,
		"Player Land"
	};

	// ブーメラン関連
	seResources_[static_cast<int>(SeId::PlayerBoomerangThrow)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/boomerang/throw1.mp3"),
		0.6f,
		"Player Boomerang Throw"
	};
	seResources_[static_cast<int>(SeId::PlayerBoomerangFly)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/boomerang/fly_loop.mp3"),
		0.5f,
		"Player Boomerang Fly"
	};
	seResources_[static_cast<int>(SeId::PlayerBoomerangReturn)] = {
		Novice::LoadAudio("./Resources/sounds/SE/player/boomerang/return.mp3"),
		0.6f,
		"Player Boomerang Return"
	};

	seResources_[static_cast<int>(SeId::PlayerStarCollect)] = {
		Novice::LoadAudio("./Resources/sounds/SE/star/collect.mp3"),
		0.5f,
		"Player Star Collect"
	};

	//----- Enemy SE -----
	seResources_[static_cast<int>(SeId::EnemyDamage1)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_damage1.mp3"),
		0.6f,
		"Enemy Damage 1"
	};
	seResources_[static_cast<int>(SeId::EnemyDamage2)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_damage2.mp3"),
		0.6f,
		"Enemy Damage 2"
	};
	seResources_[static_cast<int>(SeId::EnemyDamage3)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_damage3.mp3"),
		0.6f,
		"Enemy Damage 3"
	};
	seResources_[static_cast<int>(SeId::EnemyAttack1)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_attack1.mp3"),
		0.6f,
		"Enemy Attack 1"
	};
	seResources_[static_cast<int>(SeId::EnemyAttack2)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_attack2.mp3"),
		0.6f,
		"Enemy Attack 2"
	};
	seResources_[static_cast<int>(SeId::EnemyAttack3)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_attack2.mp3"), // 仮置き
		0.6f,
		"Enemy Attack 3"
	};
	seResources_[static_cast<int>(SeId::EnemyFindPlayer1)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_find1.mp3"),
		0.6f,
		"Enemy Find Player 1"
	};
	seResources_[static_cast<int>(SeId::EnemyFindPlayer2)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_find2.mp3"),
		0.6f,
		"Enemy Find Player 2"
	};
	seResources_[static_cast<int>(SeId::EnemyFindPlayer3)] = {
		Novice::LoadAudio("./Resources/sounds/SE/enemy/mob_find3.mp3"),
		0.6f,
		"Enemy Find Player 3"
	};

	//----- Item SE -----
	seResources_[static_cast<int>(SeId::StarSpawn)] = {
		Novice::LoadAudio("./Resources/sounds/SE/star/spawn.mp3"),
		0.5f,
		"Star Spawn"
	};

	isLoaded_ = true;
}

void SoundManager::PlayBgm(BgmId id, bool loop) {
	// 既に同じ曲が流れているなら何もしない
	if (currentBgmId_ == id && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		// 音量だけ更新
		float finalVolume = bgmVolume_ * bgmResources_[static_cast<int>(id)].volume;
		Novice::SetAudioVolume(currentBgmPlayHandle_, finalVolume);
		return;
	}

	// 別の曲が流れている、または停止中なら
	StopBgm();

	// 新しい曲を再生
	const AudioResource& resource = bgmResources_[static_cast<int>(id)];
	if (resource.handle != -1) {
		float finalVolume = bgmVolume_ * resource.volume;
		currentBgmPlayHandle_ = Novice::PlayAudio(resource.handle, loop, finalVolume);
		currentBgmId_ = id;
	}
}

void SoundManager::StopBgm() {
	if (currentBgmPlayHandle_ != -1 && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		Novice::StopAudio(currentBgmPlayHandle_);
	}
	currentBgmPlayHandle_ = -1;
	currentBgmId_ = BgmId::None;
}

void SoundManager::PlaySe(SeId id) {
	const AudioResource& resource = seResources_[static_cast<int>(id)];
	if (resource.handle != -1) {
		float finalVolume = seVolume_ * resource.volume;
		Novice::PlayAudio(resource.handle, false, finalVolume);
	}
}

void SoundManager::SetBgmVolume(float volume) {
	bgmVolume_ = volume;

	// 現在再生中のBGMがあれば、即座に音量を反映
	if (currentBgmPlayHandle_ != -1 && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		float finalVolume = bgmVolume_ * bgmResources_[static_cast<int>(currentBgmId_)].volume;
		Novice::SetAudioVolume(currentBgmPlayHandle_, finalVolume);
	}
}

void SoundManager::SetSeVolume(float volume) {
	seVolume_ = volume;
}

void SoundManager::SetBgmIndividualVolume(BgmId id, float volume) {
	bgmResources_[static_cast<int>(id)].volume = volume;

	// 現在再生中のBGMならば音量を更新
	if (currentBgmId_ == id && currentBgmPlayHandle_ != -1 && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		float finalVolume = bgmVolume_ * volume;
		Novice::SetAudioVolume(currentBgmPlayHandle_, finalVolume);
	}
}

void SoundManager::SetSeIndividualVolume(SeId id, float volume) {
	seResources_[static_cast<int>(id)].volume = volume;
}

float SoundManager::GetBgmIndividualVolume(BgmId id) const {
	return bgmResources_[static_cast<int>(id)].volume;
}

float SoundManager::GetSeIndividualVolume(SeId id) const {
	return seResources_[static_cast<int>(id)].volume;
}

void SoundManager::ApplyAudioSettings() {
	if (currentBgmPlayHandle_ != -1 && Novice::IsPlayingAudio(currentBgmPlayHandle_)) {
		float finalVolume = bgmVolume_ * bgmResources_[static_cast<int>(currentBgmId_)].volume;
		Novice::SetAudioVolume(currentBgmPlayHandle_, finalVolume);
	}
}

void SoundManager::ShowDebugWindow() {
#ifdef _DEBUG
	if (ImGui::Begin("Sound Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		// ========== グローバル音量設定 ==========
		ImGui::SeparatorText("Global Volume");

		float bgmVol = bgmVolume_;
		if (ImGui::SliderFloat("BGM Volume", &bgmVol, 0.0f, 1.0f, "%.2f")) {
			SetBgmVolume(bgmVol);
		}

		float seVol = seVolume_;
		if (ImGui::SliderFloat("SE Volume", &seVol, 0.0f, 1.0f, "%.2f")) {
			SetSeVolume(seVol);
		}

		ImGui::Spacing();

		// ========== BGM一覧 ==========
		ImGui::SeparatorText("BGM Resources");

		for (int i = 0; i < static_cast<int>(BgmId::Count); ++i) {
			AudioResource& resource = bgmResources_[i];
			if (resource.handle == -1) continue;

			ImGui::PushID(i);

			// BGM名表示
			ImGui::Text("%s", resource.name);

			// 個別音量スライダー
			if (ImGui::SliderFloat("Volume", &resource.volume, 0.0f, 0.5f, "%.2f")) {
				SetBgmIndividualVolume(static_cast<BgmId>(i), resource.volume);
			}

			// 最終音量表示
			float finalVolume = bgmVolume_ * resource.volume;
			ImGui::SameLine();
			ImGui::Text("(Final: %.2f)", finalVolume);

			// プレビュー再生ボタン
			if (ImGui::Button("Play")) {
				PlayBgm(static_cast<BgmId>(i), true);
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				StopBgm();
			}

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::Spacing();

		// ========== SE一覧 ==========
		ImGui::SeparatorText("SE Resources");

		for (int i = 0; i < static_cast<int>(SeId::Count); ++i) {
			AudioResource& resource = seResources_[i];
			if (resource.handle == -1) continue;

			ImGui::PushID(1000 + i); // IDの衝突を避ける

			// SE名表示
			ImGui::Text("%s", resource.name);

			// 個別音量スライダー
			if (ImGui::SliderFloat("Volume", &resource.volume, 0.0f, 5.0f, "%.2f")) {
				SetSeIndividualVolume(static_cast<SeId>(i), resource.volume);
			}

			// 最終音量表示
			float finalVolume = seVolume_ * resource.volume;
			ImGui::SameLine();
			ImGui::Text("(Final: %.2f)", finalVolume);

			// プレビュー再生ボタン
			if (ImGui::Button("Play")) {
				PlaySe(static_cast<SeId>(i));
			}

			ImGui::Separator();
			ImGui::PopID();
		}

		// ========== 現在のBGM情報 ==========
		ImGui::Spacing();
		ImGui::SeparatorText("Current BGM");

		if (currentBgmId_ != BgmId::None) {
			const AudioResource& current = bgmResources_[static_cast<int>(currentBgmId_)];
			ImGui::Text("Playing: %s", current.name);
			ImGui::Text("Handle: %d", currentBgmPlayHandle_);
			ImGui::Text("Is Playing: %s", Novice::IsPlayingAudio(currentBgmPlayHandle_) ? "Yes" : "No");
		}
		else {
			ImGui::Text("No BGM playing");
		}
	}
	ImGui::End();
#endif
}