#include "Vertex4Component.h"
#include <Novice.h>

// 4頂点分のTransformを行う
Vertex4 Transform(const Vertex4& vertex, const Matrix3x3& matrix) {
	Vertex4 resultVertex;
	resultVertex.leftTop = Matrix3x3::Transform(vertex.leftTop, matrix);
	resultVertex.rightTop = Matrix3x3::Transform(vertex.rightTop, matrix);
	resultVertex.leftBottom = Matrix3x3::Transform(vertex.leftBottom, matrix);
	resultVertex.rightBottom = Matrix3x3::Transform(vertex.rightBottom, matrix);

	return resultVertex;
}

/// <summary>
/// 指定された4頂点とグラフィックハンドルを使って四角形を描画します。
/// </summary>
/// <param name="v">四角形の4つの頂点座標を持つVertex4構造体の参照。</param>
/// <param name="grHandle">描画に使用するグラフィックハンドル。</param>
/// <param name="grDrawWidth">描画するグラフィックの幅（ピクセル単位）。</param>
/// <param name="grDrawHeight">描画するグラフィックの高さ（ピクセル単位）。</param>
/// <param name="color">描画する四角形の色（ARGB形式の32ビット値）。</param>
void Vertex4Component::DrawVertexQuad(const Vertex4& v, int grHandle, unsigned int grDrawWidth, unsigned int grDrawHeight, unsigned int color) const {

	Novice::DrawQuad(
		static_cast<int>(v.leftTop.x), static_cast<int>(v.leftTop.y),
		static_cast<int>(v.rightTop.x), static_cast<int>(v.rightTop.y),
		static_cast<int>(v.leftBottom.x), static_cast<int>(v.leftBottom.y),
		static_cast<int>(v.rightBottom.x), static_cast<int>(v.rightBottom.y),
		0, 0,
		grDrawWidth, grDrawHeight,
		grHandle,
		color
	);
}

/// <summary>
/// 中心座標と幅高さから四角形を描画する
/// </summary>
/// <param name="center"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="grHandle"></param>
/// <param name="grDrawWidth"></param>
/// <param name="grDrawHeight"></param>
/// <param name="color"></param>
void Vertex4Component::DrawVertexQuadWH(const Vector2& center, const float width, const float height, int grHandle, unsigned int grDrawWidth, unsigned int grDrawHeight, unsigned int color = 0xFFFFFFFF) const {
	Vertex4 localV = Vertex4::TransformVertex4(center, width, height);

	Novice::DrawQuad(
		static_cast<int>(localV.leftTop.x), static_cast<int>(localV.leftTop.y),
		static_cast<int>(localV.rightTop.x), static_cast<int>(localV.rightTop.y),
		static_cast<int>(localV.leftBottom.x), static_cast<int>(localV.leftBottom.y),
		static_cast<int>(localV.rightBottom.x), static_cast<int>(localV.rightBottom.y),
		0, 0,
		grDrawWidth, grDrawHeight,
		grHandle,
		color
	);
}