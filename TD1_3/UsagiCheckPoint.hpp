#pragma once
#include "PhysicsObject.hpp"

class UsagiCheckPoint : public PhysicsObject {
	protected:
	DrawComponent2D* onComp_ = nullptr;
	DrawComponent2D* offComp_ = nullptr;
public:
	UsagiCheckPoint() {
		delete drawComp_;
		drawComp_ = nullptr;
		//Initialize();
	}

	void Initialize() override {
		GetInfo().tag = "CheckPoint";
		rigidbody_.Initialize();
		info_.isActive = true;
		info_.isVisible = true;
		collider_.size = { 160.f, 185.f };
		collider_.offset = { 0.f, 0.f };
		onComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiCheckPoint_On));
		offComp_ = new DrawComponent2D(Tex().GetTexture(TextureId::UsagiCheckPoint_Off));
		onComp_->Initialize();
		offComp_->Initialize();
		drawComp_ = offComp_;
	}

	int OnCollision(GameObject2D* other) override {
		if (other->GetInfo().tag == "Player") {
			isActive_ = true;
			drawComp_ = onComp_;
		}
		return 0;
	}

private:
	bool isActive_;
};