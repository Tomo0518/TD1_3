#include "Matrix3x3.h"
#include <cmath>
#include <assert.h>

Matrix3x3 Matrix3x3::Add(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 result = { 0 };
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::Subtract(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 result = { 0 };
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::Multiply(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 matrix;
	for (int row = 0; row < 3; ++row) {
		for (int columns = 0; columns < 3; ++columns) {
			matrix.m[row][columns] = 0.0f;
			for (int k = 0; k < 3; ++k) {
				matrix.m[row][columns] += m1.m[row][k] * m2.m[k][columns];
			}
		}
	}
	return matrix;
};

Matrix3x3 Matrix3x3::Inverse(const Matrix3x3& m) {
	Matrix3x3 result;
	float determinant;

	determinant = {
		(m.m[0][0] * m.m[1][1] * m.m[2][2])
		+ (m.m[0][1] * m.m[1][2] * m.m[2][0])
		+ (m.m[0][2] * m.m[1][0] * m.m[2][1])
		- (m.m[0][2] * m.m[1][1] * m.m[2][0])
		- (m.m[0][1] * m.m[1][0] * m.m[2][2])
		- (m.m[0][0] * m.m[1][2] * m.m[2][1])
	};

	result.m[0][0] = (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) / determinant;
	result.m[0][1] = -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) / determinant;
	result.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) / determinant;

	result.m[1][0] = -(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) / determinant;
	result.m[1][1] = (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]) / determinant;
	result.m[1][2] = -(m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0]) / determinant;

	result.m[2][0] = (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]) / determinant;
	result.m[2][1] = -(m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0]) / determinant;
	result.m[2][2] = (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]) / determinant;

	return result;
};

/*--------------------------------------*/
/* 3x3 →　Vector2
----------------------------------------*/
// 2次元ベクトルを同時座標として3x3行列との積をとる関数
Vector2 Matrix3x3::Transform(const Vector2& vector, const Matrix3x3& matrix) {
	Vector2 result; // w=1がデカルト座標系であるので(x,y,1)のベクトルとしてmatrixとの積をとる
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + 1.0f * matrix.m[2][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + 1.0f * matrix.m[2][1];
	float w = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + 1.0f * matrix.m[2][2];
	assert(w != 0.0f); // ベクトルに対して基本的な操作を行う行列でwが0になることはない
	result.x /= w; // w=1がデカルト座標系でもあるので、w徐算することで同時座標をデカルト座標に戻す
	result.y /= w;
	return result;
};

// 3x3単位行列を返す
Matrix3x3 Matrix3x3::Identity() {
	Matrix3x3 result = {};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}