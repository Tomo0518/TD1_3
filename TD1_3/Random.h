#pragma once
#include<random>

class Random {

public:

	Random() {
		std::random_device rd;
		mt_ = std::mt19937(rd());
	}

	float RandomFloat(float min, float max) {
		std::uniform_real_distribution<float> dist(min, max);
		return dist(mt_);
	}

	int RandomInt(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(mt_);
	}

private:
	std::mt19937 mt_;
};