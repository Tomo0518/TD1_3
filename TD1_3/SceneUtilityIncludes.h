#pragma once
#include "TextureManager.h"
#include "SoundManager.h"
#include "InputManager.h"
#include "ParticleManager.h"

namespace GameServices {

	inline SoundManager& Sound() { return SoundManager::GetInstance(); }
	inline InputManager& Input() { return InputManager::GetInstance(); }
	inline ParticleManager& Particle() { return ParticleManager::GetInstance(); }
	inline TextureManager& Tex() { return TextureManager::GetInstance(); }

} // namespace GameServices

using namespace GameServices;