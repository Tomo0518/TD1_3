#pragma once
#include "TextureManager.h"
#include "SoundManager.h"
#include "InputManager.h"

namespace SceneServices {

	inline SoundManager& Sound() { return SoundManager::GetInstance(); }
	inline InputManager& Input() { return InputManager::GetInstance(); }
	inline TextureManager& Tex() { return TextureManager::GetInstance(); }

} // namespace SceneServices