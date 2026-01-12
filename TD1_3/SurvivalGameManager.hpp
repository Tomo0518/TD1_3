#pragma once
#include <cassert>
#include <list>
#include <memory>
#include <string>

#include "Camera2D.h"
#include "GameObject2D.h"
#include "TextureManager.h"

class SurvivalPlayer;
class DebrisController;
class SurvivalEnemy;

class SurvivalGameObjectManager {
public:
	SurvivalGameObjectManager() = default;
	~SurvivalGameObjectManager() { Clear(); }

	void Update(float deltaTime) {
		auto it = objects_.begin();
		while (it != objects_.end()) {
			(*it)->Update(deltaTime);

			if (!(*it)->GetInfo().isActive) {
				it = objects_.erase(it);
				continue;
			}

			++it;
		}

		enemies_.remove_if([](const std::shared_ptr<SurvivalEnemy>& e) {
			if (e->GetInfo().isActive == false) {
				e->Destroy();
				return true;
			}
			return false;
			});

		CheckCollisions();
	}

	void Draw(const Camera2D& camera) {
		for (auto& obj : objects_) {
			obj->Draw(camera);
		}
	}

	void AddObject(const std::shared_ptr<GameObject2D>& obj, const std::string& tag) {
		assert(obj && "AddObject: obj is null");
		assert(!tag.empty() && "AddObject: tag must not be empty");

		obj->GetInfo().tag = tag;

		// プロト用：テクスチャ未設定でも描画できるように white を当てる
		obj->SetTexture(TextureId::White1x1);

		objects_.push_back(obj);

		// Enemy キャッシュ
		if (auto enemy = std::dynamic_pointer_cast<SurvivalEnemy>(obj)) {
			enemies_.push_back(enemy);
		}
	}

	void SetPlayer(std::shared_ptr<SurvivalPlayer> player) { player_ = std::move(player); }
	std::shared_ptr<SurvivalPlayer> GetPlayer() const { return player_; }

	void SetDebrisController(std::shared_ptr<DebrisController> debris) { debrisController_ = std::move(debris); }
	std::shared_ptr<DebrisController> GetDebrisController() const { return debrisController_; }

	const std::list<std::shared_ptr<SurvivalEnemy>>& GetEnemies() const { return enemies_; }

	void Clear() {
		objects_.clear();
		enemies_.clear();
		player_.reset();
		debrisController_.reset();
	}

	unsigned int GetObjectsSize() const {
		return static_cast<unsigned int>(objects_.size());
	}

private:
	std::list<std::shared_ptr<GameObject2D>> objects_;

	std::shared_ptr<SurvivalPlayer> player_;
	std::shared_ptr<DebrisController> debrisController_;
	std::list<std::shared_ptr<SurvivalEnemy>> enemies_;

	void CheckCollisions();
};