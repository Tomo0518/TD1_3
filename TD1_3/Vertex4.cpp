#include "Vertex4.h"

Vertex4 Vertex4::TransformVertex4(const Vector2& center, const float width, const float height) {
	const float halfWidth = width * 0.5f;
	const float halfHeight = height * 0.5f;
	return Vertex4(
		Vector2(center.x - halfWidth, center.y - halfHeight), // leftTop
		Vector2(center.x + halfWidth, center.y - halfHeight), // rightTop
		Vector2(center.x - halfWidth, center.y + halfHeight), // leftBottom
		Vector2(center.x + halfWidth, center.y + halfHeight)  // rightBottom
	);
}