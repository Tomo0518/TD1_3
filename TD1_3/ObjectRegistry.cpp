#include "ObjectRegistry.h"

enum class ObjectRegistryType {
	WorldOrigin = 0,
	PlayerCheckPoint = 99,
	PlayerStart = 100,
	EnemyNormal = 101,
	EnemySpawner = 102,

	AttackEnemy = 103,
	AttackEnemySpawner = 104,
	FatEnemy = 105,
	// ItemCoin = 200,
	// ItemPowerUp = 201,

	Button1 = 300,
	Door1 = 400,
};

// 静的メンバの定義
std::vector<ObjectTypeInfo> ObjectRegistry::objectTypes_;

void ObjectRegistry::Initialize() {
	objectTypes_.clear();

	// ===================================================================
	// システムオブジェクト
	// ===================================================================
	objectTypes_.push_back({
		0,                  // id
		"WorldOrigin",      // name
		"System",           // category
		0xFF0000FF,         // color（赤）
		"WorldOrigin"       // tag
		});

	// ===================================================================
	// プレイヤー関連
	// ===================================================================
	objectTypes_.push_back({
		100,                // id
		"PlayerStart",      // name
		"Player",           // category
		0x00FF00FF,         // color（緑）
		"Player"            // tag
		});

	objectTypes_.push_back({
		99,
		"PlayerCheckPoint",
		"Player",
		0x00FF88FF,     // color（薄い緑）
		"CheckPoint"
		});

	// ===================================================================
	// 敵キャラクター（今後追加）
	// ===================================================================
	objectTypes_.push_back({
		101,
		"Enemy_Normal",
		"Enemy",
		0xFF00FFFF,     // color（マゼンタ）
		"Enemy"
		});

	objectTypes_.push_back({
		102,
		"EnemySpawner",
		"ObjectSpawner",
		0xFF0088FF,     // color（濃いマゼンタ）
		"EnemySpawner"
		});

	objectTypes_.push_back({
	   103,
	   "AttackEnemy",
	   "Enemy",
	   0xFFFF00FF,     // color（黄色）
	   "AttackEnemy"
		});
	objectTypes_.push_back({
		104,
		"AttackEnemySpawner",
		"ObjectSpawner",
		0xFF8800FF,     // color（オレンジ）
		"AttackEnemySpawner"
		});
	objectTypes_.push_back({
		105,
		"FatEnemy",
		"Enemy",
		0xFF8800FF,     // color（オレンジ）
		"FatEnemy"
		});

	// stage gimmick
	objectTypes_.push_back({
		300,
		"Button_1",
		"Gimmick",
		0xAAAA00FF,     // color（黄土色）
		"Button_1"
		});
	objectTypes_.push_back({
		400,
		"Door_1",
		"Gimmick",
		0x00AAAAFF,     // color（水色）
		"Door_1"
		});

	objectTypes_.push_back({
		301,
		"Button_2",
		"Gimmick",
		0xAAAA00FF,     // color（黄土色）
		"Button_2"
		});
	objectTypes_.push_back({
		401,
		"Door_2",
		"Gimmick",
		0x00AAAAFF,     // color（水色）
		"Door_2"
		});
	// ===================================================================
	// アイテム（今後追加）
	// ===================================================================
	// objectTypes_.push_back({
	//     200,
	//     "Item_Coin",
	//     "Item",
	//     0xFFFF00FF,     // color（黄色）
	//     "Coin"
	// });

	// objectTypes_.push_back({
	//     201,
	//     "Item_PowerUp",
	//     "Item",
	//     0xFF8800FF,     // color（オレンジ）
	//     "PowerUp"
	// });
}

const std::vector<ObjectTypeInfo>& ObjectRegistry::GetAllObjectTypes() {
	return objectTypes_;
}

const ObjectTypeInfo* ObjectRegistry::GetObjectType(int objectTypeId) {
	for (const auto& objType : objectTypes_) {
		if (objType.id == objectTypeId) {
			return &objType;
		}
	}
	return nullptr;  // 該当するIDが見つからない
}