//#pragma once
//#include <memory>
//
//// 前方宣言
//class GameObject2D;
//
//class ComponentBase {
//public:
//	// 依存性の注入 : 自身のオーナーの設定
//	ComponentBase(GameObject2D* owner) : owner_(owner) {}
//	virtual ~ComponentBase() = default;
//
//	// 必須のライフサイクルメソッド
//	virtual void Initialize() {}
//	virtual void Update(float dt) {};
//	// DrawはDrawComponent2Dなどの描画コンポーネントに任せるので、ここでは仮想関数として定義しない
//
//protected:
//	// 所有者へのポインタ。これを通じてTransformや他のコンポーネントにアクセス可能
//	GameObject2D* owner_ = nullptr;
//};
//
