#pragma once
#include "Vector2.h"

class Matrix3x3 {
public:
	float m[3][3];

	static Matrix3x3 Add(const Matrix3x3& m1, const Matrix3x3& m2);
	static Matrix3x3 Subtract(const Matrix3x3& m1, const Matrix3x3& m2);
	static Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2);
	static Matrix3x3 Inverse(const Matrix3x3& m);
	static Vector2 Transform(const Vector2& vector, const Matrix3x3& matrix);
	static Matrix3x3 Identity();// 3x3単位行列を返す静的関数
};
