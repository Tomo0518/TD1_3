//#include "Object2D.h"
//
//void Object2D::Update(float dt) {
//	if (!isActive_ || !isAlive_) {
//		return;
//	}
//
//	// 1.全コンポーネントのロジック実行(移動、AI、入力処理など)
//	for (const auto& comp : components_) {
//		comp->Update(dt);
//	}
//
//	// 2.確定したTransform情報をコンポーネントに同期
//	SyncComponents();
//}
//
//void Object2D::SyncComponents() {
//	// MoveComponentがtransform_を更新した後、ColliderComponentへ反映させる
//	// ColliderComponent* collider = GetComponent<ColliderComponent>(); // 毎回検索は重いので、Initialize時にキャッシュしておく方が良い
//	// if (collider) {
//	//    collider->Sync(transform_);
//	// }
//
//	// DrawComponentはDraw時に同期すればOK
//}