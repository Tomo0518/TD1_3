#pragma once

class GameSceneBase {
public:
	virtual ~GameSceneBase() = default;
	virtual void Update(float deltaTime, const char* keys, const char* preKeys) = 0;
	virtual void Draw() = 0;

	virtual int GetStageIndex() const { return -1; }
};