#pragma once
#include "Matrix3x3.h"

// #include "GameObject.h" // 循環参照を避けるためGameObject* parent_は前方宣言で

class GameObject2D; // 前方宣言

struct Transform2D {
	// ローカル座標（親からの相対位置）
	Vector2 localPosition = { 0.0f, 0.0f };
	Vector2 localScale = { 1.0f, 1.0f };
	float localRotation = 0.0f; // ラジアンか度か統一する

	// ワールド座標 (計算結果)
	Matrix3x3 worldMatrix;

	// 階層関係 (GameObject::Update内で処理されるため、GameObject::Initializeで設定)
	GameObject2D* parent = nullptr;

	// World行列を再計算する関数 (Matrix3x3の関数を利用)
	void CalculateWorldMatrix();
};