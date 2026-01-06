#include "Vector2.h"
#include <cmath>

// 加算
Vector2 Vector2::Add(const Vector2& v1, const Vector2& v2) {
	Vector2 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

// 除算
Vector2 Vector2::Subtract(const Vector2& v1, const Vector2& v2) {
	Vector2 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}

// 乗算
Vector2 Vector2::Multiply(float scalar, const Vector2& v1) {
	Vector2 result;
	result.x = v1.x * scalar;
	result.y = v1.y * scalar;
	return result;
}

// 内積
float Vector2::Dot(const Vector2& v1, const Vector2& v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

// 長さを求める
float Vector2::Length(const Vector2& v) {
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

// ノーマライズ(正規化)
Vector2 Vector2::Normalize(const Vector2& v) {
	float length = std::sqrtf(v.x * v.x + v.y * v.y);
	if (length == 0.0f) {
		return { 0.0f, 0.0f };
	}
	return { v.x / length, v.y / length };
}

// 外積
float Vector2::Cross(const Vector2& v1, const Vector2& v2) {
	return  v1.x * v2.y - v1.y * v2.x;
}

Vector2 operator+(const Vector2& v1, const Vector2& v2) { return Vector2::Add(v1, v2); }
Vector2 operator-(const Vector2& v1, const Vector2& v2) { return Vector2::Subtract(v1, v2); }
Vector2 operator*(float s, const Vector2& v) { return Vector2::Multiply(s, v); }
Vector2 operator*(const Vector2& v, float s) { return s * v; }
Vector2 operator/(const Vector2& v, float s) { return Vector2::Multiply(1.0f / s, v); }