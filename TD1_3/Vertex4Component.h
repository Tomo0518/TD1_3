#pragma once
#include "Vertex4.h"
#include "Vertex4Component.h"

class Vertex4Component {
public:
	Vertex4 worldVertex;
	Vertex4 localVertex;

	// 四隅の更新
	void SetBySize(float width, float height) {
		localVertex.leftTop = { -width / 2, height / 2 };
		localVertex.rightTop = { width / 2, height / 2 };
		localVertex.leftBottom = { -width / 2,  -height / 2 };
		localVertex.rightBottom = { width / 2,  -height / 2 };
	}

	Vertex4 Transform(const Vertex4& v, const Matrix3x3& matrix) {
		Vertex4 resultVertex;
		resultVertex.leftTop = Matrix3x3::Transform(v.leftTop, matrix);
		resultVertex.rightTop = Matrix3x3::Transform(v.rightTop, matrix);
		resultVertex.leftBottom = Matrix3x3::Transform(v.leftBottom, matrix);
		resultVertex.rightBottom = Matrix3x3::Transform(v.rightBottom, matrix);
		return resultVertex;
	}

	Vertex4 TransformScreen(const Vertex4& v, const Matrix3x3& matrix) {
		Vertex4 resultVertex;
		resultVertex.leftTop = Matrix3x3::Transform(v.leftBottom, matrix);
		resultVertex.rightTop = Matrix3x3::Transform(v.rightBottom, matrix);
		resultVertex.leftBottom = Matrix3x3::Transform(v.leftTop, matrix);
		resultVertex.rightBottom = Matrix3x3::Transform(v.rightTop, matrix);
		return resultVertex;
	}

	// 四隅を使って画像を描画する
	void DrawVertexQuad(const Vertex4& v, int grHandle, unsigned int grDrawWidth, unsigned int grDrawHeight, unsigned int color = 0xFFFFFFFF)const;

	void DrawVertexQuadWH(const Vector2& center, const float width, const float height, int grHandle, unsigned int grDrawWidth, unsigned int grDrawHeight, unsigned int color) const;
};

