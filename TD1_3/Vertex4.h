#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"

class Vertex4 {
public:
	Vertex4() = default;

	Vertex4(const Vector2& lt, const Vector2& rt, const Vector2& lb, const Vector2& rb)
		: leftTop(lt), rightTop(rt), leftBottom(lb), rightBottom(rb) {
	}

	static Vertex4 TransformVertex4(const Vector2& center, const float width, const float height);

	Vector2 leftTop;
	Vector2 rightTop;
	Vector2 leftBottom;
	Vector2 rightBottom;

};