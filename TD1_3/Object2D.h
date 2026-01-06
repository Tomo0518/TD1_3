#pragma once
#include "ComponentBase.h"
#include <vector>
#include <memory>
#include <typeindex> // コンポーネント検索に利用
#include "Vector2.h"
#include "Transform2D.h"

// 前方宣言
class ComponentBase;
class DrawComponent2D;
class Camera2D;

class Object2D {
public:

	// 初期化
	virtual void Initialize();

	// 更新
	virtual void Update(float dt);

	// 描画
	virtual void Draw(const Camera2D& camera) const;
	//virtual void Draw() const; // カメラ非対応版（カメラを使わないゲームの設計の場合使用）

	// ========================================
	// コンポーネント管理
	// ========================================

	// コンポーネント取得ヘルパー (template関数)
	template<typename T>
	T* GetComponent() const {
		for (const auto& comp : components_) {
			T* result = dynamic_cast<T*>(comp.get());
			if (result != nullptr) {
				return result;
			}
		}
		return nullptr;
	}

protected:
	// トランスフォーム情報
	Transform2D transform_;

	// アクティブ・生存フラグ
	bool isActive_ = true;
	bool isAlive_ = true;

	// コンポーネント群(Draw,Move,Collosionなど)
	std::vector<std::unique_ptr<ComponentBase>> components_;

	// コンポーネントの同期処理
	void SyncComponents();

	// コンポーネントをアタッチするヘルパー (GameObject::Initialize内で利用)
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		auto newComp = std::make_unique<T>(this, std::forward<Args>(args)...);
		T* ptr = newComp.get();
		components_.push_back(std::move(newComp));
		return ptr;
	};
};

