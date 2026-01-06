#pragma once

class Vector2 {
public:
	float x, y;

	// 加算代入
	Vector2& operator+=(const Vector2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	// 減算代入
	Vector2& operator-=(const Vector2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	// スカラー倍代入
	Vector2& operator*=(float s) {
		x *= s;
		y *= s;
		return *this;
	}

	Vector2& operator/=(float s) {
		x /= s;
		y /= s;
		return *this;
	}

	static Vector2 Add(const Vector2& v1, const Vector2& v2);
	static Vector2 Subtract(const Vector2& v1, const Vector2& v2);
	static Vector2 Multiply(float scalar, const Vector2& v1);
	static float Dot(const Vector2& v1, const Vector2& v2);
	static float Length(const Vector2& v);
	static Vector2 Normalize(const Vector2& v);
	static float Cross(const Vector2& v1, const Vector2& v2);

};

Vector2 operator+(const Vector2& v1, const Vector2& v2);
Vector2 operator-(const Vector2& v1, const Vector2& v2);
Vector2 operator*(float s, const Vector2& v);
Vector2 operator*(const Vector2& v, float s);
Vector2 operator/(const Vector2& v, float s);